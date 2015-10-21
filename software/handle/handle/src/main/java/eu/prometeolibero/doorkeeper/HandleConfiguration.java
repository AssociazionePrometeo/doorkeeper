package eu.prometeolibero.doorkeeper;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.Properties;

/**
 * Created by francesco.golia on 03/10/15.
 *
 * Configuration Object for the handle project.
 */
public class HandleConfiguration {
    public static final String KEY_AUTH_TYPE = "auth.type";
    public static final String KEY_REST_ENDPOINT = "rest.endpoint";
    public static final String KEY_REST_USERNAME = "rest.username";
    public static final String KEY_REST_PASSWORD = "rest.password";
    public static final String KEY_MQTT_BROKER = "mqtt.broker";

    public static final String AUTH_TYPE_DB = "AUTH_DB";
    public static final String AUTH_TYPE_REST = "AUTH_REST";
    private String restEndpoint;
    private String restUser;
    private String restPassword;
    private String authCheckType;
    private String mqttBroker;
    private static HandleConfiguration singleton = new HandleConfiguration();

    private HandleConfiguration(){
        // try to find the configuration in the user home
        File propertiesFile = new File(
                System.getProperty("user.home")+ File.separator
                        + ".doorkeeper" + File.separator
                        + "handleconfig.properties");
        if (propertiesFile.exists()){
            loadConfig(propertiesFile);
        }
    }

    public String getRestEndpoint() {
        return restEndpoint;
    }

    public void setRestEndpoint(String restEndpoint) {
        this.restEndpoint = restEndpoint;
    }

    public IAuthorizationChecker getAuthorizationCheckerImplementation() {
        switch (authCheckType){
            case AUTH_TYPE_DB: return new DatabaseAuthorizationChecker();
            case AUTH_TYPE_REST: return  new RESTClientAuthorizationChecker();
            default: return  new RESTClientAuthorizationChecker();
        }
    }

    public static HandleConfiguration getSingleton() {
        return singleton;
    }

    public String getAuthCheckType() {
        return authCheckType;
    }

    public void setAuthCheckType(String authCheckType) {
        this.authCheckType = authCheckType;
    }

    public String getRestUser() {
        return restUser;
    }

    public void setRestUser(String restUser) {
        this.restUser = restUser;
    }

    public String getRestPassword() {
        return restPassword;
    }

    public void setRestPassword(String restPassword) {
        this.restPassword = restPassword;
    }

    public boolean isEndpointAuthRequired(){
        return restUser != null && restPassword != null;
    }

    public void loadConfig(File f) {
        Properties p = new Properties();
        try {
            p.load(new FileInputStream(f));
        } catch (IOException e) {
            e.printStackTrace();
        }

        setAuthCheckType(p.getProperty(KEY_AUTH_TYPE));
        setRestEndpoint(p.getProperty(KEY_REST_ENDPOINT));
        setRestUser(p.getProperty(KEY_REST_USERNAME));
        setRestPassword(p.getProperty(KEY_REST_PASSWORD));
        setMqttBroker(p.getProperty(KEY_MQTT_BROKER));
    }

    public String getMqttBroker() {
        return mqttBroker;
    }

    public void setMqttBroker(String mqttBroker) {
        this.mqttBroker = mqttBroker;
    }
}
