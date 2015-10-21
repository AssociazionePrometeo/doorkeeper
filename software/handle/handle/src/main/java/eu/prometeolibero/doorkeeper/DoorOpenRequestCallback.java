package eu.prometeolibero.doorkeeper;

import org.eclipse.paho.client.mqttv3.IMqttDeliveryToken;
import org.eclipse.paho.client.mqttv3.MqttCallback;
import org.eclipse.paho.client.mqttv3.MqttMessage;


/**
 * Created by francesco.golia on 03/10/15.
 * <p/>
 * This callback takes care of the requests coming from the rfid reader
 * of one door. The aim is to grant or deny access by verifying the rfid
 * grants on a database or a via service call depending on the runtime configuration.
 */
public class DoorOpenRequestCallback implements MqttCallback {
    private HandleConfiguration hConf = HandleConfiguration.getSingleton();
    private Resource resource;
    private String doorTopic;
    private int doorQos;
    private String doorClientId;
    private String doorUsername;
    private String doorPassword;

    public void connectionLost(Throwable throwable) {
        System.out.println("Connection lost!");

    }

    public void messageArrived(String s, MqttMessage mqttMessage) throws Exception {
        User user = new User();
        byte[] rfid = mqttMessage.getPayload();
        user.setRfidTag(rfid); // the body only contains the rfid tag ID
        System.out.println("Processing Door open request...");
        System.out.println("Checking authorization...");
        // check if the access to the resource should be granted or denied
        int authorized = gethConf().getAuthorizationCheckerImplementation()
                                   .isAuthorized(user, resource);

        System.out.println("Building response message...");
        // the body of the response message will be composed by as follows:
        //    the first byte contains the authorization info 0=access granted >0 access denied
        //    the rest of the payload is the rfid tag ID
        byte[] response = new byte[rfid.length + 1];
        response[0] = (byte) authorized;
        for (int i = 1; i < response.length; i++) {
            response[i] = rfid[i - 1];
        }

        // publish the response message
        System.out.println("Publishing message on the command queue");
        MqttUtility mqtt = new MqttUtility();
        mqtt.publishMessage(doorTopic,
                response,
                doorQos,
                hConf.getMqttBroker(),
                doorClientId,
                doorUsername,
                doorPassword);
    }

    public void deliveryComplete(IMqttDeliveryToken iMqttDeliveryToken) {

    }

    public HandleConfiguration gethConf() {
        return hConf;
    }

    public void sethConf(HandleConfiguration hConf) {
        this.hConf = hConf;
    }

    public Resource getResource() {
        return resource;
    }

    public void setResource(Resource resource) {
        this.resource = resource;
    }

    public String getDoorTopic() {
        return doorTopic;
    }

    public void setDoorTopic(String doorTopic) {
        this.doorTopic = doorTopic;
    }

    public int getDoorQos() {
        return doorQos;
    }

    public void setDoorQos(int doorQos) {
        this.doorQos = doorQos;
    }

    public String getDoorClientId() {
        return doorClientId;
    }

    public void setDoorClientId(String doorClientId) {
        this.doorClientId = doorClientId;
    }

    public String getDoorUsername() {
        return doorUsername;
    }

    public void setDoorUsername(String doorUsername) {
        this.doorUsername = doorUsername;
    }

    public String getDoorPassword() {
        return doorPassword;
    }

    public void setDoorPassword(String doorPassword) {
        this.doorPassword = doorPassword;
    }
}
