import javafx.application.Application;
import javafx.application.Platform;
import javafx.scene.Parent;
import javafx.scene.Scene;
import javafx.scene.layout.BorderPane;
import javafx.stage.Stage;

import java.util.List;

/**
 * @author silhavyj A147B0362P
 *
 * This class is the entry point of the application
 * and works as an interaface between the TCP
 * communication and the graphical elements visualizing
 * the the game. The class exteds class Application (JavaFX).
 * */
public class Main extends Application {

    /** title of the window */
    private static final String TITLE = "Connect 4";

    /** primary stage of the application */
    private Stage primaryStage;
    /** main scene of the application */
    private Scene mainScene;
    /** root node (element) of the application */
    private BorderPane root;

    /** reference to a TCP client */
    private TCPClient tcpClient = null;

    /** reference to the login form of the application */
    private LoginForm loginForm;
    /** reference to the connecting form of the application */
    private ConnectingForm connectingForm;
    /** reference to the lobby of the application */
    private Lobby lobby;
    /** reference to the waiting for game request form of the application */
    private WaitingForGameRequestForm waitingForGameRequestForm;
    /** reference to the received game request form of the application */
    private ReceivedGameRequestForm receivedGameRequestForm;
    /** reference to the game form of the application */
    private Connect4 connect4;

    /**
     * Displays the login form of the application
     * */
    public void displayLoginForm() {
        primaryStage.setWidth(LoginForm.FORM_WIDTH);
        primaryStage.setHeight(LoginForm.FORM_HEIGHT);
        root.setCenter(loginForm);
    }

    /**
     * Start of a new game against the client whose
     * nick if given as a parameter.
     * @param nick nick of the client's opponent
     * */
    public void gameStart(String nick) {
        Platform.runLater(() -> {
            connect4 = new Connect4(this, mainScene, nick);
            primaryStage.setWidth(Connect4.FORM_WIDTH);
            primaryStage.setHeight(Connect4.FORM_HEIGHT);
            root.setCenter(connect4);
        });
    }

    /**
     * Game recovery - when the user loses their connection and
     * then gets connected back to the server within the defined time
     * @param data the last state of the game before the used got disconnected
     * */
    public void gameRecovery(List<int[]> data) {
        Platform.runLater(() -> {
            for (int[] coordinates : data)
                //[y x player(1/2)]
                connect4.createDisk(coordinates[0], coordinates[1], coordinates[2] == 1);
        });
    }

    /**
     * Displays the result of the game
     * @param msg message containing the result of the game
     * */
    public void setGameResultMsg(String msg) {
        Platform.runLater(() -> connect4.setGameResultLabel(msg));
    }

    /**
     * Displays the connecting form of the application
     * */
    public void displayConnectingForm() {
        displayConnectingForm(connectingForm.getConnectingInfo());
    }

    /**
     * Sets the user's name in the title
     * of the lobby of the game
     * @param nick the user's name
     * */
    public void setLobbyNick(String nick) {
        lobby.setNick(nick);
    }

    /**
     * Displays the waiting for game request form
     * of the application.
     * @param nick nick of the client the game request has been sent to
     * */
    public void displayWaitingForGameRequestForm(String nick) {
        waitingForGameRequestForm.setNick(nick);
        primaryStage.setWidth(WaitingForGameRequestForm.FORM_WIDTH);
        primaryStage.setHeight(WaitingForGameRequestForm.FORM_HEIGHT);
        root.setCenter(waitingForGameRequestForm);
    }

    /**
     * Displays the received game request form of the application
     * @param nick nick of the client who sent the game request
     * */
    public void displayReceivedGameRequestForm(String nick) {
        Platform.runLater(() -> {
            receivedGameRequestForm.setNick(nick);
            primaryStage.setWidth(ReceivedGameRequestForm.FORM_WIDTH);
            primaryStage.setHeight(ReceivedGameRequestForm.FORM_HEIGHT);
            root.setCenter(receivedGameRequestForm);
        });
    }

    /**
     * Cancels the currently played game
     * */
    public void cancelGame() {
        tcpClient.cancelGame();
    }

    /**
     * Plays the game (one turn)
     * @param xPosition x position on the board where the player
     * wants to place their disk.
     * */
    public void playGame(int xPosition) {
        tcpClient.playGame(xPosition);
    }

    /**
     * Changes the current state of the client given as a parameter
     * @param nick nick a status of whom is going to be changed
     * @param available the new status of the client
     * */
    public void changeStateOfUser(String nick, boolean available) {
        Platform.runLater(() -> lobby.changeStateOfUser(nick, available));
    }

    /**
     * Cancels the the game (from the server)
     * @param msg msg received from the server (why the was was canceled)
     * */
    public void gameCanceled(String msg) {
        Platform.runLater(() -> connect4.gameCanceled(msg));
    }

    /**
     * Sets a game message (message received from the server)
     * @param msg game message from the server
     * */
    public void setGameMsg(String msg) {
        Platform.runLater(() -> connect4.setGameMessageLbl(msg));
    }

    /**
     * Colors the winning disks (tails) - received from the server
     * @param tails list of coordinates of the winning tails
     * */
    public void colorWinningTails(List<int[]> tails) {
        Platform.runLater(() -> connect4.colorWinningTails(tails));
    }

    /**
     * Removes the client given as a parameter from the list
     * of online client currently connected to the server
     * @param nick nick of the client that is going to be removed
     * */
    public void removeOnlineUser(String nick) {
        Platform.runLater(() -> lobby.removeOnlineUser(nick));
    }

    /**
     * Adds a new clint on the list of online clients
     * currently connected to the server.
     * @param nick nick of the newly-connected client
     * */
    public void addNewOnlineUser(String nick) {
        Platform.runLater(() -> lobby.addNewOnlineUser(nick));
    }

    /**
     * Sends a game request to the client given as a parameter
     * @param nick nick of the client who is the game request
     * going to be sent to
     * */
    public void sendGameRequest(String nick) {
        tcpClient.sendGameRequest(nick);
    }

    /**
     * Cancels the currently sent game request
     * @param nick nick of the client who the game request was sent to
     * */
    public void cancelGameRequest(String nick) {
        tcpClient.cancelGameRequest(nick);
    }

    /**
     * Creates a new TCP client
     * @param ip ip address of the server
     * @param port number of the port the server is running on
     * @param nick the user's nickname
     * */
    public void createNewTCPClient(String ip, int port, String nick) {
        tcpClient = new TCPClient(this, ip, port, nick);
    }

    /**
     * Accepts the game request
     * @param nick nick of the client who sent the game request
     * */
    public void acceptGameRequest(String nick) {
        tcpClient.acceptGameRequest(nick);
    }

    /**
     * Rejects the game request
     * @param nick nick of the client who sent the game request
     * */
    public void rejectGameRequest(String nick) {
        tcpClient.rejectGameRequest(nick);
    }

    /**
     * Displays the connecting form
     * @param connectingInfo information about the connection
     * that is trying to be established (ip, port, ...)
     * */
    public void displayConnectingForm(String connectingInfo) {
        Platform.runLater(() -> {
            primaryStage.setWidth(connectingForm.FORM_WIDTH);
            primaryStage.setHeight(connectingForm.FORM_HEIGHT);
            connectingForm.setConnectionInfo(connectingInfo);
            lobby.deleteAllUsers();
            root.setCenter(connectingForm);
        });
    }

    /**
     * Disconnects from the user from the server
     * */
    public void disconnectFromServer() {
        tcpClient.closeConnection();
        tcpClient = null;
    }

    /**
     * Places a disk on the grid of disks
     * @param y y position of the disk
     * @param x x position of the disk
     * @param player1 true/false if it is player1 or player2
     * */
    public void playGame(int y, int x, boolean player1) {
        Platform.runLater(() -> connect4.createDisk(y, x, player1));
    }

    /**
     * Displays the lobby of the application
     * */
    public void displayLobby() {
        Platform.runLater(() -> {
            primaryStage.setWidth(lobby.FORM_WIDTH);
            primaryStage.setHeight(lobby.FORM_HEIGHT);
            root.setCenter(lobby);
        });
    }

    /**
     * Creates all the forms of the application
     * (lobby, login form, ...). This method is
     * called only once when the application starts
     * */
    private void createAllForms() {
        loginForm = new LoginForm(this);
        connectingForm = new ConnectingForm(this);
        lobby = new Lobby(this);
        waitingForGameRequestForm = new WaitingForGameRequestForm(this);
        receivedGameRequestForm = new ReceivedGameRequestForm(this);
    }

    /**
     * Creates the content of the application - the root
     * element as well as all the forms (login form, ...)
     * @return root element of the application
     * */
    private Parent createContent() {
        createAllForms();
        root = new BorderPane();
        displayLoginForm();
        return root;
    }

    /**
     * Closes the connection to server when
     * the application closes
     * */
    private void closeConnection() {
        if (tcpClient != null)
            tcpClient.closeConnection();
        Logger.getInstance().log(Logger.Type.INFO, "Closing the application");
    }

    /** Method start overridden from the class Application (JavaFX)
     * @param primaryStage primary stage of the application
     * @throws Exception exception if something goes wrong
     * */
    @Override
    public void start(Stage primaryStage) throws Exception {
        this.primaryStage = primaryStage;

        mainScene = new Scene(createContent());
        primaryStage.setScene(mainScene);
        primaryStage.setResizable(false);
        primaryStage.setTitle(TITLE);
        primaryStage.setOnCloseRequest(e -> closeConnection());
        primaryStage.show();
    }

    /** The entry point of the program
     * @param args argument from the command line - not used
     * */
    public static void main(String[] args) {
        launch(args);
    }
}