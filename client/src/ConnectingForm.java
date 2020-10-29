import javafx.geometry.Insets;
import javafx.geometry.Pos;
import javafx.scene.control.Button;
import javafx.scene.control.Label;
import javafx.scene.control.ProgressIndicator;
import javafx.scene.layout.VBox;
import javafx.scene.text.Font;

/**
 * @author silhavyj A17B0362P
 *
 * This class represents the window displayed
 * when connecting to the server. It is extended
 * from class VBox (JavaFX).
 * */
public class ConnectingForm extends VBox {

    /** total width of the window */
    public static final int FORM_WIDTH = 500;
    /** total height of the window */
    public static final int FORM_HEIGHT = 350;
    /** title of the window */
    private static final String TITLE_TXT = "Connecting to the server";
    /** font size of the title of the window */
    private static final int TITLE_FONT_SIZE = 19;
    /** title of the button for canceling connection */
    private static final String CANCEL_BTN_TXT = "Cancel";

    /** label holding information about the connection
     *  that is trying to be established */
    private Label connectionInfoLabel;
    /** reference to the main class */
    private Main main;

    /**
     * Constructor of the class - creates an instance of it
     * @param main reference to the main class */
    public ConnectingForm(Main main) {
        super();

        this.main = main;

        // creates the content of the window
        getChildren().add(createTitle());
        getChildren().add(new ProgressIndicator());
        getChildren().add((connectionInfoLabel = new Label()));
        getChildren().add(createCancelButton());

        setAlignment(Pos.CENTER);
        setSpacing(10);
        setPadding(new Insets(20, 20,20,20));
    }

    /**
     * Sets information about the connection
     * @param connectionInfo information about the connection
     * that is going to be displayed in the label
     * */
    public void setConnectionInfo(String connectionInfo) {
        connectionInfoLabel.setText(connectionInfo);
    }

    /**
     * Returns information about the connection from the label
     * @return information (description) about the connection
     * */
    public String getConnectingInfo() {
        return connectionInfoLabel.getText();
    }

    /**
     * Closes the windows when the user click on the button
     * to close the connection (they no longer want to keep
     * trying to connect to the server).
     * */
    private void close() {
        main.displayLoginForm();
        main.disconnectFromServer();
    }

    /**
     * Creates the button to cancel the window
     * @return button to cancel the window
     * */
    private Button createCancelButton() {
        Button btnCancel = new Button(CANCEL_BTN_TXT);
        btnCancel.setOnMouseClicked(e -> close());
        return btnCancel;
    }

    /**
     * Creates the label (title) of the window
     * @return title of the window
     * */
    private Label createTitle() {
        Label title = new Label(TITLE_TXT);
        title.setFont(new Font(TITLE_FONT_SIZE));
        return title;
    }
}