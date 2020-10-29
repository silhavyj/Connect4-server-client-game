#include "Connect4.h"

Connect4::Connect4(std::string player1, std::string player2, Server *server) {
    this->player1 = player1;
    this->player2 = player2;
    this->server = server;

    // initialize variables for
    // checking if the player who is up
    // played or not
    justPlayedMtx.lock();
    justPlayed = false;
    justPlayedMtx.unlock();

    runThreadMtx.lock();
    runThread = true;
    runThreadMtx.unlock();
    setWatchingThreadOnHold(false);

    // run the thread waiting for the client
    // that is up to play
    std::thread clientHandler(&Connect4::waitingPlayerToPlayHandler, this);
    clientHandler.detach();

    player1IsUp = true;
    memset(board, FREE, sizeof(board));

    // store all rows, columns, diagonals
    // into vectors
    storeAllRows();
    storeAllColumns();
    storeDiagonal1();
    storeDiagonal2();
}

Connect4::~Connect4() {
    stopWaitingPlayerToPlayThread();
    sleep(2); // waits for the thread to notice
}

void Connect4::storeAllRows() {
    for (int i = 0; i < ROWS; i++) {
        std::vector<std::pair<int,int>> row;
        for (int j = 0; j < COLUMNS; j++)
            row.push_back({i,j});
        rows.push_back(row);
    }
}

void Connect4::storeAllColumns() {
    for (int i = 0; i < COLUMNS; i++) {
        std::vector<std::pair<int,int>> column;
        for (int j = 0; j < ROWS; j++)
            column.push_back({j,i});
        columns.push_back(column);
    }
}

void Connect4::storeDiagonal1() {
    int y, x;
    for (int i = 0; i < ROWS + COLUMNS; i++) {
        if (i < ROWS) {
            y = i;
            x = 0;
        }
        else if (i == ROWS)
            continue;
        else {
            y = ROWS - 1;
            x = i - ROWS;
        }
        std::vector<std::pair<int,int>> diagonal;
        while (y != -1 && x != COLUMNS) {
            diagonal.push_back({y,x});
            y--;
            x++;
        }
        diagonal1.push_back(diagonal);
    }
}

void Connect4::storeDiagonal2() {
    int y, x;
    for (int i = 0; i < ROWS + COLUMNS; i++) {
        if (i < ROWS) {
            y = ROWS - i - 1;
            x = 0;
        }
        else if (i == ROWS)
            continue;
        else {
            y = 0;
            x = i - ROWS;
        }
        std::vector<std::pair<int,int>> diagonal;
        while (y != ROWS && x != COLUMNS) {
            diagonal.push_back({y,x});
            y++;
            x++;
        }
        diagonal2.push_back(diagonal);
    }
}

std::vector<std::pair<int,int>> Connect4::getWinningTiles(std::vector<std::vector<std::pair<int,int>>> &sequence) {
    std::vector<std::pair<int,int>> winningTiles;
    for (auto s : sequence) {
        if ((int)s.size() < NUMBER_OF_WINNING_TILES)
            continue;
        int counter = 0;
        for (int i = 0; i < (int)s.size(); i++) {
            if (i == 0) {
                if (board[s[i].first][s[i].second] != FREE)
                    counter = 1;
                else counter = 0;
            }
            else if (board[s[i].first][s[i].second] == FREE)
                counter = 0;
            else if (board[s[i].first][s[i].second] != board[s[i-1].first][s[i-1].second])
                counter = 1;
            else counter++;

            if (counter == NUMBER_OF_WINNING_TILES) {
                for (int k = i - (NUMBER_OF_WINNING_TILES - 1); k <= i; k++)
                    winningTiles.push_back(s[k]);
                return winningTiles;
            }
        }
    }
    return winningTiles;
}

void Connect4::setWatchingThreadOnHold(bool value) {
    LOG_GAME("thread checking the game between '" + player1 + "' and '" + player2 + "' was " + (value ? "resumed" : "paused"));
    waitingForOtherClientToConnectBackMtx.lock();
    waitingForOtherClientToConnectBack = value;
    waitingForOtherClientToConnectBackMtx.unlock();
}

bool Connect4::isDraw() {
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLUMNS; j++)
            if (board[i][j] == FREE)
                return false;
    return true;
}

Connect4::GameState Connect4::announceWinner(std::vector<std::pair<int, int> > &winningTiles, std::string player) {
    GameState gameState = board[winningTiles[0].first][winningTiles[0].second] == PLAYER_1 ? PLAYER_1_WINS : PLAYER_2_WINS;
    server->sendMessage(player1, server->O_GAME_GAME_RESULT + " You " + (player == player1 ? "won" : "lost"));
    server->sendMessage(player2, server->O_GAME_GAME_RESULT + " You " + (player == player2 ? "won" : "lost"));

    std::stringstream ss;
    for (auto it : winningTiles)
        ss << it.first << " " << it.second << " ";
    std::string winningSpots = ss.str();
    winningSpots.pop_back();
    server->sendMessage(player1, server->O_GAME_WINNING_TILES + " " + winningSpots);
    server->sendMessage(player2, server->O_GAME_WINNING_TILES + " " + winningSpots);
    return gameState;
}

void Connect4::sendMsgMoveToPlayers(int y, int x, std::string player) {
    std::string msgToPlayers = server->O_GAME_PLAY + " " + player + " " + std::to_string(y) + " " + std::to_string(x);
    server->sendMessage(player1, msgToPlayers);
    server->sendMessage(player2, msgToPlayers);
}

void Connect4::announceDraw() {
    stopWaitingPlayerToPlayThread();
    server->sendMessage(player1, server->O_GAME_GAME_RESULT + " draw");
    server->sendMessage(player2, server->O_GAME_GAME_RESULT + " draw");
}

Connect4::GameState Connect4::play(std::string player, int x) {
    if ((player == player1 && player1IsUp == false) ||
        (player == player2 && player1IsUp == true)) {
        server->sendMessage(player, server->O_GAME_MESSAGE + " it is not your turn");
        return CONTINUE;
    }
    if (board[0][x] != FREE) {
        server->sendMessage(player, server->O_GAME_MESSAGE + " this column is full. Choose another one");
        return CONTINUE;
    }
    justPlayedMtx.lock();
    justPlayed = true;
    justPlayedMtx.unlock();

    int y = 0;
    while (y+1 < ROWS && board[y+1][x] == FREE)
        y++;

    board[y][x] = player1IsUp ? PLAYER_1 : PLAYER_2;
    sendMsgMoveToPlayers(y, x, player);
    printBoard();
    std::vector<std::pair<int,int>> winningTiles;

    // rows
    winningTiles = getWinningTiles(rows);
    if (winningTiles.empty() == false)
        return announceWinner(winningTiles, player);

    // columns
    winningTiles = getWinningTiles(columns);
    if (winningTiles.empty() == false)
        return announceWinner(winningTiles, player);

    // diagonal1
    winningTiles = getWinningTiles(diagonal1);
    if (winningTiles.empty() == false)
        return announceWinner(winningTiles, player);

    // diagonal2
    winningTiles = getWinningTiles(diagonal2);
    if (winningTiles.empty() == false)
        return announceWinner(winningTiles, player);

    // is draw
    if (isDraw()) {
        stopWaitingPlayerToPlayThread();
        announceDraw();
        return DRAW;
    }

    player1IsUp = !player1IsUp;
    return CONTINUE;
}

void Connect4::stopWaitingPlayerToPlayThread() {
    runThreadMtx.lock();
    runThread = false;
    runThreadMtx.unlock();
}

void Connect4::printBoard() {
    std::cout << "[GAME BETWEEN '" + player1 + "' and '" + player2 + "']\n";
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLUMNS; j++)
            std::cout << board[i][j] << " ";
        std::cout << "\n";
    }
}

std::string Connect4::getCurrentStateOfGameForRecovery() {
    std::stringstream ss;
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLUMNS; j++)
            ss << board[i][j] << " ";
    std::string currentState = ss.str();
    currentState.pop_back();
    return currentState;
}

void Connect4::waitingPlayerToPlayHandler() {
    int timeCounter = 0;
    while (1) {
        int remainingSeconds = SECONDS_WAITING_FOR_CLIENT_TO_PLAY - timeCounter;

        waitingForOtherClientToConnectBackMtx.lock();
        if (waitingForOtherClientToConnectBack == true)
            timeCounter = 0;
        else LOG_COUNTDOWN("counter of the game between '" + player1 + "' and '" + player2 + "': " + std::to_string(remainingSeconds) + "s");
        waitingForOtherClientToConnectBackMtx.unlock();

        runThreadMtx.lock();
        if (runThread == false) {
            LOG_COUNTDOWN("counter of the game between '" + player1 + "' and '" + player2 + "' was interrupted (end of the game)");
            return;
        }
        runThreadMtx.unlock();

        timeCounter++;
        justPlayedMtx.lock();
        if (justPlayed == true) {
            justPlayed = false;
            timeCounter = 0;
        }
        justPlayedMtx.unlock();

        if (timeCounter == SECONDS_WAITING_FOR_CLIENT_TO_PLAY) {
            server->deleteGameRoom(player1IsUp ? player1 : player2, "your opponent hasn't played for " + std::to_string(SECONDS_WAITING_FOR_CLIENT_TO_PLAY) + "s", true);
            server->sendMessage(player1IsUp ? player1 : player2, server->O_GAME_CANCELED + " the game has been terminated due to you not playing");
            LOG_WARNING("counter of the game between '" + player1 + "' and '" + player2 + "' was interrupted (nobody's played in " + std::to_string(SECONDS_WAITING_FOR_CLIENT_TO_PLAY) + "s)");
            return;
        }
        sleep(1);
    }
}