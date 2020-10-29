import javafx.geometry.Insets;
import javafx.geometry.Pos;
import javafx.scene.control.*;
import javafx.scene.layout.HBox;
import javafx.scene.layout.VBox;
import javafx.scene.text.Font;

import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * @author silhavyj A147B0362P
 *
 * This class represents the lgin form the user
 * uses to connect to the server - fills out all
 * the information such as their nick, ip adress,
 * and port. This class extends class VBox (JavaFX)
 * */
public class LoginForm extends VBox {

    /** total width of the window */
    public static final int FORM_WIDTH = 325;
    /** total height of the window */
    public static final int FORM_HEIGHT  = 275;
    /** font size of the tile of the window */
    private static final int TITLE_FONT_SIZE = 19;
    /** title of the window */
    private static final String TITLE = "Connection to the server";
    /** width of labels of all the items (nick, ip, ...) */
    private static final int LABEL_WIDTH = 100;
    /** title of the nick label */
    private static final String NICK_TXT = "your nick: ";
    /** title of the ip address label */
    private static final String IP_ADDR_TXT = "IP address: ";
    /** title of the port number label */
    private static final String PORT_NUM_TXT = "port number: ";
    /** title of the button connect to the server */
    private static final String BUTTON_TXT = "Connect";

    /** regex for validation of IPv4 addresses */
    private static final String IPV4_REGEX =
                    "^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\." +
                    "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\." +
                    "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\." +
                    "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$";

    /** Pattern for IPv4 addresses - when it comes to validation */
    private static final Pattern IPv4_PATTERN = Pattern.compile(IPV4_REGEX);

    /** nick nack input text field */
    private TextField nickTxtField;
    /** ip address input text field */
    private TextField iPAddrTxtField;
    /** port number input text field */
    private TextField portNumTxtField;

    /** reference to the main class of the application */
    private Main main;

    /**
     * Constructor of the class - creates an instance of it
     * @param main reference to the main class of the application
     *  */
    public LoginForm(Main main) {
        super();

        this.main = main;

        // creates the content on the window
        getChildren().add(createTitle());
        getChildren().add(createNickItem());
        getChildren().add(createIPAddrItem());
        getChildren().add(createPortNumberItem());
        getChildren().add(createButtonItem());

        setAlignment(Pos.CENTER);
        setSpacing(10);
        setPadding(new Insets(20, 20,20,20));
    }

    /**
     * Validates the nick the user enters into the input field.
     * The nick supposed to be a one word only.
     * @return true if the nick is valid, false otherwise
     *  */
    private boolean isValidNick() {
        String nick = nickTxtField.getText();
        if (nick == null || nick.equals(""))
            return false;
        if (nick.split(" ").length != 1)
            return false;
        return true;
    }

    /**
     * Validates the ip address the user enters into
     * the input field. The ip address is supposed
     * to be an IPv4 format.
     * @return true, if the ip address is a valid IPv4 format,
     * false otherwise
     * */
    private boolean isValidIP() {
        String ipAddr = iPAddrTxtField.getText();
        if (ipAddr == null || ipAddr.equals(""))
            return false;
        Matcher matcher = IPv4_PATTERN.matcher(ipAddr);
        return matcher.matches();
    }

    /**
     * Validates the port number the user enters into
     * the input field. The port is supposed to be an
     * integer 0-65535
     * @return true, if the port is valid, false otherwise
     * */
    private boolean isValidPortNumber() {
        String portNumber = portNumTxtField.getText();
        if (portNumber == null || portNumber.equals(""))
            return false;
        try {
            int val = Integer.parseInt(portNumber);
            if (val < 0 || val > 65535)
                return false;
        } catch (Exception e) {
            return false;
        }
        return true;
    }

    /**
     * Connects to the server.
     * If all the input fields are fill out correctly,
     * the program will start attempting connecting
     * to the server. Otherwise, an error message
     * will be thrown.
     * */
    private void connectToServer() {
        // check if the nick is valid
        if (isValidNick() == false) {
            Alert alert = new Alert(Alert.AlertType.ERROR, "Your nick is supposed to be one not empty word only. For example, 'John'. Please fix this issue before attempting to connect to the server again.", ButtonType.OK);
            alert.setHeaderText("Invalid format of your nick!");
            alert.showAndWait();
            nickTxtField.setText("");
        }
        // check if the ip address is valid
        else if (isValidIP() == false) {
            Alert alert = new Alert(Alert.AlertType.ERROR, "The IP address you entered is not in the IPv4 format. Please fix this issue before attempting to connect to the server again.", ButtonType.OK);
            alert.setHeaderText("Invalid format of the IP address!");
            alert.showAndWait();
            iPAddrTxtField.setText("");
        }
        // check if the port number is valid
        else if (isValidPortNumber() == false) {
            Alert alert = new Alert(Alert.AlertType.ERROR, "The port is supposed to be an integer number between 0 and 65535. Please fix this issue before attempting to connect to the server again.", ButtonType.OK);
            alert.setHeaderText("Invalid format of the port number!");
            alert.showAndWait();
            portNumTxtField.setText("");
        }
        // connect to the server
        else {
            main.createNewTCPClient(iPAddrTxtField.getText(), Integer.parseInt(portNumTxtField.getText()), nickTxtField.getText());
            main.setLobbyNick(nickTxtField.getText());
            main.displayConnectingForm(
                    "Trying to establish a TCP connection " +
                    "to the server. This might\ntake a while since the server" +
                    " may not yet be online.\n\n" +
                    "IP address: " + iPAddrTxtField.getText() + "\n" +
                    "port: " + portNumTxtField.getText() + "\n" +
                    "nick: " + nickTxtField.getText());
        }
    }

    /**
     * Creates the button the user is supposed to click on
     * when they wan to connect to the server.
     * @return button connect
     * */
    private Button createButtonItem() {
        Button btn = new Button(BUTTON_TXT);
        btn.setOnMouseClicked(e -> connectToServer());
        return btn;
    }

    /**
     * Creates the port number items
     * @return hBox containing all the items regarding
     * entering the port number
     * */
    private HBox createPortNumberItem() {
        HBox hBox = new HBox();
        hBox.setAlignment(Pos.CENTER);

        // create the label
        Label lbl = new Label(PORT_NUM_TXT);
        lbl.setPrefWidth(LABEL_WIDTH);

        // create the text field
        portNumTxtField = new TextField();

        hBox.getChildren().add(lbl);
        hBox.getChildren().add(portNumTxtField);
        return hBox;
    }

    /**
     * Creates the ip address items.
     * @return hBox containing all the items regarding
     * entering the ip address
     * */
    private HBox createIPAddrItem() {
        HBox hBox = new HBox();
        hBox.setAlignment(Pos.CENTER);

        // create the label
        Label lbl = new Label(IP_ADDR_TXT);
        lbl.setPrefWidth(LABEL_WIDTH);

        // create the input field
        iPAddrTxtField = new TextField();

        hBox.getChildren().add(lbl);
        hBox.getChildren().add(iPAddrTxtField);
        return hBox;
    }

    /**
     * Creates the nick items.
     * @return hBox containing all the items regarding
     * entering the nick name
     * */
    private HBox createNickItem() {
        HBox hBox = new HBox();
        hBox.setAlignment(Pos.CENTER);

        // creates the label
        Label lbl = new Label(NICK_TXT);
        lbl.setPrefWidth(LABEL_WIDTH);

        // creates the input field
        nickTxtField = new TextField();

        hBox.getChildren().add(lbl);
        hBox.getChildren().add(nickTxtField);
        return hBox;
    }

    /**
     * Creates the tile of the window
     * @return the tile of the window
     * */
    private Label createTitle() {
        Label title = new Label(TITLE);
        title.setFont(new Font(TITLE_FONT_SIZE));
        title.setPadding(new Insets(0,0,10,0));
        return title;
    }
}