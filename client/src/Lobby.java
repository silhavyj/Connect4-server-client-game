import javafx.collections.ObservableList;
import javafx.geometry.Insets;
import javafx.geometry.Pos;
import javafx.scene.Node;
import javafx.scene.control.Button;
import javafx.scene.control.Label;
import javafx.scene.control.ScrollPane;
import javafx.scene.control.Separator;
import javafx.scene.layout.HBox;
import javafx.scene.layout.VBox;
import javafx.scene.paint.Color;
import javafx.scene.shape.Circle;
import javafx.scene.text.Font;

/**
 * @author silhavyj A147B0362P
 *
 * This class represents the lobby where the user can see other
 * clients that are connected to the server and can
 * them a game request if they're not already in a game.
 * This class is extended from class VBox (JavaFx).
 * */
public class Lobby extends VBox {

    /** total width of the form */
    public static final int FORM_WIDTH = 400;
    /** total height of the form */
    public static final int FORM_HEIGHT = 600;

    /** font size of the nick name label */
    private static final int NICK_FONT_SIZE = 22;
    /** label of the button for disconnecting from the server */
    private static final String DISCONNECT_BUTTON_TXT = "Disconnect";
    /** title of the window */
    private static final String TITLE = "Online Users";
    /** width of the title of the window */
    private static final int TITLE_WIDTH = 250;
    /** font size of the tile of the widow */
    private static final int TITLE_FONT_SIZE = 19;
    /** color of a client that is available to play a game */
    private static final Color ONLINE_COLOR = Color.GREEN;
    /** color of a client that already busy playing a game */
    private static final Color IN_GAME_COLOR = Color.ORANGE;
    /** width of the "box" holding information about a user */
    private static final int ACTIVE_USER_LBL_WIDTH = 200;
    /** radius of the "online" circle the user has next to their name */
    private static final int ONLINE_CIRCLE_RADIUS = 10;
    /** title of the button for sending a game request */
    private static final String GAME_REQUEST_BUTTON_TXT = "Play";

    /** title of the window */
    private Label nickLabel = null;
    /** reference to the main class */
    private Main main;

    /**
     * Constructor of the class - creates an instance of it
     * @param main reference to the main class
     * */
    public Lobby(Main main) {
        super();

        this.main = main;

        // creates the content of the window
        getChildren().add(createNick());
        getChildren().add(createTitle());
        getChildren().add(new Separator());
        getChildren().add(createScrollPaneForUsers());
        getChildren().add(new Separator());
        getChildren().add(createDisconnectButton());

        setAlignment(Pos.TOP_CENTER);
        setSpacing(10);
        setPadding(new Insets(20, 20,20,20));
    }

    /**
     * Changes the state of the user given as a parameter.
     * Each client can be either busy playing a game or
     * available to be sent a game request.
     * @param nick nick of the client
     * @param available new state of the client
     * */
    public void changeStateOfUser(String nick, boolean available) {
        Logger.getInstance().log(Logger.Type.INFO, "chaning state of client '" + nick + "' to " + (available ? "available" : "busy (playing a game)"));
        ObservableList<Node> users = ((VBox)(((ScrollPane)(getChildren().get(3))).getContent())).getChildren();
        for (Node node : users) {
            Label lbl = (Label) (((HBox) node).getChildren().get(1));
            if (lbl.getText().equals(nick)) {
                ((Circle)(((HBox) node).getChildren().get(0))).setFill(available ? ONLINE_COLOR : IN_GAME_COLOR);
                (((HBox) node).getChildren().get(2)).setDisable(!available);
                break;
            }
        }
    }

    /**
     * Deletes all the clients at once.
     * This method is used when closing the window so when it
     * is open again there are no users from the previous "action".
     * */
    public void deleteAllUsers() {
        ((VBox)(((ScrollPane)(getChildren().get(3))).getContent())).getChildren().clear();
    }

    /**
     * Creates a new online user and adds it to the list.
     * @param nick nick of the new client that just got connected to the server
     * */
    public void addNewOnlineUser(String nick) {
        Logger.getInstance().log(Logger.Type.INFO, "adding client '" + nick + "' on the list of online clients");
        HBox hBox = new HBox();
        hBox.setSpacing(7);
        hBox.setAlignment(Pos.CENTER_LEFT);
        hBox.getChildren().add(createNewActiveUserOnlineCircle());
        hBox.getChildren().add(createNewActiveUserLabel(nick));
        hBox.getChildren().add(createSendGameRequestButton(nick));
        ((VBox)(((ScrollPane)(getChildren().get(3))).getContent())).getChildren().add(hBox);
    }

    /**
     * Removes a client from the list when they get disconnected from the server
     * @param nick nick of the client that is going to be removed
     * */
    public void removeOnlineUser(String nick) {
        Logger.getInstance().log(Logger.Type.INFO, "remove client '" + nick + "' from the list of online clients");
        ObservableList<Node> users = ((VBox)(((ScrollPane)(getChildren().get(3))).getContent())).getChildren();
        for (Node node : users) {
            Label lbl = (Label) (((HBox) node).getChildren().get(1));
            if (lbl.getText().equals(nick)) {
                users.remove(node);
                break;
            }
        }
    }

    /**
     * Creates a button used to send a game request to the
     * appropriate client.
     * @param nick nick of the client the game request will be sent to
     * @return button used to send a game request to the client
     * */
    private NickButton createSendGameRequestButton(String nick) {
        NickButton btn = new NickButton(GAME_REQUEST_BUTTON_TXT, nick);
        btn.setOnMouseClicked(e -> sendGameRequest(btn.getNick()));
        return btn;
    }

    /**
     * Sends a game request to the client given as a parameter
     * @param nick nick of the client the game request will be sent to
     * */
    private void sendGameRequest(String nick) {
        main.sendGameRequest(nick);
        main.displayWaitingForGameRequestForm(nick);
    }

    /**
     * Creates an "online" circle of an active client
     * @return online circle of an active client
     * */
    private Circle createNewActiveUserOnlineCircle() {
        Circle circle = new Circle(ONLINE_CIRCLE_RADIUS);
        circle.setFill(ONLINE_COLOR);
        return circle;
    }

    /**
     * Creates a new label holding the client's nickname
     * @param nick nickname of the client
     * @return label holding the nickname of the client
     * */
    private Label createNewActiveUserLabel(String nick) {
        Label label = new Label(nick);
        label.setPrefWidth(ACTIVE_USER_LBL_WIDTH);
        return label;
    }

    /**
     * Disconnects the client from the server.
     * This is used when the user wants to leave the server on purpose.
     * */
    private void disconnect() {
        main.disconnectFromServer();
        main.displayLoginForm();
    }

    /**
     * Sets nick of the client
     * @param nick nick of the client
     * */
    public void setNick(String nick) {
        nickLabel.setText(nick);
    }

    /**
     * Creates a button used fro disconnecting for the server.
     * This is the button the user is supposed to click on
     * when they want to leave the server.
     * @return button disconnecting the client from the server
     * */
    private Button createDisconnectButton() {
        Button btn = new Button(DISCONNECT_BUTTON_TXT);
        btn.setOnMouseClicked(e -> disconnect());
        return btn;
    }

    /**
     * Creates a nick of a new client.
     * @return label holding the nick of the client
     * */
    private Label createNick() {
        nickLabel = new Label();
        nickLabel.setFont(new Font(NICK_FONT_SIZE));
        return nickLabel;
    }

    /**
     * Creates the title of the window (lobby)
     * @return title of the window
     * */
    private Label createTitle() {
        Label lbl = new Label(TITLE);
        lbl.setPrefWidth(TITLE_WIDTH);
        lbl.setAlignment(Pos.CENTER);
        lbl.setFont(new Font(TITLE_FONT_SIZE));
        return lbl;
    }

    /**
     * Creates a scroll-pane for all the users so the
     * user can simply scroll down if the list
     * of online clients is too long.
     * @return scroll-pane of the list of clients
     * */
    private ScrollPane createScrollPaneForUsers() {
        ScrollPane scrollPane = new ScrollPane();
        scrollPane.setHbarPolicy(ScrollPane.ScrollBarPolicy.ALWAYS);
        scrollPane.setVbarPolicy(ScrollPane.ScrollBarPolicy.NEVER);

        VBox vBox = new VBox();
        vBox.setSpacing(10);
        vBox.setPadding(new Insets(10,10,10,10));
        vBox.setPrefHeight(500);

        scrollPane.setContent(vBox);
        scrollPane.setPrefHeight(500);
        return scrollPane;
    }
}