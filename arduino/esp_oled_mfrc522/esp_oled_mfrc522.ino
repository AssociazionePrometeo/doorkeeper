#include <ESP8266WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <MQTT.h>
#include "font.h"
#include "mem.h"
/*
* MFRC522 SPI rfid      esp8266-evb
* -----------------------------------------------------------------------------------------------------------
*RST                    GPIO15  pin10    
*SDA(SS)                GPIO16  pin13   
*MOSI                   GPIO13  pin8   
*MISO                   GPIO12  pin7   
*SCK                    GPIO14  pin9   
*GND                    GND        
*3,3V                   3,3V       
*/

#define RST_PIN		15 
#define SS_PIN		16

MFRC522 mfrc522(SS_PIN, RST_PIN);	// Create MFRC522 instance
char uidCard[8]; // Stores scanned ID read from RFID Module + 1 byte for \0

// SSD1306 I2C oled
/*
* OLED                  esp8266-evb
* -----------------------------------------------------------------------------------------------------------
*SDA                    GPIO2   pin6   
*SCL                    GPIO4   pin5   
*GND                    GND        
*3,3V                   3,3V       
*/
#define offset 0x00   // offset=0 for SSD1306 controller
#define OLED_address  0x3c

//MQTT Parameters
#define MQTT_SERVER "192.168.1.10"
#define MQTT_CLIENTID "Zero_port1"
#define MQTT_USER "zero"
#define MQTT_PASS "zero"
#define MQTT_PRESENCE_TOPIC  "/s19/p/1"
#define MQTT_CMD_TOPIC "/s19/acl/p/1"
boolean MQTT_RESET = false;

//WIFI Parameters
#define WIFI_SSID  "gemini"
#define WIFI_PASS  "lavispateresa"

//Relay
#define RELAY_PIN 5
#define RELAY_OPEN_TIME 5000
boolean opening = false;

MQTT myMqtt(MQTT_CLIENTID,MQTT_SERVER,8883,0);

void myConnectedCb() {
    Serial.println("MQTT: Connected");
    myMqtt.subscribe((char*)MQTT_PRESENCE_TOPIC, 0);
    myMqtt.subscribe((char*)MQTT_CMD_TOPIC, 0);    
    myMqtt.publish((char*)MQTT_PRESENCE_TOPIC, MQTT_CLIENTID, strlen(MQTT_CLIENTID), 0, 0);
}

void myDisconnectedCb(){
    Serial.println("MQTT: Disconnected");
    clear_display();
    MQTT_RESET=true;
}

void restartMyMqtt() {
    Serial.println("Starting restart MQTT client");
    MQTT_RESET=false;
    while (!myMqtt.isConnected()) {
      Serial.println("MQTT not connected");
      delay(5000);
      myMqtt.disconnect();
      delay(200);
      myMqtt.connect();
    }
}

void myPublishedCb(){
    //MQTT_Client* client = (MQTT_Client*)args;
    Serial.println("MQTT: Published");
}

void myDataCb(String& topic, String& data){
    Serial.print(topic);
    Serial.print(": ");
    Serial.println(data);
    
    if (topic.compareTo(String(MQTT_CMD_TOPIC))==0 &&
        data.compareTo( String(String(uidCard) + "|open") )==0) {
          opening=true;
    }
    else Serial.println("Nothing to do");
}

void setup() {
    //Setup Serial
    Serial.begin(115200);
    while (!Serial);

    //Setup Relay
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN,LOW);  

    //Setup OLED via I2C
    Wire.begin(2,4);
    init_OLED();
    reset_display(); 
    clear_display();
    sendStrXY((char*)"Oled... ok",1,0);

    //Setup NFC via SPI
    sendStrXY((char*)"NFC.... ",2,0);
    SPI.begin();			
    mfrc522.PCD_Init();
    while (!ShowReaderDetails());
    Serial.println(F("NFC ready"));
    sendStrXY((char*)"NFC.... ok",2,0);

    //Setup WIFI
    sendStrXY((char*)"WIFI... ",3,0);
    Serial.print("Connecting to ");
    Serial.println(WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");  
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    sendStrXY((char*)"WIFI... ok",3,0);
    
    //Setup MQTT
    sendStrXY((char *)"MQTT... ",4,0);      
    Serial.println(F("Setup MQTT Client"));
    myMqtt.setUserPwd(MQTT_USER, MQTT_PASS);
    myMqtt.onConnected(myConnectedCb);
    myMqtt.onDisconnected(myDisconnectedCb);
    myMqtt.onPublished(myPublishedCb);
    myMqtt.onData(myDataCb);
    Serial.println("connect mqtt...");
    while (!myMqtt.isConnected()) {
      myMqtt.connect();
      delay(5000);
    }
    sendStrXY((char *)"MQTT... ok",4,0);

    //End Setup
    delay(1000);
    sendStrXY((char *)"Setup terminated",5,0);
    delay(1000);
    clear_display();
    sendStrXY((char *)"Ready...",1,0);
}

void loop() {
    if(getID()) {
      uidToChar(mfrc522.uid.uidByte, uidCard, mfrc522.uid.size);
      //dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
      Serial.println(uidCard);
      myMqtt.publish(MQTT_PRESENCE_TOPIC,uidCard,strlen(uidCard),0,0);
      //Serial.println();
      clear_display();
      //sendStrXY(uidCard,3,0);
      sendStrXY((char*)"Verifying...",3,0);
    }
    if (opening) {
      opening=false;
      //imporre un controllo sul tempo intercorso tra getID e opening=true
      sendStrXY((char *)"Open door... ok",3,0);
      delay(200);
      digitalWrite(RELAY_PIN,HIGH); // Turns ON Relays
      delay(RELAY_OPEN_TIME);
      digitalWrite(RELAY_PIN,LOW); // Turns OFF Relays
      delay(200);
      clear_display();
      sendStrXY((char *)"Ready...",1,0);
    }
    if (MQTT_RESET) {
      Serial.println("Found MQTT_RESET true");
      restartMyMqtt();
    }
}

// Helper routine to dump a byte array as hex values to Serial
void dump_byte_array(byte *buffer, byte bufferSize) {
    for (byte i = 0; i < bufferSize; i++) {
      Serial.print(buffer[i] < 0x10 ? " 0" : " ");
      Serial.print(buffer[i], HEX);
    }
}

// Helper routine to dump a byte array as char array
void uidToChar(byte arr[], char uidCard[], byte arrLength) {
    int charLength=(arrLength*2 * sizeof(char))+1;
    for (int i=0;i<arrLength;i++){
         byte dummy=arr[i]/16;
         if (dummy<9){
           uidCard[i*2]=dummy+48;
         }else{
           uidCard[i*2]=(dummy)+55;
         }
         dummy=arr[i]%16; 
         if (dummy<9){
           uidCard[(i*2)+1]=dummy+48;
         }else{
           uidCard[(i*2)+1]=(dummy)+55;
         }    
    }
    uidCard[charLength-1]=0;
}

bool ShowReaderDetails() {
  	// Get the MFRC522 software version
  	byte v = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
  	Serial.println(F("MFRC522 Software Version: 0x"));
  	Serial.print(v, HEX);
  	if (v == 0x91)
  		Serial.print(F(" = v1.0"));
  	else if (v == 0x92)
  		Serial.print(F(" = v2.0"));
  	else
  		Serial.print(F(" (unknown)"));
  	Serial.println("");
  	// When 0x00 or 0xFF is returned, communication probably failed
  	if ((v == 0x00) || (v == 0xFF)) {
  		Serial.println(F("WARNING: Communication failure, is the MFRC522 properly connected?"));
      return false;
  	}
   return true;
}

///////////////////////////////////////// Get PICC's UID ///////////////////////////////////
int getID() {
  // Getting ready for Reading PICCs
  if ( ! mfrc522.PICC_IsNewCardPresent()) { //If a new PICC placed to RFID reader continue
    return 0;
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) { //Since a PICC placed get Serial and continue
    delay(50);
    return 0;
  }
  mfrc522.PICC_HaltA(); // Stop reading
  return 1;
}

///////////////////////////////////////// Oled display function /////////////////////////////
//==========================================================//
// Resets display depending on the actual mode.
static void reset_display(void)
{
  displayOff();
  clear_display();
  displayOn();
}

//==========================================================//
// Turns display on.
void displayOn(void)
{
  sendcommand(0xaf);        //display on
}

//==========================================================//
// Turns display off.
void displayOff(void)
{
  sendcommand(0xae);		//display off
}

//==========================================================//
// Clears the display by sendind 0 to all the screen map.
static void clear_display(void)
{
  unsigned char i,k;
  for(k=0;k<8;k++)
  {	
    setXY(k,0);    
    {
      for(i=0;i<(128 + 2 * offset);i++)     //locate all COL
      {
        SendChar(0);         //clear all COL
        //delay(10);
      }
    }
  }
}

//==========================================================//
// Actually this sends a byte, not a char to draw in the display. 
// Display's chars uses 8 byte font the small ones and 96 bytes
// for the big number font.
static void SendChar(unsigned char data) 
{
  //if (interrupt && !doing_menu) return;   // Stop printing only if interrupt is call but not in button functions
  
  Wire.beginTransmission(OLED_address); // begin transmitting
  Wire.write(0x40);//data mode
  Wire.write(data);
  Wire.endTransmission();    // stop transmitting
}

//==========================================================//
// Prints a display char (not just a byte) in coordinates X Y,
// being multiples of 8. This means we have 16 COLS (0-15) 
// and 8 ROWS (0-7).
static void sendCharXY(unsigned char data, int X, int Y)
{
  setXY(X, Y);
  Wire.beginTransmission(OLED_address); // begin transmitting
  Wire.write(0x40);//data mode
  
  for(int i=0;i<8;i++)          
    Wire.write(pgm_read_byte(myFont[data-0x20]+i));
    
  Wire.endTransmission();    // stop transmitting
}

//==========================================================//
// Used to send commands to the display.
static void sendcommand(unsigned char com)
{
  Wire.beginTransmission(OLED_address);     //begin transmitting
  Wire.write(0x80);                          //command mode
  Wire.write(com);
  Wire.endTransmission();                    // stop transmitting
}

//==========================================================//
// Set the cursor position in a 16 COL * 8 ROW map.
static void setXY(unsigned char row,unsigned char col)
{
  sendcommand(0xb0+row);                //set page address
  sendcommand(offset+(8*col&0x0f));       //set low col address
  sendcommand(0x10+((8*col>>4)&0x0f));  //set high col address
}


//==========================================================//
// Prints a string regardless the cursor position.
static void sendStr(unsigned char *string)
{
  unsigned char i=0;
  while(*string)
  {
    for(i=0;i<8;i++)
    {
      SendChar(pgm_read_byte(myFont[*string-0x20]+i));
    }
    *string++;
  }
}

//==========================================================//
// Prints a string in coordinates X Y, being multiples of 8.
// This means we have 16 COLS (0-15) and 8 ROWS (0-7).
static void sendStrXY( char *string, int X, int Y)
{
  setXY(X,Y);
  unsigned char i=0;
  while(*string)
  {
    for(i=0;i<8;i++)
    {
      SendChar(pgm_read_byte(myFont[*string-0x20]+i));
    }
    *string++;
  }
}


//==========================================================//
// Inits oled and draws logo at startup
static void init_OLED(void)
{
  sendcommand(0xae);		//display off
  sendcommand(0xa6);            //Set Normal Display (default)
    // Adafruit Init sequence for 128x64 OLED module
    sendcommand(0xAE);             //DISPLAYOFF
    sendcommand(0xD5);            //SETDISPLAYCLOCKDIV
    sendcommand(0x80);            // the suggested ratio 0x80
    sendcommand(0xA8);            //SSD1306_SETMULTIPLEX
    sendcommand(0x3F);
    sendcommand(0xD3);            //SETDISPLAYOFFSET
    sendcommand(0x0);             //no offset
    sendcommand(0x40 | 0x0);      //SETSTARTLINE
    sendcommand(0x8D);            //CHARGEPUMP
    sendcommand(0x14);
    sendcommand(0x20);             //MEMORYMODE
    sendcommand(0x00);             //0x0 act like ks0108
    
    //sendcommand(0xA0 | 0x1);      //SEGREMAP   //Rotate screen 180 deg
    sendcommand(0xA0);
    
    //sendcommand(0xC8);            //COMSCANDEC  Rotate screen 180 Deg
    sendcommand(0xC0);
    
    sendcommand(0xDA);            //0xDA
    sendcommand(0x12);           //COMSCANDEC
    sendcommand(0x81);           //SETCONTRAS
    sendcommand(0xCF);           //
    sendcommand(0xd9);          //SETPRECHARGE 
    sendcommand(0xF1); 
    sendcommand(0xDB);        //SETVCOMDETECT                
    sendcommand(0x40);
    sendcommand(0xA4);        //DISPLAYALLON_RESUME        
    sendcommand(0xA6);        //NORMALDISPLAY             

  clear_display();
  sendcommand(0x2e);            // stop scroll
  //----------------------------REVERSE comments----------------------------//
    sendcommand(0xa0);		//seg re-map 0->127(default)
    sendcommand(0xa1);		//seg re-map 127->0
    sendcommand(0xc8);
    delay(1000);
  //----------------------------REVERSE comments----------------------------//
  // sendcommand(0xa7);  //Set Inverse Display  
  // sendcommand(0xae);		//display off
  sendcommand(0x20);            //Set Memory Addressing Mode
  sendcommand(0x00);            //Set Memory Addressing Mode ab Horizontal addressing mode
  //  sendcommand(0x02);         // Set Memory Addressing Mode ab Page addressing mode(RESET)  
}


