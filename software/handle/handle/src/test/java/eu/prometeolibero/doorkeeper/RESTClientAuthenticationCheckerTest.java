package eu.prometeolibero.doorkeeper;

import org.junit.Test;
import static org.junit.Assert.*;

/**
 * Created by francesco.golia on 04/10/15.
 */
public class RESTClientAuthenticationCheckerTest {
    @Test
    public void testRestClient(){
        RESTClientAuthorizationChecker client = new RESTClientAuthorizationChecker();
        User u = new User();
        u.setRfidTag("59dd84ad602".getBytes());
        Resource r = new Resource();
        r.setId(1L);
        assertEquals(1,client.isAuthorized(u, r),0);
    }
}
