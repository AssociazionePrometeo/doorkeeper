package eu.prometeolibero.doorkeeper;

import javax.ws.rs.core.MediaType;
import javax.ws.rs.core.UriBuilder;

import javax.ws.rs.client.Client;
import javax.ws.rs.client.ClientBuilder;
import javax.ws.rs.client.WebTarget;
import org.glassfish.jersey.client.ClientConfig;

import org.apache.commons.codec.binary.Hex;
import org.glassfish.jersey.client.authentication.HttpAuthenticationFeature;

/**
 * Created by francesco.golia on 03/10/15.
 */
public class RESTClientAuthorizationChecker implements IAuthorizationChecker {
    HandleConfiguration hConf = HandleConfiguration.getSingleton();

    public int isAuthorized(User user, Resource resource) {
        ClientConfig config = new ClientConfig();
        Client client = ClientBuilder.newClient(config);

        // Authentication Required
        if(hConf.isEndpointAuthRequired()) {
            HttpAuthenticationFeature feature = HttpAuthenticationFeature
                    .basic(hConf.getRestUser(), hConf.getRestPassword());
            client.register(feature);
        }

        WebTarget target = client.target(UriBuilder.fromUri(hConf.getRestEndpoint()).build());
        CheckReservationResponse response =  target.path("/resources")
                       .path("/"+resource.getId())
                       .path("/check")
                       .path("/"+Hex.encodeHexString(user.getRfidTag()))
                       .request(MediaType.APPLICATION_JSON)
                       .get(CheckReservationResponse.class);
        return response.allow ? 0 : 1 ;
    }

    final static class CheckReservationResponse{
        public String time;
        public String reservation;
        public boolean allow;
    }
}
