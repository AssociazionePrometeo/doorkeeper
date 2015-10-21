package eu.prometeolibero.doorkeeper;

/**
 * Created by francesco.golia on 03/10/15.
 */
public class User {
    private String id;
    private String firstName;
    private String lastName;
    private byte[] rfidTag;


    public String getId() {
        return id;
    }

    public void setId(String id) {
        this.id = id;
    }

    public String getFirstName() {
        return firstName;
    }

    public void setFirstName(String firstName) {
        this.firstName = firstName;
    }

    public String getLastName() {
        return lastName;
    }

    public void setLastName(String lastName) {
        this.lastName = lastName;
    }

    public byte[] getRfidTag() {
        return rfidTag;
    }

    public void setRfidTag(byte[] rfidTag) {
        this.rfidTag = rfidTag;
    }
}
