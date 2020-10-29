import javafx.geometry.Insets;
import javafx.geometry.Pos;
import javafx.scene.control.Label;
import javafx.scene.control.ProgressIndicator;
import javafx.scene.layout.VBox;

/**
 * @author silhavyj A147B0362P
 *
 * This class represents a form the users sees
 * when they send a game request to another client
 * and wait for their response. The class is extended
 * from class VBox (JavaFX).
 * */
public class WaitingForGameRequestForm extends VBox {

    /** the total width of the form */
    public static final int FORM_WIDTH = 450;
    /** the total height of the form */
    public static final int FORM_HEIGHT = 250;
    /** title of the button for canceling the game request */
    private static final String CANCEL_BTN_TXT = "Cancel";

    /** reference to the main class */
    private Main main;
    /** button used for canceling the game request */
    private NickButton cancelBtn;
    /** label holding the description of the form (who the game request was sent to, ...) */
    private Label descLabel;

    /**
     * Constructor of the class - creates an instance of it
     * @param main reference to the main class
     * */
    public WaitingForGameRequestForm(Main main) {
        super();

        this.main = main;

        // creates the content of the form
        getChildren().add(new ProgressIndicator());
        getChildren().add((descLabel = new Label()));
        getChildren().add(createCancelBtn());

        setAlignment(Pos.CENTER);
        setSpacing(10);
        setPadding(new Insets(20, 20,20,20));
    }

    /**
     * Sets the nick of the client the game request was sent to
     * @param nick the receiver's nickname
     * */
    public void setNick(String nick) {
        descLabel.setText("Waiting for client '" + nick + "' to reply to your game request.");
        cancelBtn.setNick(nick);
    }

    /**
     * Creates the button the user can use to cancel
     * their own game request.
     * @return button for canceling the game request
     * */
    private NickButton createCancelBtn() {
        cancelBtn = new NickButton(CANCEL_BTN_TXT);
        cancelBtn.setOnMouseClicked(e -> cancelGameRequest());
        return cancelBtn;
    }

    /**
     * Cancels the game request
     * */
    private void cancelGameRequest() {
        main.cancelGameRequest(cancelBtn.getNick());
        main.displayLobby();
    }
}