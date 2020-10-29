#ifndef CONNECT4_H
#define CONNECT4_H

#include <iostream>
#include <vector>
#include <utility>
#include <cstring>
#include <thread>
#include <mutex>

#include "Server.h"

// forward declaration
class Server;

/// \author silhavyj A17B0362P
///
/// The purpose of this class is to handle
/// the logic of the game Connect4 - https://en.wikipedia.org/wiki/Connect_Four.
/// This class is directly used by class #Server when two
/// players decide to play a game.
class Connect4 {
public:
    /// number of rows on the grid
    static const int ROWS = 6;
    /// number of columns on the grid
    static const int COLUMNS = 7;
    /// number of winning tiles (disks)
    static const int NUMBER_OF_WINNING_TILES = 4;
    /// If the player who's up is not playing within 30s
    /// the game will be automatically terminated
    static const int SECONDS_WAITING_FOR_CLIENT_TO_PLAY = 30;

    /// State of the game
    enum GameState {
        PLAYER_1_WINS, ///< the client1 (player1) wins the game
        PLAYER_2_WINS, ///< the client2 (player2) wins the game
        CONTINUE,      ///< neither of the players has won the game yet
        DRAW           ///< draw - there is no room left on the grid
    };

private:
    /// State of each tail on the grid
    enum State {
        FREE,     ///< neither of the players occupies this position
        PLAYER_1, ///< player1 occupies this position
        PLAYER_2  ///< player2 occupies this position
    };

    /// grid of the game (board)
    State board[ROWS][COLUMNS];
    /// indication of who's turn it is
    bool player1IsUp;
    /// nick of player1 (client1)
    std::string player1;
    /// nick of player2 (client2)
    std::string player2;
    /// Reference to the server that
    /// is used for sending messages to both clients
    /// (who's up, if the other client lost their connection, etc.)
    Server *server;

    /// lock used when accessing variable #justPlayed
    std::mutex justPlayedMtx;
    /// indication that either of the clients
    /// just played - the counter of 30s is set back down to 0
    bool justPlayed;

    /// lock used when accessing variable #runThread
    std::mutex runThreadMtx;
    /// indication of whether or not the thread
    /// checking if the player who is up played within
    /// 30s should be terminated
    bool runThread;

    /// lock used when accessing variable #waitingForOtherClientToConnectBack
    std::mutex waitingForOtherClientToConnectBackMtx;
    /// When this variable is true, the counter checking if the player
    /// whose turn it is played within 30s will be set back down to 0.
    /// This indicates that one of the players has lost their connection.
    bool waitingForOtherClientToConnectBack;

    /// a vector of all the rows of the grid
    std::vector<std::vector<std::pair<int,int>>> rows;
    /// a vector of all the columns of the grid
    std::vector<std::vector<std::pair<int,int>>> columns;
    /// a vector of all the diagonals of the grid (first direction)
    std::vector<std::vector<std::pair<int,int>>> diagonal1;
    /// a vector of all the diagonals of the grid (second direction)
    std::vector<std::vector<std::pair<int,int>>> diagonal2;

private:
    /// Stores positions (y,x) of all the rows into #rows
    void storeAllRows();

    /// Stores positions (y,x) of all the columns into #columns
    void storeAllColumns();

    /// Stores positions (y,x) of all the diagonals
    /// running in the first direction into #diagonal1
    void storeDiagonal1();

    /// Stores positions (y,x) of all the diagonals
    /// running in the second direction into #diagonal2
    void storeDiagonal2();

    /// Check is there is a winning sequence of four tiles in a row
    ///
    /// If there is a winning sequence, it will return the positions
    /// of the winning tiles. For example, [5,0];[4,0];[3,0];[2,0].
    /// Otherwise, an empty vector will be returned.
    ///
    /// \param sequence in which we are looking for winning tiles (four in a row)
    /// \return either an empty vector or a vector containing the positions of winning tiles
    std::vector<std::pair<int,int>> getWinningTiles(std::vector<std::vector<std::pair<int,int>>>& sequence);

    /// Announces the winner of the game
    ///
    /// It sends appropriate messages to both clients announcing the
    /// winner of the game as well as the positions of the winning
    /// tales so the clients can see the winning sequence of tiles.
    ///
    /// \param winningTiles the sequence of winning tiles
    /// \param player who put the last tile (disk) on the grid
    /// \return #GameState indicating the game is over (one of the players won)
    GameState announceWinner(std::vector<std::pair<int,int>> &winningTiles, std::string player);

    /// Checks if it is draw (the either board is full of disks and no one won)
    /// \return true if it is draw. Otherwise, false.
    bool isDraw();

    /// Prints out the current state (board) of the game
    ///
    /// This method is used for debugging purposes so we can
    /// see the state of every game on the server side as well.
    void printBoard();

    /// Send a massage to both players when the player who is up just played
    ///
    /// It sends the position of the disk along with the name of
    /// the player who played it, so the client can visualize it.
    ///
    /// \param y y position of the disk (tile)
    /// \param x x position of the disk (tile)
    /// \param player nick of the player who just played
    void sendMsgMoveToPlayers(int y, int x, std::string player);

    /// Announced draw of the game
    ///
    /// This method sends a message to both clients
    /// announcing the end of the game (draw).
    void announceDraw();

    /// "Thread" waiting for the player who is up to play their turn.
    ///
    /// They have 30s to play. Otherwise, the game will be
    /// automatically terminated due to them not playing.
    void waitingPlayerToPlayHandler();

    /// Sets a flag to stop the thread waiting for the
    /// client who is up to play their turn.
    ///
    /// This method is called when the game is over
    /// and the instance needs to be deleted from the memory.
    void stopWaitingPlayerToPlayThread();

public:
    /// Constructor of the class - creates an instance of it
    /// \param player1 nick of the player1 (client1)
    /// \param player2 nick of the player2 (client2)
    /// \param server a reference to the server used for sending messages to he clients
    Connect4(std::string player1, std::string player2, Server *server);

    /// Destructor of the class
    ~Connect4();

    /// Plays one turn.
    ///
    /// If the player who is trying to play is not supposed to play yet because
    /// it is not their turn, the game will send off a message
    /// saying it is not their turn yet. Otherwise, one turn of the
    /// game will be played.
    ///
    /// \param player who is trying to play
    /// \param x x position of on the grid where they want to put their disk
    /// \return state of the game #GameState
    GameState play(std::string player, int x);

    /// Returns the current state of the game formatted
    /// so it could be send off to the client who just got reconnected
    /// back to the server (was in the game before and lost their connection)
    /// \return current state of the game appropriately formatted
    std::string getCurrentStateOfGameForRecovery();

    /// Pauses/runs the thread waiting for the player who is up
    /// to play because either of the clients just lost their
    /// connection - waiting for them to reconnect back to the server
    /// \param value true/false whether the thread should be paused
    void setWatchingThreadOnHold(bool value);
};

#endif