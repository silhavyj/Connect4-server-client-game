#ifndef INPUT_SHELL_H
#define INPUT_SHELL_H

#include <iostream>
#include "Server.h"

/// \author silhavyj A17B0362P
///
/// This class deals with the parameters the users
/// enters when running the program. These parameters
/// allow him to change the port the server runs on
/// as well as the maximum number of clients that
/// can be connected to the server at a time.
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/// ./server -p 53333 -c 20
/// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class InputShell {
public:
    /// parameter p that allows the user to set a port
    /// on which they want the server to run
    /// ### Example
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// ./server -p 53333
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    const std::string PORT_ARG = "-p";

    /// parameter c that allows the user to set the
    /// maximum number of clients that can be connected
    /// to the server at a time
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /// ./server -c 6
    /// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    const std::string MAX_NUMBER_OF_CLIENTS_ARG = "-c";

    /// indication of an invalid number used
    /// when parsing the number of maximum clients or
    /// the port number
    const int INVALID_NUM_ARG = -1;

private:
    /// Flag saying if the the input the user
    /// entered is valid or not
    bool valid;

    /// number of port the server runs on
    int port;

    /// maximum number of clients that can be
    /// connected to the server at a time
    int maxNumberOfClients;

private:
    /// Returns a number (an integer) from the string given as a parameter
    ///
    /// If the string parameter is not a number.
    /// The method will return #INVALID_NUM_ARG.
    ///
    /// \param str string we want to cast into an integer
    /// \return an integer value of the string (if everything goes well)
    /// #INVALID_NUM_ARG otherwise
    int getNum(std::string str) const;

public:
    /// Constructor of the class - creates an instance of it
    /// \param argc number of arguments the user put into
    /// the terminal when running the program
    /// \param argv argument values
    InputShell(int argc, char *argv[]);

    /// Returns whether or not the parameters that the user
    /// entered when running the program are all valid.
    /// \return true - if everything is okay. False, otherwise.
    bool isValid() const;

    /// Returns the number of the port the server runs on.
    ///
    /// This may be either the number the user put into the
    /// terminal or the default port defined in class #Server.
    ///
    /// \return the number of the port
    int getPort() const;

    /// Returns the maximum number of clients that can be connected to the server at a time
    ///
    /// This may be either the number the user put into the
    /// terminal or the default number defined in class #Server.
    ///
    /// \return the maximum number of clients
    int getMaxNumberOfClients();

    /// Prints out the help fro the user if they
    /// enter invalid parameters when running the program.
    void printHelp() const;
};

#endif
