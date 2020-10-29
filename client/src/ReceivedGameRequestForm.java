import javafx.geometry.Insets;
import javafx.geometry.Pos;
import javafx.scene.control.Label;
import javafx.scene.layout.HBox;
import javafx.scene.layout.VBox;
import javafx.scene.text.Font;

/**
 * @author silhavyj A147B0362P
 *
 * This class represents a form the users sees
 * when they just sent off a game request to
 * another client and wait for their reponse.
 * This class is exetnded from class VBox (JavaFX).
 * */
public class ReceivedGameRequestForm extends VBox {

    /** total width of the form */
    public static final int FORM_WIDTH = 500;
    /** total height of the form */
    public static final int FORM_HEIGHT = 300;

    /** font size of the title of the window */
    private static final int LABEL_FONT_SIZE = 19;
    /** title of the button for accepting the game request */
    private static final String BTN_ACCEPT_TXT = "Accept";
    /** title of the button for rejecting the game request */
    private static final String BTN_REJECT_TXT = "Reject";

    /** button for accepting the game request */
    private NickButton btnAccept;
    /** button for rejecting the game request */
    private NickButton btnReject;
    /** title of the form */
    private Label title;
    /** reference to the main class */
    private Main main;

    /**
     * Constructor of the class - creates an instance of it
     * @param main reference to the main class
     * */
    public ReceivedGameRequestForm(Main main) {
        super();

        this.main = main;

        // create the content of the form
        getChildren().add(createTitle());
        getChildren().add(createDescriptionTitle());
        getChildren().add(createButtons());

        setAlignment(Pos.CENTER);
        setSpacing(10);
        setPadding(new Insets(20, 20,20,20));
    }

    /**
     * Displays the nick of the client who sent the game request
     * @param nick the nick of the client who sent the game request
     * */
    public void setNick(String nick) {
        btnAccept.setNick(nick);
        btnReject.setNick(nick);
        title.setText("You received a game request from client '" + nick + "'");
    }

    /**
     * Creates a button the user will use for
     * accepting the game request
     * @return accept button
     * */
    private NickButton createAcceptButton() {
        btnAccept = new NickButton(BTN_ACCEPT_TXT);
        btnAccept.setOnMouseClicked(e -> main.acceptGameRequest(btnAccept.getNick()));
        return btnAccept;
    }

    /**
     * Creates a button the user will use for
     * rejecting the game request
     * @return reject button
     * */
    private NickButton createRejectButton() {
        btnReject = new NickButton(BTN_REJECT_TXT);
        btnReject.setOnMouseClicked(e -> {
            main.rejectGameRequest(btnReject.getNick());
            main.displayLobby();
        });
        return btnReject;
    }

    /**
     * Creates both buttons - for accepting
     * and rejecting the game request
     * @return hBox holding both buttons
     * */
    private HBox createButtons() {
        HBox hBox = new HBox();
        hBox.getChildren().addAll(createAcceptButton(), createRejectButton());
        hBox.setAlignment(Pos.CENTER);
        hBox.setSpacing(20);
        return hBox;
    }

    /**
     * Creates the description of the for - who sent the game request etc.
     * @return label - the description of the form
     * */
    private Label createDescriptionTitle() {
        Label desc = new Label(
                "Would you like to accept this challenge" +
                " and play Connect4 against them? You have 30s to " +
                "reply to this request. Otherwise, it will be " +
                "automatically canceled.");
        desc.setWrapText(true);
        return desc;
    }

    /**
     * Creates the title of the form
     * @return title of the form
     * */
    private Label createTitle() {
        title = new Label();
        title.setFont(new Font(LABEL_FONT_SIZE));
        return title;
    }
}