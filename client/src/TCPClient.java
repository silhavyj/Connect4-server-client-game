import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Socket;
import java.util.*;

/**
 * @author silhavyj A147B0362P
 *
 * This clies takes of the TCP connection to the server.
 * If the connection goes off, it will try to repeatedly
 * connect back to the server until either the connection
 * is successfully re-established again or the user changes
 * their mind and cancels it. This class is extended from
 * class Thread.
 * */
public class TCPClient extends Thread {

    /** id of the protocol */
    private static final String PROTOCOL_ID = "silhavyj";
    /** left alignment of length of the message */
    private static final int ALIGN = 4;

    /** message to set the user's nickname */
    private static final String O_NICK          = "NICK";
    /** message to leave the server (on purpose) */
    private static final String O_EXIT          = "EXIT";
    /** message to send a game request to another client */
    private static final String O_RQ            = "RQ";
    /** message to cancel a game request */
    private static final String O_RQ_CANCELED   = "RQ_CANCELED";
    /** message to either accept or reject a game request */
    private static final String O_RPL           = "RPL";
    /** message to cancel the game */
    private static final String O_GAME_CANCELED = "GAME_CANCELED";
    /** message to play the game (one turn) */
    private static final String O_GAME_PLAY     = "GAME_PLAY";

    /**
     * when the connection goes off this is the sleep period
     * of the main thread handling the client
     * (until the connection is re-established again)
     * */
    private static final int THREAD_IS_PAUSED_SLEEP_MS = 10;

    /** reference to the main class of the application */
    private Main main;
    /** ip address of the server */
    private String ip;
    /** port the server runs on*/
    private int port;
    /** nickname of the user (client) */
    private String nick;

    /** flag is the client is disconnected */
    private boolean disconnect;
    /** reader used to read messages from the socket */
    private BufferedReader input;
    /** printer used to send messages through the socket */
    private PrintWriter output;
    /** socket used for the communication with the server */
    private Socket socket;
    /** thread handling ping messages */
    private HeartBeat heartBeat;

    /** flag is the main thread handling the client is paused */
    private boolean mainThreadPaused;
    /** flag if the main thread handling the client should be run again */
    private boolean starting;
    /** flag which client is up (when playing a game) */
    private boolean player1;
    /** flag is the client is playing a game */
    private boolean gameIsOn;
    /** flag if the user already accepted a game request (they cannot spam the button) */
    private boolean previouselyAccepted;

    /** Different states of the client */
    private enum State {
        LOBBY,    // the client is in the lobby
        GAME_RQ,  // the client sent or received a game request
        GAME      // the client is playing a game
    }

    /** current state of the client */
    private State state;

    /**
     * This internal class takes care of ping messages
     * that are being sent in order to find out whether
     * or not the server is still online. This class is
     * extended from class Thread.
     * */
    private class HeartBeat extends Thread {

        /** delay between sending a ping message (delay of the thread) */
        private static final long HEART_BEAT_DELAY_MILLIS = 2500;
        /** time within witch a response to the ping message should be received */
        private static final int WAITING_FOR_PING_REPLY_SEC = 6;
        /** the ping message itself */
        private static final String PING = "PING";
        /** flag is the trying is attempting to connect to the server */
        private boolean connecting;
        /** flag if a response to the ping massage has been received */
        private boolean receivedPing;

        /** Constructor of the class - creates an instance of it */
        public HeartBeat() {
            // log the start of the thread
            Logger.getInstance().log(Logger.Type.INFO, "creating a heart beat of the TCP client");
            connecting = true;
            receivedPing = false;
        }

        /**
         * The body of the thread
         * This class is overridden from class Thread
         * */
        @Override
        public void run() {
            while (disconnect == false) {
                // trying to attempt to the server
                if (connecting) {
                    Logger.getInstance().log(Logger.Type.INFO, "trying to establish a TCP connection to the server");
                    try {
                        socket = new Socket(ip, port);
                        input = new BufferedReader(new InputStreamReader(socket.getInputStream()));
                        output = new PrintWriter(socket.getOutputStream(), false);
                        connecting = false;
                        runThread();
                    } catch (IOException e) {
                    }
                }
                // sending ping messages
                else {
                    Logger.getInstance().log(Logger.Type.INFO, "sending a PING message to the server");
                    try {
                        receivedPing = false;
                        String pingMSg = PROTOCOL_ID + String.format("%0" + ALIGN + "d", PING.length()) + PING + "\r";
                        System.out.println(pingMSg);
                        socket.getOutputStream().write(pingMSg.getBytes());
                        socket.getOutputStream().flush();

                        // waiting for a response to the ping message
                        for (int i = 0; i < WAITING_FOR_PING_REPLY_SEC; i++) {
                            System.out.println("waiting..." +  (WAITING_FOR_PING_REPLY_SEC - i) + "s");
                            if (receivedPing == true)
                                break;
                            sleep(1000);
                        }

                        // if the server is not responding
                        // start attempting to connect to it again
                        if (!receivedPing) {
                            main.displayConnectingForm();
                            connecting = true;
                        }
                    } catch (IOException | InterruptedException e) {
                        Logger.getInstance().log(Logger.Type.WARNING, "Server is now offline");
                        main.displayConnectingForm();
                        connecting = true;
                    }
                }
                // delay of the thread
                try {
                    sleep(HEART_BEAT_DELAY_MILLIS);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        }
    }

    /**
     * Constructor of the class - creates an instance of it
     * @param main reference to the main class of the application
     * @param ip ip address of the server
     * @param port port the server runs on
     * @param nick the user's nickname
     * */
    public TCPClient(Main main, String ip, int port, String nick) {
        // log the start of the client
        Logger.getInstance().log(Logger.Type.INFO, "creating a new TCP client");
        this.main = main;
        this.ip = ip;
        this.nick = nick;
        this.port = port;

        // initialize all variables
        disconnect = false;
        mainThreadPaused = false;
        starting = true;
        player1 = false;
        gameIsOn = false;
        previouselyAccepted = false;

        // start attempting to connect to the server
        heartBeat = new HeartBeat();
        heartBeat.setDaemon(true);
        heartBeat.start();
    }

    /**
     * Sends a message off to the server
     * @param msg message that is going to be send to the server
     * */
    private void sendMessage(String msg) {
        msg = PROTOCOL_ID + String.format("%0" + ALIGN + "d", msg.length()) + msg + "\r";
        Logger.getInstance().log(Logger.Type.MSG, "Sending a message to the client: " + msg);
        if (output != null) {
            output.write(msg);
            output.flush();
        }
    }

    /**
     * Starts the main thread handling the client
     * */
    private void runThread() {
        sendMessage(O_NICK + " " + nick);
        main.displayLobby();
        mainThreadPaused = false;
        state = State.LOBBY;
        if (starting == true) {
            starting = false;
            setDaemon(true);
            start();
        }
    }

    /**
     * Closes the connecting to the server.
     * It closes both the input and output as
     * well as the socket itself
     * */
    public void closeConnection() {
        Logger.getInstance().log(Logger.Type.INFO, "closing the TCP connection to the server from client");
        disconnect = true;
        sendMessage(O_EXIT);
        try {
            if (socket != null)
                socket.close();
            if (input != null)
                input.close();
            if (output != null)
                output.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    /**
     * The main thread handing the client
     * receiving messages from the server
     * */
    @Override
    public void run() {
        String buffer;
        String[] tokens;

        while (true) {
            // if the thread is paused - the server
            // is most likely down
            while (mainThreadPaused == true) {
                try {
                    sleep(THREAD_IS_PAUSED_SLEEP_MS);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
            // otherwise accept messages
            // from the server
            try {
                if (input.ready()) {
                    // if there is something to read
                    buffer = input.readLine().trim();
                    if (buffer.length() == 0)
                        continue;

                    // read the message, log it, and split the message up
                    Logger.getInstance().log(Logger.Type.MSG, "received msg from the server: " + buffer);
                    String protocolId = buffer.substring(0, PROTOCOL_ID.length());
                    if (protocolId.equals(PROTOCOL_ID) == false) {
                        Logger.getInstance().log(Logger.Type.ERROR, "the server sent an invalid message: " + buffer);
                        System.exit(0);
                    }
                    buffer = buffer.substring(PROTOCOL_ID.length() + ALIGN, buffer.length());
                    
                    tokens = buffer.split(" ");
                    if (isValidMessage(tokens) == false) {
                        Logger.getInstance().log(Logger.Type.ERROR, "the server sent an invalid message: " + buffer);
                        System.exit(0);
                    }

                    // test if the message is valid
                    ReceivedMsg msg = getMsg(tokens[0]);
                    if (msg == ReceivedMsg.INVALID_PROTOCOL) {
                        System.exit(0);
                        return;
                    }
                    // if the message is just an acknowledgement - ignore it
                    if (msg == ReceivedMsg.OK) {
                        heartBeat.receivedPing = true;
                        continue;
                    }
                    // set of prohibited messages (depends on the state
                    // of the client)
                    HashSet<ReceivedMsg> prohibitedMsgs = new HashSet<>();
                    switch (state) {
                        case LOBBY:
                            prohibitedMsgs.addAll(Arrays.asList(
                                    ReceivedMsg.GAME_PLAY,
                                    ReceivedMsg.GAME_CANCELED,
                                    ReceivedMsg.GAME_MSG,
                                    ReceivedMsg.GAME_WINNING_TAILS,
                                    ReceivedMsg.GAME_RESULT));
                            break;
                        case GAME_RQ:
                            prohibitedMsgs.addAll(Arrays.asList(
                                    ReceivedMsg.GAME_RECOVERY,
                                    ReceivedMsg.RQ,
                                    ReceivedMsg.GAME_PLAY,
                                    ReceivedMsg.GAME_CANCELED,
                                    ReceivedMsg.GAME_MSG,
                                    ReceivedMsg.GAME_WINNING_TAILS,
                                    ReceivedMsg.GAME_RESULT
                            ));
                            break;
                        case GAME:
                            prohibitedMsgs.addAll(Arrays.asList(
                                    ReceivedMsg.RQ_CANCELED,
                                    ReceivedMsg.RQ,
                                    ReceivedMsg.GAME_START));
                            break;
                    }
                    // if the received message is on the list of prohibited
                    // messages - the server is not following the protocol
                    if (processCommand(msg, tokens, prohibitedMsgs) == false) {
                        Logger.getInstance().log(Logger.Type.ERROR, "the server is not following the protocol " + buffer);
                        System.exit(0);
                        return;
                    }
                    // change the current state of the client
                    // depending on the current state + the message
                    switch (msg) {
                        case GAME_START:
                            state = State.GAME;
                            break;
                        case GAME_CANCELED:
                        case RQ_CANCELED:
                            state = State.LOBBY;
                            break;
                        case RQ:
                            state = State.GAME_RQ;
                            break;
                    }
                }
            } catch (Exception e) {
               // e.printStackTrace();
                Logger.getInstance().log(Logger.Type.INFO, "Thread handling client '" + nick + "' closes");
                mainThreadPaused = true;
            }
        }
    }

    /**
     * Processes a message received from the server
     * @param msg message (enumeration) received from the server
     * @param tokens the raw message split up into tokens
     * @param prohibited set of prohibited messages
     * @return false, if the message is on the list of prohibited messages, true otherwise
     * */
    private boolean processCommand(ReceivedMsg msg, String[] tokens, HashSet<ReceivedMsg> prohibited) {
        // check if it is allowed to
        // execute the message received from the server
        if (prohibited.contains(msg))
            return false;

        switch (msg) {
            case GAME_RECOVERY:
                main.gameRecovery(getRecoveryData(tokens));
                break;
            case ADD_CLIENT:
                main.addNewOnlineUser(tokens[1]);
                break;
            case REMOVE_CLIENT:
                main.removeOnlineUser(tokens[1]);
                break;
            case GAME_PLAYER_STATE:
                main.changeStateOfUser(tokens[1], tokens[2].equals("ON"));
                break;
            case RQ:
                player1 = false;
                main.displayReceivedGameRequestForm(tokens[1]);
                break;
            case RQ_CANCELED:
                main.displayLobby();
                break;
            case GAME_START:
                gameIsOn = true;
                main.gameStart(tokens[1]);
                break;
            case GAME_PLAY:
                int y = Integer.parseInt(tokens[2]);
                int x = Integer.parseInt(tokens[3]);
                main.playGame(y, x, player1 ? tokens[1].equals(nick) : !tokens[1].equals(nick));
                break;
            case GAME_CANCELED:
                gameIsOn = false;
                previouselyAccepted = false;
                main.gameCanceled(getMessageFromTokens(tokens));
                break;
            case GAME_MSG:
                main.setGameMsg(getMessageFromTokens(tokens));
                break;
            case GAME_WINNING_TAILS:
                gameIsOn = false;
                main.colorWinningTails(getCoordinates(tokens));
                break;
            case GAME_RESULT:
                main.setGameResultMsg(getMessageFromTokens(tokens));
                break;
        }
        return true;
    }

    /**
     * Processes data for game recovery.
     * It parses it from the messages and returns it as a list of
     * coordinates of individual disks on the board.
     * @param tokens message split up into tokens
     * @return list of coordinates of the disks on the board
     * */
    private List<int[]> getRecoveryData(String[] tokens) {
        List<int[]> data = new ArrayList<>();
        int y = 0;
        int x = 0;
        int value;

        for (int i = 1; i <= Connect4.ROWS * Connect4.COLUMNS; i++) {
            value = Integer.parseInt(tokens[i]);
            if (value != 0)
                data.add(new int[] { y, x, value });
            x++;
            if (i % Connect4.COLUMNS == 0) {
                y++;
                x = 0;
            }
        }
        return data;
    }

    /**
     * Returns list of coordinates used when
     * highlighting the winning disks (tails)
     * @param tokens message split up into tokens
     * @return list of coordinates
     * */
    private List<int[]> getCoordinates(String[] tokens) {
        List<int[]> coordinates = new ArrayList<>();
        for (int i = 1; i < tokens.length; i++) {
            int[] tmp = new int[2];
            tmp[0] = Integer.parseInt(tokens[i]);
            tmp[1] = Integer.parseInt(tokens[i+1]);
            coordinates.add(tmp);
            i++;
        }
        return coordinates;
    }

    /**
     * Extract the message ("announcement") from the raw message
     * received from the server
     * @param tokens message split up into tokens
     * @return message ("announcement") extract from the raw message
     * */
    private String getMessageFromTokens(String[] tokens) {
        StringBuilder ss = new StringBuilder();
        for (int i = 1; i < tokens.length; i++) {
            ss.append(tokens[i]);
            if (i < tokens.length - 1)
                ss.append(" ");
        }
        return ss.toString();
    }

    /**
     * "Converts" a string message into an enumeration
     * @param msgStr message e.g. "RQ"
     * @return associated enumeration (if exists - null otherwise)
     * */
    private ReceivedMsg getMsg(String msgStr) {
        for (ReceivedMsg msg : ReceivedMsg.values())
            if (msgStr.equals(msg.toString()))
                return msg;
        return null;
    }

    /**
     * Validates the message split up into tokens
     * given as a parameter (tokens[0] - type of the message)
     * @param tokens message split up into tokens
     * @return true if the message is valid, false otherwise
     * */
    private boolean isValidMessage(String[] tokens) {
        for (ReceivedMsg msg : ReceivedMsg.values())
            if (tokens[0].equals(msg.toString())) {
                return msg.isValid(tokens);
            }
        return false;
    }

    /**
     * Accepts the received game request (from another client)
     * @param nick nick of the client who sent the game request
     * */
    public void acceptGameRequest(String nick) {
        if (previouselyAccepted == false) {
            previouselyAccepted = true;
            player1 = false;
            sendMessage(O_RPL + " " + nick + " YES");
        }
    }

    /**
     * Rejects the received game request (from another client)
     * @param nick nick of the client who sent the game request
     * */
    public void rejectGameRequest(String nick) {
        state = State.LOBBY;
        sendMessage(O_RPL + " " + nick + " NO");
    }

    /**
     * Sends a game request to another client
     * @param nick nick of the client who the game request is going to be sent to
     * */
    public void sendGameRequest(String nick) {
        state = State.GAME_RQ;
        player1 = true;
        sendMessage(O_RQ + " " + nick);
    }

    /**
     * Cancels the game request the user sent to another client
     * @param nick nick of the client the game request was sent to
     * */
    public void cancelGameRequest(String nick) {
        state = State.LOBBY;
        sendMessage(O_RQ_CANCELED + " " + nick);
    }

    /**
     * Cancels the game that is being played
     * */
    public void cancelGame() {
        if (gameIsOn) {
            gameIsOn = false;
            sendMessage(O_GAME_CANCELED);
        }
    }

    /**
     * Plays the game (one turn)
     * @param xPosition x position where the user wants to place their disk
     * */
    public void playGame(int xPosition) {
        if (gameIsOn)
            sendMessage(O_GAME_PLAY + " " + xPosition);
    }
}