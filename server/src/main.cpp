#include "Server.h"
#include "InputShell.h"

/// The entry point of the application
///
/// \param argc number of arguments the user enters in the terminal
/// \param argv argument values
/// \return 0 on successful execution. EXIT_FAILURE,
/// if the user enters invalid parameters when starting the application
int main(int argc, char *argv[]) {
    // process the parameters entered by the user
    InputShell inputShell(argc, argv);
    if (inputShell.isValid() == false) {
        inputShell.printHelp();
        exit(EXIT_FAILURE);
    }
    // run the server
    Server server(inputShell.getPort(), inputShell.getMaxNumberOfClients());
    server.startServer();
    return 0;
}