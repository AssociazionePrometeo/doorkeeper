package eu.prometeolibero.doorkeeper;

/**
 * Created by francesco.golia on 03/10/15.
 */
public interface IAuthorizationChecker {
    public int isAuthorized(User user, Resource resource);
}
