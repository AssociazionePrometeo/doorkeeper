/**
 * \file
 *       ESP8266 MQTT Bridge example
 * \author
 *       Tuan PM <tuanpm@live.com>
 */
#include <SoftwareSerial.h>
#include <espduino.h>
#include <mqtt.h>
#include <MFRC522.h>
#include <SPI.h>
//#include <MemoryFree.h>

//Board Pin definition
#define ESP_CHPD_PIN   4
#define DEBUG_PORT_RX  7
#define DEBUG_PORT_TX  8
#define RST_PIN        9           // Configurable, see typical pin layout above
#define SS_PIN        10          // Configurable, see typical pin layout above

//MQTT Parameters
#define MQTT_SERVER "192.168.1.10"
#define MQTT_CLIENTID "ZERO_port1"
#define MQTT_USER "zero"
#define MQTT_PASS "zero"
#define MQTT_PRESENCE_TOPIC  "/s19/p/1"
#define MQTT_CMD_TOPIC "/s19/acl/p/1"

//WIFI Parameters
#define WIFI_SSID  "gemini"
#define WIFI_PASS  "lavispateresa"


//ESP
SoftwareSerial debugPort(DEBUG_PORT_RX, DEBUG_PORT_TX); // RX, TX
ESP esp(&Serial, &debugPort, ESP_CHPD_PIN);
MQTT mqtt(&esp);
boolean wifiConnected = false;
boolean mqttIsConnect = false;

//NFC
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.
char uidCard[8]; // Stores scanned ID read from RFID Module + 1 byte for \0


void wifiCb(void* response)
{
  uint32_t status;
  RESPONSE res(response);

  if(res.getArgc() == 1) {
    res.popArgs((uint8_t*)&status, 4);
    debugPort.println(status);
    if(status == STATION_GOT_IP) {
      debugPort.println(F("WIFI CONNECTED"));
//      mqtt.connect("192.168.1.10", 1883, false); 
      mqtt.connect(MQTT_SERVER, 8883, 0); //1 for ssl security 
      wifiConnected = true;
    } 
    else {
      wifiConnected = false;
      mqtt.disconnect();
    }
    
  }
}

void mqttConnected(void* response)
{
  mqttIsConnect = true;
  debugPort.println(F("Connected"));
  mqtt.subscribe(MQTT_PRESENCE_TOPIC,1);
  mqtt.subscribe(MQTT_CMD_TOPIC,1);
  mqtt.publish(MQTT_PRESENCE_TOPIC,MQTT_CLIENTID);
}
void mqttDisconnected(void* response)
{
  mqttIsConnect = false;
  delay(5000);  
}
void mqttData(void* response)
{
  RESPONSE res(response);

  debugPort.print("Received: topic=");
  String topic = res.popString();
  debugPort.println(topic);

  debugPort.print("data=");
  String data = res.popString();
  debugPort.println(data);

}
void mqttPublished(void* response)
{
  debugPort.println(F("After mqttPublished"));
}

///////////////////////////////////////// Get PICC's UID ///////////////////////////////////
int getID() {
  // Getting ready for Reading PICCs
  if ( ! mfrc522.PICC_IsNewCardPresent()) { //If a new PICC placed to RFID reader continue
    return 0;
  }
  if ( ! mfrc522.PICC_ReadCardSerial()) { //Since a PICC placed get Serial and continue
    return 0;
  }
  mfrc522.PICC_HaltA(); // Stop reading
  return 1;
}

void uidToChar(byte arr[], char uidCard[], byte arrLength) {
  int charLength=(arrLength*2 * sizeof(char))+1;
//  char *myCharArray = (char*) malloc((arrLength*2) * sizeof(char))+1;

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

void ShowReaderDetails() {
  // Get the MFRC522 software version
  byte v = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
  debugPort.print(F("MFRC522 Software Version: 0x"));
  debugPort.print(v, HEX);
  if (v == 0x91)
    debugPort.print(F(" = v1.0"));
  else if (v == 0x92)
    debugPort.print(F(" = v2.0"));
  else
    debugPort.print(F(" (unknown)"));
  debugPort.println(F(""));
  // When 0x00 or 0xFF is returned, communication probably failed
  if ((v == 0x00) || (v == 0xFF)) {
  debugPort.println(F("WARNING: Communication failure, is the MFRC522 properly connected?"));
  }
}

int freeRam () {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

void setup() {
  delay(2000);
  Serial.begin(19200);
  Serial.println(F("Serial ok"));
  debugPort.begin(115200);
  esp.enable();
  Serial.println(F("esp enable"));
  delay(500);
  esp.reset();
  delay(500);
  while(!esp.ready())
  delay(1000);


  //Setup NFC
  SPI.begin();		// Init SPI bus
  mfrc522.PCD_Init();	// Init MFRC522 card
  ShowReaderDetails(); // Show details of PCD - MFRC522 Card Reader details
  debugPort.println(F("Scan PICC to see UID, type, and data blocks..."));
  delay(500);

  //Setup MQTT client
  debugPort.println(F("ARDUINO: setup mqtt client"));
  if(!mqtt.begin(MQTT_CLIENTID, MQTT_USER, MQTT_PASS, 120, 1)) {
    debugPort.println(F("ARDUINO: fail to setup mqtt"));
    while(1);
  }

  /*setup mqtt lwt message */
  debugPort.println(F("ARDUINO: setup mqtt lwt"));
  while (!mqtt.lwt("/lwt", "offline", 0, 0)) {  //or mqtt.lwt("/lwt", "offline")
    debugPort.println(F("ARDUINO: fail to setup mqtt lwt"));
  }
  
  /*setup mqtt events */
  mqtt.connectedCb.attach(&mqttConnected);
  mqtt.disconnectedCb.attach(&mqttDisconnected);
  mqtt.publishedCb.attach(&mqttPublished);
  mqtt.dataCb.attach(&mqttData);

  /*setup esp for wifi sta*/
  debugPort.println(F("ARDUINO: setup wifi"));
  esp.wifiCb.attach(&wifiCb);
  esp.wifiConnect(WIFI_SSID,WIFI_PASS);
  debugPort.println(F("ARDUINO: system started"));
}

void loop() {
  esp.process();

  if (wifiConnected && mqttIsConnect) {
/*  
    // Look for new cards
    if ( ! mfrc522.PICC_IsNewCardPresent()) {
    	return;
    }
    // Select one of the cards
    if ( ! mfrc522.PICC_ReadCardSerial()) {
    	return;
    }
*/
    while(getID()>=1) {
      uidToChar(mfrc522.uid.uidByte, uidCard, mfrc522.uid.size);
      mqtt.publish(MQTT_PRESENCE_TOPIC,uidCard);
      debugPort.print(F("Publishing on "));
      debugPort.print(F(MQTT_PRESENCE_TOPIC));
      debugPort.print(F("value "));
      debugPort.println(uidCard);


    }
  }
  //else reset esp board??
//}  end if wifiConnected && mqttIsConnect
  //debugPort.println(freeRam());
  /*debugPort.print(F("Free mem:"));
  debugPort.println(freeMemory());*/
}

