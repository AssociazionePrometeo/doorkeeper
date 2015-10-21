package eu.prometeolibero.doorkeeper;

import org.apache.commons.codec.DecoderException;
import org.apache.commons.codec.binary.Hex;
import org.junit.Test;

/**
 * Created by francesco.golia on 03/10/15.
 */
public class MqttUtilityTest {

    @Test
    public void publishMessageOnTheCommandQueue(){
        String topic        = "/s19/p/1";
        String content      = "59dd84ad60" ;
        int qos             = 2;
        String broker       = "tcp://localhost:8883";
        String clientId     = "14";
        String username     = "zero1";
        String password     = "zero";

        MqttUtility utility = new MqttUtility();
        try {
            utility.publishMessage(topic,
                    Hex.decodeHex(content.toCharArray()),
                    qos,
                    broker,
                    clientId,
                    username,
                    password);
        } catch (DecoderException e) {
            e.printStackTrace();
        }
    }
}
