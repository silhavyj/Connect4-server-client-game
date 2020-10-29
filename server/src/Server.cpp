#include "Server.h"

// function prototypes
bool validNick(const std::vector<std::string>& tokens);
bool validGameRq(const std::vector<std::string>& tokens);
bool validExit(const std::vector<std::string>& tokens);
bool validPing(const std::vector<std::string>& tokens);
bool validGetNick(const std::vector<std::string>& tokens);
bool validGetState(const std::vector<std::string>& tokens);
bool validGetAllClients(const std::vector<std::string>& tokens);
bool validRqCanceled(const std::vector<std::string>& tokens);
bool validHelp(const std::vector<std::string>& tokens);
bool validReply(const std::vector<std::string>& tokens);
bool validGameCanceled(const std::vector<std::string>& tokens);
bool validGamePlay(const std::vector<std::string>& tokens);

Server::Server(int port, int maxClients) {
    this->maxClients = maxClients;
    conn.port = port;
    numberOfClients = 0;
    srand(time(0));

    // initialize the table of commands
    msgValidation["EXIT"] = {I_EXIT, &validExit, "leaves the server"};
    msgValidation["PING"] = {I_PING, &validPing, "pings the server (test)"};
    msgValidation["/STATE"] = {I_GET_STATE, &validGetState, "returns client's current state"};
    msgValidation["/NICK"] = {I_GET_NICK, &validGetNick, "returns client's nick"};
    msgValidation["/HELP"] = {I_HELP, &validHelp, "prints out help"};
    msgValidation["/ALL_CLIENTS"] = {I_GET_ALL_CLIENTS, &validGetAllClients, "returns nicks of all clients connected to the server"};

    msgValidation["NICK"] = {I_NICK, &validNick, "<nick> sets the client's nick to the value given as a parameter (one word)"};
    msgValidation["RQ"] = {I_GAME_RQ, &validGameRq, "<nick> sends a game request to the client"};
    msgValidation["RQ_CANCELED"] = {I_RQ_CANCELED, &validRqCanceled, "<nick> cancels the game request sent to the client"};
    msgValidation["RPL"] = {I_RPL, &validReply, "<nick> <YES/NO> accepts/rejects the game request sent from the client"};

    msgValidation["GAME_CANCELED"] = {I_GAME_CANCELED, &validGameCanceled, "exists the current game"};
    msgValidation["GAME_PLAY"] = {I_GAME_PLAY, &validGamePlay, "x plays the game (one move)"};
}

void Server::startServer() {
    LOG_BOOTING("<[STARTING SERVER]>");
    LOG_BOOTING("[port=" + std::to_string(conn.port) + " max clients=" + std::to_string(maxClients) + "]");
    createFileDescriptor();
    attachSocketToPort();
    bindServer();
    setListening();
    run();
}

void Server::createFileDescriptor() {
    LOG_BOOTING("creating a socket file descriptor");
    if ((conn.serverFd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        LOG_ERR("creating the socket file descriptor failed");
        exit(EXIT_FAILURE);
    }
}

void Server::attachSocketToPort() {
    LOG_BOOTING("attaching the socket to the port");
    int opt = 1;
    if (setsockopt(conn.serverFd, SOL_SOCKET, SO_REUSEADDR , &opt, sizeof(opt))) {
        LOG_ERR("attaching the socket to the port failed");
        exit(EXIT_FAILURE);
    }
}

void Server::bindServer() {
    LOG_BOOTING("binding the server");
    conn.address.sin_family = AF_INET;
    conn.address.sin_addr.s_addr = INADDR_ANY;
    conn.address.sin_port = htons(conn.port);

    signal(SIGPIPE, SIG_IGN);

    if (bind(conn.serverFd, (struct sockaddr *)&(conn.address), sizeof(conn.address)) < 0) {
        LOG_ERR("binding the server failed");
        exit(EXIT_FAILURE);
    }
}

void Server::setListening() {
    LOG_BOOTING("setting listening");
    if (listen(conn.serverFd, 5) < 0) {
        LOG_ERR("listening failed");
        exit(EXIT_FAILURE);
    }
}

std::string Server::ipStr(struct sockaddr_in address) const {
    unsigned long addr = address.sin_addr.s_addr;
    char buffer[50];
    sprintf(buffer, "%d.%d.%d.%d",
            (int)(addr  & 0xff),
            (int)((addr & 0xff00) >> 8),
            (int)((addr & 0xff0000) >> 16),
            (int)((addr & 0xff000000) >> 24));
    return std::string(buffer);
}

std::vector<std::string> split(const std::string& str, char separator) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string x;
    while (getline(ss, x, separator))
        if (x != "")
            tokens.emplace_back(x);
    return tokens;
}

void Server::run() {
    LOG_BOOTING("<[ SERVER STARTED ]>");

    int socket;
    int addrLen = sizeof(conn.address);

    while (1) {
        if ((socket = accept(conn.serverFd, (struct sockaddr *)&(conn.address), (socklen_t*)&addrLen)) < 0) {
            LOG_ERR("accepting a socket failed");
            exit(EXIT_FAILURE);
        }
        std::string clientIp = ipStr(conn.address);
        LOG_INFO("new client (" + clientIp + ") just got connected to the server");

        if (numberOfClients == maxClients) {
            LOG_WARNING("the maximum number of clients has been reached");
            LOG_WARNING("disconnecting client " + clientIp + " from the server");
            close(socket);
            continue;
        }
        numberOfClients++;
        Client *client = new Client(socket, clientIp, PROTOCOL_ID);
        std::thread clientHandler(&Server::handleClient, this, client);
        clientHandler.detach();
        usleep(100000);
    }
}

void Server::enteringNickHandler(Client *client) {
    clientMtx.lock();
    std::string clientStr = client->toStr();
    clientMtx.unlock();

    for (int i = 0; i < SECONDS_WAITING_FOR_CLIENT_ENTER_NICK; i++) {
        clientMtx.lock();
        Client::State state = client->getState();
        clientMtx.unlock();

        if (state != Client::NICK) {
            LOG_COUNTDOWN("waiting for client " + clientStr +" was interrupted");
            return;
        }
        int remainingSeconds = SECONDS_WAITING_FOR_CLIENT_ENTER_NICK - i;
        LOG_COUNTDOWN("waiting for client " + clientStr + " to enter their nick (remaining second: " + std::to_string(remainingSeconds) + ")");
        sleep(1);
    }
    LOG_ERR("client " + clientStr + " did not enter their nick within " + std::to_string(SECONDS_WAITING_FOR_CLIENT_ENTER_NICK) + "s");
    clientMtx.lock();
    client->setHandlingThreadRunning(false);
    clientMtx.unlock();
}

void Server::clientPingHandler(Client *client) {
    int counter = 0;
    int remainingTime;
    std::string clientStr = "UNDEFINED_NICK";
    Client::State state;
    
    while (counter != SECONDS_PING_REPLY) {
        clientMtx.lock();
        state = client->getState();
        clientStr = client->toStr();
        clientMtx.unlock();

        if (state == Client::KILL_THREAD) {
            LOG_COUNTDOWN("waiting for client " + clientStr + " to send a PING msg was interrupted");
            return;
        }

        remainingTime = SECONDS_PING_REPLY - counter;
        LOG_COUNTDOWN("waiting for client " + clientStr + " to send a PING msg " + std::to_string(remainingTime) + "s");
        clientMtx.lock();
        if (client->getReceivedPing()) {
            client->setReceivedPing(false);
            counter = 0;
        }
        else counter++;
        clientMtx.unlock();
        sleep(1);
    }
    LOG_COUNTDOWN("client " + clientStr +" has not sent a PING within " + std::to_string(SECONDS_PING_REPLY) + "s");
    clientMtx.lock();
    client->setHandlingThreadRunning(false);
    clientMtx.unlock();
}

int Server::recvNBytes(int socket, char *buff, ssize_t numberOfBytes) {
    char *pos = buff;
    ssize_t receivedBytes;

    while (numberOfBytes > 0 && (receivedBytes = recv(socket, pos, numberOfBytes, 0)) > 0) {
        pos += receivedBytes;
        numberOfBytes -= receivedBytes;
    }
    if (receivedBytes == 0)
        return 1;
    if (numberOfBytes > 0 || receivedBytes < 0)
        return -1;
    *pos = '\0';
    return 0; 
}

void Server::handleClient(Client *client) {
    LOG_INFO("thread handling client " + client->toStr() + " started successfully");
    char buffer[BUFF_SIZE];
    int msgLen = 0;
    std::string receivedMsg;
    std::vector<std::string> tokens;
    IncomingMsg msg;
    std::thread clientWaitingClientHandler;
    std::thread clientReconnectingHandler;
    std::thread clientPingThread;
    std::string receiver;
    std::string sender;
    int xPosition;

    fd_set sockets;
    FD_ZERO(&sockets);
    struct timeval timeout;

    std::thread clientEnteringNickHandler(&Server::enteringNickHandler, this, client);
    clientEnteringNickHandler.detach();

    clientPingThread = std::thread(&Server::clientPingHandler, this, client);
    clientPingThread.detach();

    while (client->isHandlingThreadRunning()) {
        FD_SET(client->getSocket(), &sockets);
        timeout.tv_sec  = 0;
        timeout.tv_usec = U_SECONDS_SOCKETS_TIMEOUT;
        select(FD_SETSIZE, &sockets, NULL, NULL, &timeout);

        if (FD_ISSET(client->getSocket(), &sockets)) {
                msgLen = recvNBytes(client->getSocket(), buffer, PROTOCOL_ID.length());
                if (msgLen != 0) {
                    LOG_ERR("Client ('" + client->getNick() + "') Receiving the protocol id failed");
                    break;
                }
                std::string protocolId(buffer);
                LOG_INFO("Client ('" + client->getNick() + "') - protocolId: " + protocolId);
                if (protocolId != PROTOCOL_ID) {
                    LOG_ERR("Client ('" + client->getNick() + "') - message does not match the protocol id");
                    msgLen = -1;
                    break;
                }
                msgLen = recvNBytes(client->getSocket(), buffer, 4);
                if (msgLen != 0) {
                    LOG_ERR("Client ('" + client->getNick() + "') - Receiving the number of bytes of the message failed");
                    break;
                }
                int len = atoi(buffer);
                LOG_INFO("Client ('" + client->getNick() + "') - len: " + std::to_string(len));
                if (len >= BUFF_SIZE) {
                    LOG_ERR("Client ('" + client->getNick() + "') - The message is too big fro the buffer");
                    msgLen = -1;
                    break;
                }
                msgLen = recvNBytes(client->getSocket(), buffer, len + 1);
                if (msgLen != 0) {
                    LOG_ERR("Client ('" + client->getNick() + "') - Receiving the the message failed");
                    break;
                }
                receivedMsg = std::string(buffer);
                receivedMsg.pop_back();
                if (receivedMsg == "")
                    continue;

                LOG_MSG("received message from client " + client->toStr() + ": '" + receivedMsg + "'");

                tokens = split(receivedMsg, MSG_SEPARATOR);
                msg = getTypeOfMessage(tokens);

                if (msg == UNKNOWN) {
                    client->sendMessage(O_INVALID_PROTOCOL + " unknown message");
                    LOG_ERR("client " + client->toStr() + " sent an unknown message: '" + receivedMsg + "'");

                    if (client->getState() == Client::GAME)
                        deleteGameRoom(client->getNick(), "your opponent was not following the protocol and was kicked out of the server", true);
                    if (client->getState() == Client::SENT_RQ)
                        sendMessageToAllClients(receiver, O_GAME_PLAYER_STATE + " " + receiver + " ON", true);
                    if (client->getState() == Client::RECV_RQ) {
                        sender = getGameRequestSender(client->getNick());
                        sendMessageToAllClients(sender, O_GAME_PLAYER_STATE + " " + sender + " ON", true);
                    }
                    if (client->getState() == Client::NICK) {
                        Client::State state = client->getState();
                        client->setState(Client::KILL_THREAD);
                        sleep(2);
                        client->setState(state);
                    }
                    client->setHandlingThreadRunning(false);

                    removeClient(client);
                    return;
                }
                if (msg == I_EXIT) {
                    sendMessageToAllClients(client->getNick(), O_REMOVE_CLIENT + " " + client->getNick(), false);
                    if (client->getState() == Client::GAME)
                        deleteGameRoom(client->getNick(), "your opponent has suddenly left the server (on purpose)", true);
                    client->sendMessage(O_ACKNOWLEDGE_MSG);
                    if (client->getState() == Client::SENT_RQ)
                        sendMessageToAllClients(receiver, O_GAME_PLAYER_STATE + " " + receiver + " ON", true);
                    if (client->getState() == Client::RECV_RQ) {
                        sender = getGameRequestSender(client->getNick());
                        sendMessageToAllClients(sender, O_GAME_PLAYER_STATE + " " + sender + " ON", true);
                    }
                    if (client->getState() == Client::SENT_RQ || client->getState() == Client::RECV_RQ)
                        deleteGameRequest(client->getNick());
                        
                    Client::State state = client->getState();
                    client->setState(Client::KILL_THREAD);
                    sleep(2);
                    client->setState(state);
                
                    removeClient(client);
                    return;
                }
                else if (msg == I_PING) {
                    client->sendMessage(O_ACKNOWLEDGE_MSG);
                    client->setReceivedPing(true);
                }
                else if (msg == I_GET_STATE)
                    client->sendMessage(std::to_string(client->getState()));
                else if (msg == I_GET_ALL_CLIENTS)
                    client->sendMessage(getNicksAllClients());
                else if (msg == I_GET_NICK)
                    client->sendMessage(client->getNick());
                else if (msg == I_HELP)
                    client->sendMessage(getHelp());
                else {
                    switch (client->getState()) {
                        case Client::NICK:
                            if (msg != I_NICK) {
                                LOG_ERR("client " + client->toStr() + " is not following the protocol by not setting their name first");
                                client->sendMessage(O_INVALID_PROTOCOL + " you are supposed to set your nick first");
                                removeClient(client);
                                return;
                            }
                            if (existsClient(tokens[1])) {
                                LOG_ERR("client " + client->toStr() + " is trying to set their nick to a name that already exists ('" + tokens[1] + "'). Closing their connection.");
                                client->setState(Client::KILL_THREAD);
                                sleep(2);
                                removeClientByReference(client);
                                return;
                            }
                            client->setNick(tokens[1]);
                            addNewClient(client);
                            client->setState(Client::LOBBY);
                            client->sendMessage(O_ACKNOWLEDGE_MSG);

                            sendOtherOnlineClientsToClient(client);
                            sendBusyClientsToClient(client);

                            LOG_INFO("client " + client->toStr() + " just set their nick to '" + client->getNick() + "'");

                            if (isPlayerOnReconnectingList(client->getNick()))
                                removePlayerFromReconnectingList(client->getNick(), true);
                            break;
                        case Client::LOBBY:
                            if (msg != I_GAME_RQ) {
                                LOG_ERR("client " + client->toStr() + " is in the lobby and not sending a game request to another client");
                                client->sendMessage(O_INVALID_PROTOCOL + " in the lobby, you're supposed to send a game request to another player");
                                removeClient(client);
                                return;
                            }
                            if (existsClient(tokens[1]) == false) {
                                LOG_ERR("client " + client->toStr() + " is attempting to send a game request to client '" + tokens[1] + "' that does not exist");
                                client->sendMessage(O_INVALID_PROTOCOL + " there is no client with nick '" + tokens[1] + "'");
                                removeClient(client);
                                return;
                            }
                            if (tokens[1] == client->getNick()) {
                                LOG_ERR("client " + client->toStr() + " is attempting to send a game request to himself");
                                client->sendMessage(O_INVALID_PROTOCOL + " you cannot send a game request to yourself");
                                removeClient(client);
                                return;
                            }
                            if (getStateOfClient(tokens[1]) != Client::LOBBY) {
                                LOG_ERR("client " + client->toStr() + " is attempting to send a game request to client '" + tokens[1] + "' that is now already playing a game");
                                client->sendMessage(O_INVALID_PROTOCOL + " you cannot send a game request to a client that is already playing a game");
                                removeClient(client);
                                return;
                            }
                            receiver = tokens[1];
                            client->setState(Client::SENT_RQ);
                            setClientState(receiver, Client::RECV_RQ);

                            client->sendMessage(O_ACKNOWLEDGE_MSG);
                            sendMessage(receiver, O_RQ_RECEIVED + " " + client->getNick());

                            sendMessageToAllClients(client->getNick(), O_GAME_PLAYER_STATE + " " + client->getNick() + " OFF", true);
                            sendMessageToAllClients(receiver, O_GAME_PLAYER_STATE + " " + receiver + " OFF", true);

                            addGameRequest(client->getNick(), receiver);
                            addGameRequest(receiver, client->getNick());

                            clientWaitingClientHandler = std::thread(&Server::waitingForReplyToGameRQHandler, this, client->getNick(), receiver);
                            clientWaitingClientHandler.detach();
                            break;
                        case Client::SENT_RQ:
                            if (msg != I_RQ_CANCELED) {
                                LOG_ERR("client " + client->toStr() + " is supposed to either wait for a reply to the game request or cancel it");
                                client->sendMessage(O_INVALID_PROTOCOL + " you can either cancel the request or wait for a reply from the other player");
                                sendMessageToAllClients(receiver, O_GAME_PLAYER_STATE + " " + receiver + " ON", true);
                                removeClient(client);
                                return;
                            }
                            if (existsClient(tokens[1]) == false) {
                                LOG_ERR("client " + client->toStr() + " is attempting to cancel a game request from client '" + tokens[1] + "' that does not exist");
                                client->sendMessage(O_INVALID_PROTOCOL + " there is no client with nick '" + tokens[1] + "'");
                                sendMessageToAllClients(receiver, O_GAME_PLAYER_STATE + " " + receiver + " ON", true);
                                removeClient(client);
                                return;
                            }
                            if (receiver != tokens[1]) {
                                LOG_ERR("client " + client->toStr() + " is attempting to cancel someone else's game request - client '" + tokens[1] + "'");
                                client->sendMessage(O_INVALID_PROTOCOL + " you can only cancel your own game request");
                                sendMessageToAllClients(receiver, O_GAME_PLAYER_STATE + " " + receiver + " ON", true);
                                removeClient(client);
                                return;
                            }
                            client->sendMessage(O_ACKNOWLEDGE_MSG);
                            sendMessage(tokens[1], O_RQ_CANCELED + " " + client->getNick());
                            client->setState(Client::LOBBY);
                            setClientState(tokens[1], Client::LOBBY);

                            sendMessageToAllClients(client->getNick(), O_GAME_PLAYER_STATE + " " + client->getNick() + " ON", true);
                            sendMessageToAllClients(receiver, O_GAME_PLAYER_STATE + " " + receiver + " ON", true);
                            break;
                        case Client::RECV_RQ:
                            sender = getGameRequestSender(client->getNick());
                            if (msg != I_RPL) {
                                LOG_ERR("client " + client->toStr() + " is supposed to reply to the game request (accept or reject)");
                                client->sendMessage(O_INVALID_PROTOCOL + " you're supposed to reply to the game request");
                                sendMessageToAllClients(sender, O_GAME_PLAYER_STATE + " " + sender + " ON", true);
                                removeClient(client);
                                return;
                            }
                            if (existsClient(tokens[1]) == false) {
                                LOG_ERR("client " + client->toStr() + " is attempting to reply to a game request from client '" + tokens[1] + "' that does not exist");
                                client->sendMessage(O_INVALID_PROTOCOL + " there is no client with nick '" + tokens[1] + "'");
                                sendMessageToAllClients(sender, O_GAME_PLAYER_STATE + " " + sender + " ON", true);
                                removeClient(client);
                                return;
                            }
                            if (sender != tokens[1]) {
                                LOG_ERR("client " + client->toStr() + " is attempting to reply to a game request from client '" + tokens[1] + "' that did not send him the game request");
                                client->sendMessage(O_INVALID_PROTOCOL + " client '" + tokens[1] + "' did not send you the game request");
                                sendMessageToAllClients(sender, O_GAME_PLAYER_STATE + " " + sender + " ON", true);
                                removeClient(client);
                                return;
                            }
                            if (tokens[2] == "YES") {
                                client->setState(Client::GAME);
                                setClientState(sender, Client::GAME);

                                client->sendMessage(O_START_GAME + " " + sender);
                                sendMessage(sender, O_START_GAME + " " + client->getNick());

                                addGameRoom(sender, client->getNick());
                                LOG_GAME("a game between clients '" + sender + "' and '" + client->getNick() + "' just started");
                            }
                            else if (tokens[2] == "NO") {
                                sendMessage(tokens[1], O_RQ_CANCELED + " " + client->getNick());
                                client->setState(Client::LOBBY);
                                setClientState(tokens[1], Client::LOBBY);
                                client->sendMessage(O_ACKNOWLEDGE_MSG);

                                sendMessageToAllClients(client->getNick(), O_GAME_PLAYER_STATE + " " + client->getNick() + " ON", true);
                                sendMessageToAllClients(tokens[1], O_GAME_PLAYER_STATE + " " + tokens[1] + " ON", true);

                                LOG_INFO("client '" + client->getNick() + "' rejected a game request sent from client '" + sender + "'");
                            }
                            break;
                        case Client::GAME:
                            if (msg == I_GAME_PLAY) {
                                xPosition = stoi(tokens[1]);
                                gameRoomsMtx.lock();
                                Connect4 *game = gameRooms[client->getNick()]->game;
                                Connect4::GameState gameState = game->play(client->getNick(), xPosition);
                                gameRoomsMtx.unlock();
                                if (gameState != Connect4::CONTINUE) {
                                    deleteGameRoom(client->getNick(), "the game is over", true);
                                    client->sendMessage(O_GAME_CANCELED + " the game is over");
                                }
                            }
                            else if (msg == I_GAME_CANCELED) {
                                LOG_GAME("client '" + client->getNick() + "' canceled the game");
                                deleteGameRoom(client->getNick(), "your opponent canceled the game", true);
                                client->sendMessage(O_GAME_CANCELED + " you just canceled the game");
                            }
                            else {
                                LOG_ERR("client '" + client->getNick() + "' is playing a game and not following the protocol");
                                deleteGameRoom(client->getNick(), "your opponent was not following the protocol and was kicked out of the server", true);
                                client->sendMessage(O_INVALID_PROTOCOL + " when you're playing a game, you're supposed to either play or cancel it");
                                removeClient(client);
                                return;
                            }
                            break;
                        case Client::KILL_THREAD:
                            break;
                    }
                }
        }
    }
    if (msgLen == -1) {
        client->sendMessage(O_INVALID_PROTOCOL + " unknown message");
        LOG_ERR("client " + client->toStr() + " sent an unknown message: '" + receivedMsg + "'");

        if (client->getState() == Client::GAME)
            deleteGameRoom(client->getNick(), "your opponent was not following the protocol and was kicked out of the server", true);
        if (client->getState() == Client::SENT_RQ)
            sendMessageToAllClients(receiver, O_GAME_PLAYER_STATE + " " + receiver + " ON", true);
        if (client->getState() == Client::RECV_RQ) {
            sender = getGameRequestSender(client->getNick());
            sendMessageToAllClients(sender, O_GAME_PLAYER_STATE + " " + sender + " ON", true);
        }
        Client::State state = client->getState();
        client->setState(Client::KILL_THREAD);
        sleep(2);
        client->setState(state);

        client->setHandlingThreadRunning(false);

        removeClient(client);
        return;
    }
    if (msgLen == 1) {
        if (client->getState() == Client::GAME)
            deleteGameRoom(client->getNick(), "your opponent has suddenly left the server (on purpose)", true);
        if (client->getState() == Client::SENT_RQ)
            sendMessageToAllClients(receiver, O_GAME_PLAYER_STATE + " " + receiver + " ON", true);
        if (client->getState() == Client::RECV_RQ) {
            sender = getGameRequestSender(client->getNick());
            sendMessageToAllClients(sender, O_GAME_PLAYER_STATE + " " + sender + " ON", true);
        }
        Client::State state = client->getState();
        client->setState(Client::KILL_THREAD);
        sleep(2);
        client->setState(state);
        removeClient(client);
        return;
    }

    LOG_WARNING("lost connection with the client " + client->toStr());
    if (client->getState() == Client::SENT_RQ)
        sendMessageToAllClients(receiver, O_GAME_PLAYER_STATE + " " + receiver + " ON", true);
    if (client->getState() == Client::RECV_RQ) {
        sender = getGameRequestSender(client->getNick());
        sendMessageToAllClients(sender, O_GAME_PLAYER_STATE + " " + sender + " ON", true);
    }
    if (client->getState() == Client::GAME) {
        bool stillHasOpponent = playerStillHasOpponentInGame(client->getNick());
        std::string opponent = getPlayersOpponent(client->getNick(), true);
        removePlayerFromGameRoom(client->getNick());

        if (stillHasOpponent) {
            LOG_GAME("client '" + client->getNick() + "' lost their connection. Waiting for them " + std::to_string(SECONDS_WAITING_FOR_DISCONNECTED_PLAYER) + "s");
            addPlayerToReconnectingList(client->getNick(), opponent);
            clientReconnectingHandler = std::thread(&Server::waitingForPlayerToConnectBackHandler, this, client->getNick(), opponent);
            clientReconnectingHandler.detach();
        }
        else removeBothPlayersFromTheReconnectingList(client->getNick(), opponent);
    }
    Client::State state = client->getState();
    client->setState(Client::KILL_THREAD);
    sleep(2);
    client->setState(state);
    if (client->getState() == Client::SENT_RQ || client->getState() == Client::RECV_RQ)
        deleteGameRequest(client->getNick());
    removeClient(client);
}

void Server::sendOtherOnlineClientsToClient(Client *client) {
    clientMtx.lock();
    for (auto it : clients)
        if (it.first != client->getNick())
            client->sendMessage(O_ADD_CLIENT + " " + it.first);
    clientMtx.unlock();
}

void Server::sendBusyClientsToClient(Client *client) {
    clientMtx.lock();
    for (auto it : clients)
        if (it.first != client->getNick()) {
            Client::State state = it.second->getState();
            if (state == Client::GAME || state == Client::RECV_RQ || state == Client::SENT_RQ)
                client->sendMessage(O_GAME_PLAYER_STATE + " " + it.first + " OFF");
        }
    clientMtx.unlock();
}

void Server::deleteGameRequest(std::string client) {
    std::string sender = getGameRequestSender(client);
    setClientState(client, Client::LOBBY);
    setClientState(sender, Client::LOBBY);
    sendMessage(client, O_RQ_CANCELED + " " + sender);
    sendMessage(sender, O_RQ_CANCELED + " " + client);
}

void Server::waitingForPlayerToConnectBackHandler(std::string player, std::string opponent) {
    for (int i = 0; i < SECONDS_WAITING_FOR_DISCONNECTED_PLAYER; i++) {
        if (!isPlayerStillInGame(opponent) || !isPlayerOnReconnectingList(player)) {
            LOG_COUNTDOWN("waiting for client '" + player + "' to reconnect back to the server was interrupted");
            removeBothPlayersFromTheReconnectingList(player, opponent);
            return;
        }
        int remainingSeconds = SECONDS_WAITING_FOR_DISCONNECTED_PLAYER - i;
        LOG_COUNTDOWN("waiting for client '" + player + "' to reconnect back to the server (remaining second: " + std::to_string(remainingSeconds) + ")");
        sleep(1);
    }
    removePlayerFromReconnectingList(player, false);
}

bool Server::isPlayerOnReconnectingList(std::string player) {
    reconnectingClientsMtx.lock();
    bool isOnList = reconnectingClients.find(player) != reconnectingClients.end();
    reconnectingClientsMtx.unlock();
    return isOnList;
}

bool Server::isPlayerStillInGame(std::string player) {
    gameRoomsMtx.lock();
    bool stillExists = gameRooms.find(player) != gameRooms.end();
    gameRoomsMtx.unlock();
    return stillExists;
}

void Server::removeBothPlayersFromTheReconnectingList(std::string player1, std::string player2) {
    reconnectingClientsMtx.lock();
    if (reconnectingClients.find(player1) != reconnectingClients.end())
        reconnectingClients.erase(player1);
    if (reconnectingClients.find(player2) != reconnectingClients.end())
        reconnectingClients.erase(player2);
    reconnectingClientsMtx.unlock();
}

void Server::removePlayerFromReconnectingList(std::string player, bool successfullyConnected) {
    reconnectingClientsMtx.lock();
    std::string opponent = reconnectingClients[player];
    if (successfullyConnected) {
        addToGameRoom(player, opponent);
        sendMessageToAllClients(player, O_GAME_PLAYER_STATE + " " + player + " OFF", true);
        LOG_GAME("client '" + player + "' has been successfully added back to the game against client '" + opponent + "'");
    }
    else {
        sendMessage(opponent, O_GAME_CANCELED + " the other player has not been connected back to the server within " + std::to_string(SECONDS_WAITING_FOR_DISCONNECTED_PLAYER) + "s");
        deleteGameRoom(opponent, "", false);
        setClientState(opponent, Client::LOBBY);
        LOG_GAME("client '" + player + "' has NOT yet been connected back to the server - ending the game against client '" + opponent + "'");
    }
    reconnectingClients.erase(player);
    reconnectingClientsMtx.unlock();
}

void Server::addPlayerToReconnectingList(std::string player, std::string opponent) {
    reconnectingClientsMtx.lock();
    reconnectingClients[player] = opponent;
    reconnectingClientsMtx.unlock();
}

void Server::addToGameRoom(std::string player, std::string opponent) {
    gameRoomsMtx.lock();
    gameRooms[player] = gameRooms[opponent];
    setClientState(player, Client::GAME);
    sendMessage(player, O_START_GAME + " " + opponent);
    sendMessage(player, O_GAME_MESSAGE + " you've been successfully added back to the game against " + opponent);
    sendMessage(player, O_GAME_RECOVERY + " " + gameRooms[player]->game->getCurrentStateOfGameForRecovery());
    sendMessage(opponent, O_GAME_MESSAGE + " your opponent is back in the game");
    gameRooms[player]->game->setWatchingThreadOnHold(false);
    gameRoomsMtx.unlock();
}

void Server::addGameRoom(std::string player1, std::string player2) {
    gameRoomsMtx.lock();
    Connect4 *game = new Connect4(player1, player2, this);
    GameRoom_t *gameRoom = new GameRoom_t{player1, player2, game};

    gameRooms[player1] = gameRoom;
    gameRooms[player2] = gameRoom;
    gameRoomsMtx.unlock();
}

void Server::deleteGameRoom(std::string player, std::string msgToOtherPlayer, bool lockReconnectingClients) {
    gameRoomsMtx.lock();
    std::string opponent = getPlayersOpponent(player, false);
 
    if (gameRooms.find(opponent) != gameRooms.end()) {
        gameRooms.erase(opponent);
        setClientState(opponent, Client::LOBBY);
        sendMessage(opponent, O_GAME_CANCELED + " " + msgToOtherPlayer);
        sendMessageToAllClients(opponent, O_GAME_PLAYER_STATE + " " + opponent + " ON", true);
    }
    else {
        LOG_GAME("taking opponent '" + opponent + "' of player '" + player + "' off the list of clients waiting to reconnect");
        if (lockReconnectingClients)
            reconnectingClientsMtx.lock();
        reconnectingClients.erase(opponent);
        if (lockReconnectingClients)
            reconnectingClientsMtx.unlock();
    }
    if (gameRooms.find(player) == gameRooms.end()) {
        gameRoomsMtx.unlock();
        return;
    }
    delete gameRooms[player]->game;
    delete gameRooms[player];
    gameRooms.erase(player);
    setClientState(player, Client::LOBBY);
    sendMessageToAllClients(player, O_GAME_PLAYER_STATE + " " + player + " ON", true);

    gameRoomsMtx.unlock();
    LOG_GAME("the game between " + player + " and " + opponent + " is over");
}

void Server::removePlayerFromGameRoom(std::string player) {
    gameRoomsMtx.lock();
    std::string opponent = getPlayersOpponent(player, false);
    if (gameRooms.find(opponent) == gameRooms.end()) {
        LOG_GAME("the opponent of player '" + player + "' is not connected to the server either -> deleting the game");
        removeBothPlayersFromTheReconnectingList(player, opponent);
        delete gameRooms[player]->game;
        delete gameRooms[player];
    }
    else {
        sendMessage(opponent, O_GAME_MESSAGE + " other player lost their connection. Waiting for him " + std::to_string(SECONDS_WAITING_FOR_DISCONNECTED_PLAYER) + "s");
        gameRooms[opponent]->game->setWatchingThreadOnHold(true);
    }
    gameRooms.erase(player);
    gameRoomsMtx.unlock();
}

bool Server::playerStillHasOpponentInGame(std::string player) {
    gameRoomsMtx.lock();
    std::string opponent = getPlayersOpponent(player, false);
    bool stillHasOpponent = gameRooms.find(opponent) != gameRooms.end();
    gameRoomsMtx.unlock();
    return stillHasOpponent;
}

std::string Server::getPlayersOpponent(std::string player, bool lock) {
    if (lock)
        gameRoomsMtx.lock();
    if (gameRooms.find(player) == gameRooms.end()) {
        if (lock)
            gameRoomsMtx.unlock();
        return Client::UNDEFINED_NICK;
    }
    std::string opponent = Client::UNDEFINED_NICK;
    if (gameRooms[player]->player1 == player)
        opponent = gameRooms[player]->player2;
    else opponent = gameRooms[player]->player1;
    if (lock)
        gameRoomsMtx.unlock();
    return opponent;
}

void Server::addGameRequest(std::string sender, std::string receiver) {
    gameRequestsMtx.lock();
    gameRequests[receiver] = sender;
    gameRequestsMtx.unlock();
}

std::string Server::getGameRequestSender(std::string receiver) {
    std::string sender = Client::UNDEFINED_NICK;
    gameRequestsMtx.lock();
    auto it = gameRequests.find(receiver);
    if (it != gameRequests.end())
        sender = it->second;
    gameRequestsMtx.unlock();
    return sender;
}

void Server::waitingForReplyToGameRQHandler(std::string sender, std::string receiver) {
    bool clientLostConnection = false;
    for (int i = 0; i < SECONDS_WAITING_FOR_REPLY_TO_GAME_RQ; i++) {
        if (!existsClient(sender) || !existsClient(receiver)) {
            LOG_COUNTDOWN("waiting of client '" + sender + "' for client '" + receiver + "' was interrupted. One of the clients is no longer connected to the server");
            clientLostConnection = true;
            break;
        }
        if (getStateOfClient(sender) != Client::SENT_RQ || getStateOfClient(receiver) != Client::RECV_RQ) {
            LOG_COUNTDOWN("waiting (countdown) of client '" + sender + "' for client ' to reply to the game request" + receiver + "' was interrupted");
            return;
        }
        int remainingSeconds = SECONDS_WAITING_FOR_REPLY_TO_GAME_RQ - i;
        LOG_COUNTDOWN("client '" + sender + "' is waiting for client '" + receiver + "' to reply to their game request (remaining second: " + std::to_string(remainingSeconds) + ")");
        sleep(1);
    }
    if (!clientLostConnection) {
        LOG_COUNTDOWN("countdown of client '" + sender + "' is waiting for client '" + receiver + "' is over ");
    }
    setClientState(sender, Client::LOBBY);
    setClientState(receiver, Client::LOBBY);
    sendMessage(sender, O_RQ_CANCELED + " " + receiver);
    sendMessage(receiver, O_RQ_CANCELED + " " + sender);
    sendMessageToAllClients(sender, O_GAME_PLAYER_STATE + " " + sender + " ON", true);
    sendMessageToAllClients(receiver, O_GAME_PLAYER_STATE + " " + receiver + " ON", true);
}

void Server::sendMessage(std::string nick, std::string msg) {
    clientMtx.lock();
    auto it = clients.find(nick);
    if (it != clients.end())
        it->second->sendMessage(msg);
    else {
        LOG_WARNING("trying to send a message to client '" + nick + "' that no longer exists");
    }
    clientMtx.unlock();
}

void Server::setClientState(std::string nick, Client::State state) {
    clientMtx.lock();
    auto it = clients.find(nick);
    if (it != clients.end())
        it->second->setState(state);
    else {
        LOG_WARNING("trying to change the sate of client '" + nick + "' that no longer exists");
    }
    clientMtx.unlock();
}

Client::State Server::getStateOfClient(std::string nick) {
    clientMtx.lock();
    Client::State state = clients[nick]->getState();
    clientMtx.unlock();
    return state;
}

bool Server::existsClient(std::string nick) {
    clientMtx.lock();
    bool exists = clients.find(nick) != clients.end();
    clientMtx.unlock();
    return exists;
}

void Server::addNewClient(Client *client) {
    clientMtx.lock();
    clients[client->getNick()] = client;
    sendMessageToAllClients(client->getNick(), O_ADD_CLIENT + " " + client->getNick(), false);
    clientMtx.unlock();
}

std::string Server::getNicksAllClients() {
    clientMtx.lock();
    std::stringstream ss;
    ss << "[";
    for (auto it : clients)
        ss << it.first << MSG_SEPARATOR;
    clientMtx.unlock();
    std::string allClients = ss.str();
    allClients.pop_back();
    allClients += "]";
    return allClients;
}

void Server::removeClient(Client *client) {
    numberOfClients--;
    if (client->getState() == Client::NICK)
        removeClientByReference(client);
    else removeClientByNick(client->getNick());
}

void Server::removeClientByReference(Client *client) {
    LOG_INFO("closing the connection for the client " + client->toStr());
    delete client;
}

void Server::removeClientByNick(std::string nick) {
    clientMtx.lock();
    sendMessageToAllClients(nick, O_REMOVE_CLIENT + " " + nick, false);
    removeClientByReference(clients[nick]);
    clients.erase(nick);
    clientMtx.unlock();
}

void Server::sendMessageToAllClients(std::string sender, std::string message, bool lock) {
    if (lock)
        clientMtx.lock();
    for (auto it : clients)
        if (it.first != sender)
            it.second->sendMessage(message);
    if (lock)
        clientMtx.unlock();
}

std::string Server::getHelp() const {
    std::stringstream ss;
    ss << "\r\n";
    for (auto it : msgValidation)
        ss << it.first << " " << it.second.description << "\r\n";
    return ss.str();
}

Server::IncomingMsg Server::getTypeOfMessage(const std::vector<std::string>& tokens) const {
    if (tokens.empty())
        return UNKNOWN;
    auto msg = msgValidation.find(tokens[0]);
    if (msg == msgValidation.end())
        return UNKNOWN;
    if (msg->second.validation == NULL)
        return msg->second.msg;
    if (msg->second.validation(tokens) == false)
        return UNKNOWN;
    return msg->second.msg;
}

bool validNick(const std::vector<std::string>& tokens) {
    return tokens.size() == 2;
}

bool validGameRq(const std::vector<std::string>& tokens) {
    return tokens.size() == 2;
}

bool validExit(const std::vector<std::string>& tokens) {
    return tokens.size() == 1;
}

bool validPing(const std::vector<std::string>& tokens) {
    return tokens.size() == 1;
}

bool validGetNick(const std::vector<std::string>& tokens) {
    return tokens.size() == 1;
}

bool validGetState(const std::vector<std::string>& tokens) {
    return tokens.size() == 1;
}

bool validGetAllClients(const std::vector<std::string>& tokens) {
    return tokens.size() == 1;
}

bool validRqCanceled(const std::vector<std::string>& tokens) {
    return tokens.size() == 2;
}

bool validHelp(const std::vector<std::string>& tokens) {
    return tokens.size() == 1;
}

bool validReply(const std::vector<std::string>& tokens) {
    if (tokens.size() != 3)
        return false;
    return tokens[2] == "YES" || tokens[2] == "NO";
}

bool validGameCanceled(const std::vector<std::string>& tokens) {
    return tokens.size() == 1;
}

bool validGamePlay(const std::vector<std::string>& tokens) {
    if (tokens.size() != 2)
        return false;
    try {
        for (char c : tokens[1])
            if (c < '0' || c > '9')
                return false;
        int x = stoi(tokens[1]);
        if (x < 0 || x >= Connect4::COLUMNS)
            return false;
    }
    catch (std::invalid_argument& e){
        return false;
    }
    catch (std::out_of_range& e) {
        return false;
    }
    catch (...) {
        return false;
    }
    return true;
}