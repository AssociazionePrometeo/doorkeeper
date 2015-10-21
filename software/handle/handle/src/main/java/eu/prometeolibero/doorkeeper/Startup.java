package eu.prometeolibero.doorkeeper;

import org.eclipse.paho.client.mqttv3.*;
import org.eclipse.paho.client.mqttv3.persist.MemoryPersistence;

import java.io.File;

/**
 * Hello world!
 *
 */
public class Startup
{
    public static boolean exit = false;

    public static void main( String[] args ){
        HandleConfiguration hConf = HandleConfiguration.getSingleton() ;
        if(args.length>0) {
            File f = new File(args[0]);
            f.exists();
            hConf.loadConfig(f);
        }

        // Initialize handlers and subscribe to queues
        // TODO retrive from the mqtt database the queues to subscribe to
        Resource room = new Resource();
        room.setId(1L);
        DoorOpenRequestCallback doorOpenRequestCallback = new DoorOpenRequestCallback();
        doorOpenRequestCallback.setResource(room);
        doorOpenRequestCallback.setDoorTopic("/s19/acl/p/1");
        doorOpenRequestCallback.setDoorQos(2);
        doorOpenRequestCallback.setDoorClientId("20");
        doorOpenRequestCallback.setDoorUsername("zero1");
        doorOpenRequestCallback.setDoorPassword("zero");
        MqttAsyncClient client = null;
        MemoryPersistence persistence = new MemoryPersistence();
        try {
            client = new MqttAsyncClient(hConf.getMqttBroker(), "1", persistence);
            MqttConnectOptions connOpts = new MqttConnectOptions();
            connOpts.setCleanSession(true);
            connOpts.setUserName("zero1");
            connOpts.setPassword("zero".toCharArray());
            System.out.println("Connecting to broker: " + hConf.getMqttBroker());
            client.connect(connOpts).waitForCompletion();
            System.out.println("Connected");
            client.setCallback(doorOpenRequestCallback);
            // subscribe to the door request queue
            client.subscribe("/s19/p/1",2).waitForCompletion();

            System.out.println("Startup Finished");
            while(!exit){
                try {
                   // System.out.println("IsClientConnected: "+client.isConnected());
                    Thread.sleep(1000);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }

            client.disconnect();
            System.out.println("Disconnected");
        } catch(MqttException me) {
            System.out.println("reason "+me.getReasonCode());
            System.out.println("msg "+me.getMessage());
            System.out.println("loc "+me.getLocalizedMessage());
            System.out.println("cause "+me.getCause());
            System.out.println("excep "+me);
            me.printStackTrace();
        }
    }

}
