package eu.prometeolibero.doorkeeper;

import org.apache.commons.codec.binary.Hex;


/**
 * Created by francesco.golia on 03/10/15.
 *
 * Check the user authorization on the database;
 */
public class DatabaseAuthorizationChecker implements IAuthorizationChecker {
    public int isAuthorized(User user, Resource resource) {
        return 0;
    }
}
