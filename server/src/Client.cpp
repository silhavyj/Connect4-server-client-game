#include "Client.h"

const std::string Client::UNDEFINED_NICK = "UNDEFINED_NICK";

Client::Client(int socket, std::string ip, std::string protocolId) {
    this->socket = socket;
    this->ip = ip;
    this->protocolId = protocolId;

    this->state = NICK;

    handlingThreadRunning = true;
    nick = UNDEFINED_NICK; // initially, the nick is UNDEFINED
}

Client::~Client() {
    close(socket);  // closes the socket
}

std::string Client::leftAlign(int n) const {
    if (n >= 0 && n <= 9) return "000" + std::to_string(n);
    if (n >= 10 && n <= 99) return "00" + std::to_string(n);
    if (n >= 100 && n <= 999) return "0" + std::to_string(n);
    return std::to_string(n);
}

void Client::sendMessage(std::string msg) const {
    msg = protocolId + leftAlign(msg.length()) + msg;
    LOG_MSG("sending a message to client " + toStr() + ": '" + msg + "'");
    msg += "\r\n"; //add '\r\n' at the end of every message
  
    if (msg.length() > BUFF_SIZE) {
        LOG_ERR("The message the server is trying to send off to the client ('" + getNick() + "') is too long");
        return;
    }

    char buff[BUFF_SIZE];
    strcpy(buff, msg.c_str());

    size_t len = strlen(buff);
    char *pos = buff;
    ssize_t numberOfSentBytes;

    while (len > 0 && (numberOfSentBytes = send(socket, pos, len, 0)) > 0) {
        pos += numberOfSentBytes;
        len -= (size_t)numberOfSentBytes;
    }
    if (len > 0 || numberOfSentBytes < 0) {
        LOG_ERR("error when sending a message to client " + nick + ": " + msg);
    }
}

bool Client::isHandlingThreadRunning() const {
    return handlingThreadRunning;
}

void Client::setHandlingThreadRunning(bool state) {
    handlingThreadRunning = state;
}

int Client::getSocket() const {
    return socket;
}

std::string Client::getNick() const {
    return nick;
}

void Client::setNick(std::string nick) {
    this->nick = nick;
}

Client::State Client::getState() const {
    return state;
}

void Client::setState(State state) {
    this->state = state;
}

std::string Client::toStr() const {
    return "[nick='" + nick + "' | ip=" + ip + "]";
}

void Client::setReceivedPing(bool value) {
    receivedPing = value;
}

bool Client::getReceivedPing() const {
    return receivedPing;
}