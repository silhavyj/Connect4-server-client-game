#ifndef SERVER_H
#define SERVER_H

#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <list>
#include <map>
#include <utility>
#include <unordered_map>

#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <signal.h>

#include "Client.h"
#include "Logger.h"
#include "Connect4.h"

// forward declaration
class Client;
class Connect4;

/// A function that is used to split up a string
/// by the character given as a parameter.
///
/// \param str string that is going to be split up
/// \param separator character by which we want to split the string up
/// \return result of splitting up - tokens
std::vector<std::string> split(const std::string& str, char separator);

/// \author silhavyj A17B0362P
///
/// This class represents the server itself.
/// It accepts new clients and handles each of them
/// individually by creating new threads.
class Server {
public:
    /// id of the protocol
    const std::string PROTOCOL_ID = "silhavyj";

    /// default port on which the server runs
    static const int PORT_DEFAULT = 53333;

    /// default number of clients that can be
    /// connected to the server at a time
    static const int MAX_CLIENTS_DEFAULT = 10;

    /// size of the buffer for the messages
    /// received from clients
    static const int BUFF_SIZE = 256;

    /// message separator as defined
    /// in the protocol itself
    static const char MSG_SEPARATOR = ' ';

    /// amount of seconds of waiting for a client to play
    /// (after that, the game will be automatically terminated)
    static const int SECONDS_WAITING_FOR_REPLY_TO_GAME_RQ = 30;

    /// amount of second of waiting for a client to connect back to
    /// the server after they lose their connection while playing a game
    static const int SECONDS_WAITING_FOR_DISCONNECTED_PLAYER = 60;

    /// amount of seconds of waiting for a client to enter their nick
    static const int SECONDS_WAITING_FOR_CLIENT_ENTER_NICK = 10;

    /// amount of second of waiting for a client to send
    /// a ping message (if they do not do so, they probably lost
    /// their connection and will be treated accordingly)
    static const int SECONDS_PING_REPLY = 6;

    /// amount of microseconds as a timeout for the method select
    /// that checks if there is some data ready to read off the socket
    static const int U_SECONDS_SOCKETS_TIMEOUT = 10000;

    /// types of incoming messages from a client
    enum IncomingMsg {
        I_EXIT,            ///< client wants to leave the server
        I_PING,            ///< client sends a ping message
        I_HELP,            ///< client requires to print out help so they can see what commands they can use
        I_GET_NICK,        ///< client requires to find out what their nick actually is
        I_GET_ALL_CLIENTS, ///< client requires to get a list of all the clients connected to the server
        I_GET_STATE,       ///< client requires to find out their current state

        I_NICK,            ///< client sets their nick
        I_GAME_RQ,         ///< client sends a game request to another client
        I_RQ_CANCELED,     ///< client cancels their own game request
        I_RPL,             ///< client accepts/rejects a game request received from another client

        I_GAME_CANCELED,   ///< client cancels the game
        I_GAME_PLAY,       ///< client plays the game

        UNKNOWN            ///< message not specified within the protocol
    };

    /// message sent to a client from the server - invalid protocol
    const std::string O_INVALID_PROTOCOL   = "INVALID_PROTOCOL";
    /// message sent to a client from the server - ok (acknowledgement)
    const std::string O_ACKNOWLEDGE_MSG    = "OK";
    /// message sent to a client from the server - remove a client
    const std::string O_REMOVE_CLIENT      = "REMOVE_CLIENT";
    /// message sent to a client from the server - add a client
    const std::string O_ADD_CLIENT         = "ADD_CLIENT";
    /// message sent to a client from the server - cancel a game request
    const std::string O_RQ_CANCELED        = "RQ_CANCELED";
    /// message sent to a client from the server - received a game request from another client
    const std::string O_RQ_RECEIVED        = "RQ";
    /// message sent to a client from the server - start of a new game
    const std::string O_START_GAME         = "GAME_START";
    /// message sent to a client from the server - game canceled
    const std::string O_GAME_CANCELED      = "GAME_CANCELED";
    /// message sent to a client from the server - one turn in a game (play game)
    const std::string O_GAME_PLAY          = "GAME_PLAY";
    /// message sent to a client from the server - message fro the game
    const std::string O_GAME_MESSAGE       = "GAME_MSG";
    /// message sent to a client from the server - game recovery (client just got back connected to the server after they'd lost their connection)
    const std::string O_GAME_RECOVERY      = "GAME_RECOVERY";
    /// message sent to a client from the server - change state of another client (free/busy playing a game)
    const std::string O_GAME_PLAYER_STATE  = "GAME_PLAYER_STATE";
    /// message sent to a client from the server - list of winning tails of the game
    const std::string O_GAME_WINNING_TILES = "GAME_WINNING_TAILS";
    /// message sent to a client from the server - result of the game (either you've lost or won)
    const std::string O_GAME_GAME_RESULT   = "GAME_RESULT";

private:
    /// connection of the server
    struct Connection_t {
        int port;                   ///< port on which the server runs
        int serverFd;               ///< server socket file descriptor
        struct sockaddr_in address; ///< socket address
    } conn; ///< instance of the server connection

    /// structure holding information
    /// and an incoming message
    struct IncomingMsgInfo {
        IncomingMsg msg;  ///< type of the message (#IncomingMsg)
        bool (*validation)(const std::vector<std::string>& tokens); ///< pointer to a function used for validation of the message
        std::string description; ///< short description of the purpose of the message
    };

    /// structure holding information
    /// about one room (two players playing a game)
    struct GameRoom_t {
        std::string player1; ///< player 1
        std::string player2; ///< player 2
        Connect4 *game;      ///< reference to the game itself
    };

    /// maximum number of clients that can be connected to the server at a time
    int maxClients;
    /// current number of clients connected to the server
    int numberOfClients;
    /// map where the key is an incoming message (as a string)
    /// for example "RQ" and the value is the structure holding
    /// information about that message (#IncomingMsgInfo).
    std::map<std::string, IncomingMsgInfo> msgValidation;

    /// lock used when accessing clients
    std::mutex clientMtx;
    /// map holding all the clients connected to the server
    /// where the key is their nick and the value is a reference to them
    std::unordered_map<std::string, Client *> clients;

    /// lock used when accessing game requests
    std::mutex gameRequestsMtx;
    /// map holding information on who sent a game request to whom
    std::unordered_map<std::string, std::string> gameRequests;

    /// lock used when accessing game rooms
    /// (two players playing a game)
    std::mutex gameRoomsMtx;
    /// map holding information on which player
    /// is in which game room
    std::unordered_map<std::string, GameRoom_t *> gameRooms;

    /// lock used when the list of reconnecting clients
    /// (clients who lost their connection while playing a game)
    std::mutex reconnectingClientsMtx;
    /// map where the key is the player for whom the server is
    /// waiting to reconnect (they lost their connection while playing a game)
    /// and the value their opponent
    std::unordered_map<std::string, std::string> reconnectingClients;

public:
    /// Constructor of the class - creates an instance of it
    /// \param port the port number the server runs on
    /// \param maxClients maximum number of clients that can be connected to the server at a time
    Server(int port, int maxClients);

    /// Boots up the server
    void startServer();

    /// Copy constructor of the class. It was deleted
    /// because there is no need to use it within this project.
    Server(Server &) = delete;

    /// Assignment operator of the the class.
    /// It was deleted because there is no need to use it
    /// within this project.
    void operator=(Server const &) = delete;

    /// Sends a message to the client given as a parameter
    ///
    /// This method is used from the outside of the class
    /// by class #Connect4 when sending a message to players.
    ///
    /// \param nick of the client the message is going to be sent to
    /// \param msg the message itself
    void sendMessage(std::string nick, std::string msg);

    /// Deletes a game room
    ///
    /// This method is called from the outside of the class
    /// by class #Connect4 when terminating the game due to
    /// either of the clients not playing for 30s.
    ///
    /// \param player nick of the player who was supposed to play
    /// \param msgToOtherPlayer message to the other player (what happened)
    /// \param lockReconnectingClients use the lock for accessing the data structure (true/false)
    void deleteGameRoom(std::string player, std::string msgToOtherPlayer, bool lockReconnectingClients);

private:
    /// Creates a new file descriptor (when the server boots up)
    void createFileDescriptor();
    /// Attaches the socket to the port (when the server boots up)
    void attachSocketToPort();
    /// Binds the server (when the server boots up)
    void bindServer();
    /// Sets listening fro clients (when the server boots up)
    void setListening();
    /// Runs the thread accepting connections from clients
    void run();

    /// Thread that handles the client given as a parameter
    /// \param client reference to the client the thread will take care of
    void handleClient(Client *client);

    /// Thread waiting for a client to reply to a game request.
    ///
    /// They have 30s (#SECONDS_WAITING_FOR_REPLY_TO_GAME_RQ) to do so.
    /// If they do not do so, the game request will be automatically
    /// canceled.
    ///
    /// \param sender nick of the client who sent the game request
    /// \param receiver nick of the client who received the game request
    void waitingForReplyToGameRQHandler(std::string sender, std::string receiver);

    /// Thread waiting for a newly-connected client to enter their nick.
    ///
    /// They have 10s (#SECONDS_WAITING_FOR_CLIENT_ENTER_NICK) to do so.
    /// If they do not do so, they are not following the protocol and will
    /// be mercilessly cut off.
    ///
    /// \param client reference to the client that is supposed to enter their nick
    void enteringNickHandler(Client *client);

    /// Returns a message type from the tokens given as a parameter.
    ///
    /// The tokens make up the entire message the client sent off
    /// to the server split up by #MSG_SEPARATOR. The type of
    /// the message should be at position 0.
    ///
    /// \param tokens split up message sent by a client
    /// \return the type of the message (#IncomingMsg) including #UNKNOWN
    IncomingMsg getTypeOfMessage(const std::vector<std::string>& tokens) const;

    /// Removes the client given as a parameter from the
    /// the data structure holding all clients connected to the server.
    ///
    /// This method is used for clients who already entered their
    /// nicks and are therefor recognizable by the nicks.
    ///
    /// \param nick of the client that is going to be deleted
    void removeClientByNick(std::string nick);

    /// Removes the client given as a parameter from the memory
    ///
    /// This method is used when the client has not
    /// entered their nick yet (they are not included
    /// in the data structure holding all clients connected to the server).
    ///
    /// \param client reference to the client that is going to be removed
    void removeClientByReference(Client *client);

    /// Removes the client given as a parameter from the memory
    ///
    /// This method first finds out whether or not the client
    /// can be recognized by their nick and then, it will decide
    /// which on of the methods (#removeClientByReference, #removeClientByNick) it should use.
    ///
    /// \param client reference to the client that is going to be removed
    void removeClient(Client *client);

    /// Sends a broadcast message to all the clients connected to the server
    ///
    /// This method is widely used to notify all the clients when
    /// adding/removing a client to/from the server.
    ///
    /// \param sender nick of the client who is sending the message
    /// \param message the broadcast message itself
    /// \param lock true/false - whether or not to used the lock when accessing
    /// the data structure holding all the clients
    void sendMessageToAllClients(std::string sender, std::string message, bool lock);

    /// Returns nicks of all the clients connected to the server
    ///
    /// This method is used when the client requires to see
    /// the nicks of the clients connected to the server
    /// (#I_GET_ALL_CLIENTS).
    ///
    /// \return nicks of all the clients connected to the server
    std::string getNicksAllClients();

    /// Adds the new client given as a parameter to the
    /// dat structure holding all the clients
    /// \param client the is going to be added to the data structure
    void addNewClient(Client *client);

    /// Finds out if the nick is already taken
    /// \param nick we are curious about whether it already exists on the server or not
    /// \return true, if the nick is taken, false otherwise.
    bool existsClient(std::string nick);

    /// Returns the current state of the client given as a parameter
    /// \param nick of the client
    /// \return the state of the client
    Client::State getStateOfClient(std::string nick);

    /// Sets a state of the client given as a parameter
    /// \param nick of the client
    /// \param state the new state of the client
    void setClientState(std::string nick, Client::State state);

    /// Returns help (a set of commands with their descriptions
    /// the user can perform).
    /// \return the help
    std::string getHelp() const;

    //bool recvNBytes(int socket, char *buff, ssize_t len);

    /// Sends a list of all online clients to the client given as a parameter
    ///
    /// This method is used when the client gets connected
    /// to the server and needs to know who else is online.
    ///
    /// \param client the client the list will be sent to
    void sendOtherOnlineClientsToClient(Client *client);

    /// Sends a list of clients that are currently in a game
    /// to the client given as a parameter
    ///
    /// This method is used when the client gets connected
    /// to the server and needs to know they can send a
    /// game request to.
    ///
    /// \param client the client the list will be sent to
    void sendBusyClientsToClient(Client *client);

    /// Adds a new game request to the appropriate data structure
    /// \param sender client who sent the game request
    /// \param receiver client who received the game request
    void addGameRequest(std::string sender, std::string receiver);

    /// Returns sender of a game request by the receiver
    ///
    /// If such a game request exists, the method will return
    /// the nick of the client who sent it. Otherwise, it will
    /// return UNDEFINED_NICK.
    ///
    /// \param receiver nick of the client who received the game request
    /// \return nick of the client who sent the game request (if exists)
    std::string getGameRequestSender(std::string receiver);

    /// Deletes a game game request from (from the data structure)
    /// \param client nick of the client for whom we want to delete the game request
    void deleteGameRequest(std::string client);

    /// Creates a new game room between the two clients given as a parameter
    /// \param player1 nick of the first client
    /// \param player2 nick of the second client
    void addGameRoom(std::string player1, std::string player2);

    /// Adds a client who just got reconnected back to the game they were playing
    ///
    /// If a clients is playing a game and loses their connection. Their opponent
    /// will wait for 60s (#SECONDS_WAITING_FOR_DISCONNECTED_PLAYER), and if they
    /// get re-connected back to the server, they will be put back in the game,
    /// so they can continue playing it.
    ///
    /// \param player the client who just got re-connected
    /// \param opponent the client's opponent who was waiting for them to get re-connected
    void addToGameRoom(std::string player, std::string opponent);

    /// Returns the player's opponent (nick if exists)
    /// \param player client of whom we want to know the opponent
    /// \param lock true/false - whether or not to use the lock when accessing the data structure
    /// \return nick of the client's opponent.
    std::string getPlayersOpponent(std::string player, bool lock);

    /// Checks if the client's opponent is still waiting fro them in the game
    /// after they lost their connection
    /// \param player client that lost their connection
    /// \return true, if the opponent is still waiting the game. Otherwise, false.
    bool playerStillHasOpponentInGame(std::string player);

    /// Removes the client given as a parameter from the game room
    ///
    /// This method is called either when the client loses their
    /// connection while playing a game or when the game is
    /// canceled on purpose and the whole game room needs to ne
    /// deleted.
    ///
    /// \param player client that is going to be removed from the game room
    void removePlayerFromGameRoom(std::string player);

    /// Checks if the player given as a parameter is still playing a game
    /// \param player client we want to know if they are still in a game
    /// \return true, if the client is still in the game. Otherwise, false.
    bool isPlayerStillInGame(std::string player);

    /// Adds the client given as a parameter to the list of clients
    /// waiting for their connection to get re-establish after they
    /// lose it while playing a game.
    /// \param player client who just lost their connection
    /// \param opponent client's opponent that is still in the game
    void addPlayerToReconnectingList(std::string player, std::string opponent);

    /// Thread waiting for a player (client) to re-connect back to the server
    /// so they can continue playing the game.
    ///
    /// The thread waits for 60s (#SECONDS_WAITING_FOR_DISCONNECTED_PLAYER).
    /// If the client gets re-connected withing this amount of time, they will
    /// will put back in the game. Otherwise, the game will be automatically
    /// terminated.
    ///
    /// \param player client who lost their connection
    /// \param opponent client's opponent that is still waiting in the game
    void waitingForPlayerToConnectBackHandler(std::string player, std::string opponent);

    /// Removes the client given as a parameter from the list of clients
    /// waiting to get re-connected to the server.
    ///
    /// If the client got connected back to the server within those
    /// 60s (#SECONDS_WAITING_FOR_DISCONNECTED_PLAYER), then they will
    /// be put back in the game against their opponent. Otherwise,
    /// the game will be automatically terminated.
    ///
    /// \param player client that is going to be removed fro the list
    /// \param successfullyConnected true/false - if they got successfully
    /// reconnected within the 60s
    void removePlayerFromReconnectingList(std::string player, bool successfullyConnected);

    /// Removes both clients playing a game from the list of
    /// clients waiting to get re-connected back to the server after
    /// they lost their connection.
    /// \param player1 client1 playing the game
    /// \param player2 client2 playing the game
    void removeBothPlayersFromTheReconnectingList(std::string player1, std::string player2);

    /// Returns if the client given as a parameter is on the list of
    /// clients waiting to get re-connected back to the server.
    /// \param player that we want to find out if is on the re-connecting list
    /// \return true, if the client is on the list, false otherwise.
    bool isPlayerOnReconnectingList(std::string player);

    /// Thread that waits for the client given as parameter
    /// to send a ping message every 6s (#SECONDS_PING_REPLY).
    ///
    /// If the client does not do so, the server will think
    /// they just lost their connection and will be treated
    /// accordingly.
    ///
    /// \param client that is supposed to send a ping message every 6s
    void clientPingHandler(Client *client);

    /// Returns an ip address of a client as a string in dotted
    /// decimal notation.
    /// \param address
    /// \return ip address as a string in dotted decimal notation
    std::string ipStr(struct sockaddr_in address) const;

private:
    /// Checks if the message split up into tokens given as a parameter
    /// if a valid #I_EXIT message.
    /// \param tokens message sent by a client split up into tokens
    /// \return true if the message is valid, false otherwise.
    friend bool validExit(const std::vector<std::string>& tokens);

    /// Checks if the message split up into tokens given as a parameter
    /// if a valid #I_PING message.
    /// \param tokens message sent by a client split up into tokens
    /// \return true if the message is valid, false otherwise.
    friend bool validPing(const std::vector<std::string>& tokens);

    /// Checks if the message split up into tokens given as a parameter
    /// if a valid #I_GET_NICK message.
    /// \param tokens message sent by a client split up into tokens
    /// \return true if the message is valid, false otherwise.
    friend bool validGetNick(const std::vector<std::string>& tokens);

    /// Checks if the message split up into tokens given as a parameter
    /// if a valid #I_GET_STATE message.
    /// \param tokens message sent by a client split up into tokens
    /// \return true if the message is valid, false otherwise.
    friend bool validGetState(const std::vector<std::string>& tokens);

    /// Checks if the message split up into tokens given as a parameter
    /// if a valid #I_GET_ALL_CLIENTS message.
    /// \param tokens message sent by a client split up into tokens
    /// \return true if the message is valid, false otherwise.
    friend bool validGetAllClients(const std::vector<std::string>& tokens);

    /// Checks if the message split up into tokens given as a parameter
    /// if a valid #I_NICK message.
    /// \param tokens message sent by a client split up into tokens
    /// \return true if the message is valid, false otherwise.
    friend bool validNick(const std::vector<std::string>& tokens);

    /// Checks if the message split up into tokens given as a parameter
    /// if a valid #I_GAME_RQ message.
    /// \param tokens message sent by a client split up into tokens
    /// \return true if the message is valid, false otherwise.
    friend bool validGameRq(const std::vector<std::string>& tokens);

    /// Checks if the message split up into tokens given as a parameter
    /// if a valid #I_GAME_RQ message.
    /// \param tokens message sent by a client split up into tokens
    /// \return true if the message is valid, false otherwise.
    friend bool validRqCanceled(const std::vector<std::string>& tokens);

    /// Checks if the message split up into tokens given as a parameter
    /// if a valid #I_HELP message.
    /// \param tokens message sent by a client split up into tokens
    /// \return true if the message is valid, false otherwise.
    friend bool validHelp(const std::vector<std::string>& tokens);

    /// Checks if the message split up into tokens given as a parameter
    /// if a valid #I_RPL message.
    /// \param tokens message sent by a client split up into tokens
    /// \return true if the message is valid, false otherwise.
    friend bool validReply(const std::vector<std::string>& tokens);

    /// Checks if the message split up into tokens given as a parameter
    /// if a valid #I_GAME_CANCELED message.
    /// \param tokens message sent by a client split up into tokens
    /// \return true if the message is valid, false otherwise.
    friend bool validGameCanceled(const std::vector<std::string>& tokens);

    /// Checks if the message split up into tokens given as a parameter
    /// if a valid #I_GAME_PLAY message.
    /// \param tokens message sent by a client split up into tokens
    /// \return true if the message is valid, false otherwise.
    friend bool validGamePlay(const std::vector<std::string>& tokens);

    /// Receives len bytes from the socket given as a parameter
    /// \param socket sockent we want to read data from
    /// \param buff buffer
    /// \param len number of bytes we want to read
    /// \return true if the reading is successful, false otherwise
    int recvNBytes(int socket, char *buff, ssize_t len);
};

#endif