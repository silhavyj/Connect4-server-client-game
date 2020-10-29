#ifndef CLIENT_H
#define CLIENT_H

#include <iostream>
#include <unistd.h>
#include <cstring>

#include <sys/types.h>
#include <sys/socket.h>

#include "Logger.h"

/// \author silhavyj A17B0362P
///
/// This class holds information about a client
/// that gets connected to the server. For example,
/// it holds their nick, socket, state, etc.
class Client {
public:
    /// undefined nick (used as a default
    /// value before they enter their nick)
    static const std::string UNDEFINED_NICK;

    /// size of the buffer used when sending
    /// messages to the client
    static const int BUFF_SIZE = 128;

    /// State of the client
    enum State {
        NICK,       ///< the client is supposed to enter their nick
        LOBBY,      ///< the client is in the lobby waiting for a game request, or they can send off one
        SENT_RQ,    ///< the client sent a game request and is waiting for a response from their opponent
        RECV_RQ,    ///< the client received a game request and is supposed to reply to it (accept/reject)
        GAME,       ///< the client is now playing a game
        KILL_THREAD ///< temporary state used for killing all threads associated with the client
    };

private:
    /// socket the client uses for communication with the server
    int socket;
    /// ip address of the client
    std::string ip;
    /// nick of the client
    std::string nick;
    /// state of the client
    State state;
    /// true/false if the main thread of the client
    /// is running or not - used for killing it
    bool handlingThreadRunning;
    /// true/false the client sent a ping message
    /// to the server
    bool receivedPing;
    /// id of the protocol
    std::string protocolId;

public:
    /// Constructor of the class - creates an instance of it
    ///
    /// \param socket the client uses for communication with the server
    /// \param ip address of the client (IPv4)
    /// \param protocolId id of the protocol
    Client(int socket, std::string ip, std::string protocolId);

    /// Destructor of the class - closes the socket used for communication with the server
    ~Client();

    /// Sends a message to the client (from the server)
    /// \param msg messaget that is going to be send off to the client
    void sendMessage(std::string msg) const;

    /// Returns a string representation of the client (IP, nick, ...)
    /// \return string representation of the client
    std::string toStr() const;

    /// Getter of the socket of the client
    ///
    /// This getter is used when adding the socket
    /// to the set of sockets meant to be used by
    /// method select().
    ///
    /// \return the socket of the client
    int getSocket() const;

    /// Getter of the current state of the client
    /// \return the state of the client
    State getState() const;

    /// Setter of the state of the client
    /// \param state - the new state of the client
    void setState(State state);

    /// Getter of the nick of the client
    /// \return nick of the client
    std::string getNick() const;

    /// Setter of the nick of the client
    /// \param nick - the new nick of the client
    void setNick(std::string nick);

    /// Returns true/false whether the main thread
    /// handling the client should continue running
    /// \return false, if the thread should stop. Otherwise, true
    bool isHandlingThreadRunning() const;

    /// Setter of the state of the main thread handling the client
    /// \param state - true/false new state of the thread
    void setHandlingThreadRunning(bool state);

    /// Setter of the state whether or not the client
    /// just sent a ping message to the server
    /// \param value - true/false new state of the variable
    void setReceivedPing(bool value);

    /// Getter of the state whether or not the client
    /// just sent a ping message to the server
    /// \return true, if the client just sent a ping message
    /// to the server. Otherwise, false.
    bool getReceivedPing() const;

    /// Aligns the number given as a parameter
    /// up to four zeros from left.
    ///
    /// \param n number that is going to be aligned
    /// return string (aligned number)
    inline std::string leftAlign(int n) const;
};

#endif