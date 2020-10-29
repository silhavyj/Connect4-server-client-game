import javafx.scene.control.Button;

/**
 * @author silhavyj A147B0362P
 *
 * This class represents a simple button holding
 * a nick of a client, so when the user clicks
 * on it, the button know who it should send a
 * game request to (for example). The class is
 * extended from class Button (JavaFX).
 * */
public class NickButton extends Button {

    /** nick of a client */
    private String nick;

    /**
     * Constructor of the class - creates an instance of it
     * @param text title of the button
     * */
    public NickButton(String text) {
        super(text);
        this.nick = null;
    }

    /**
     * Constructor of the class - creates an instance of it
     * @param text title of the button
     * @param nick client's nick
     * */
    public NickButton(String text, String nick) {
        super(text);
        this.nick = nick;
    }

    /**
     * Getter of the nick
     * @return the client's nick
     * */
    public String getNick() {
        return nick;
    }

    /**
     * Setting of the nick
     *  @param nick the new value of nick
     *  */
    public void setNick(String nick) {
        this.nick = nick;
    }
}