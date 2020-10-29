#include "InputShell.h"

InputShell::InputShell(int argc, char *argv[]) {
    // set all variables to their default values
    port = Server::PORT_DEFAULT;
    maxNumberOfClients = Server::MAX_CLIENTS_DEFAULT;

    // check the number of arguments
    // the user entered
    if (argc <= 5 && argc & 1) {
        int i = 1;

        while (i < argc) {
            std::string token(argv[i++]);
            if (!(i & 1)) {
                // -p 53333
                if (token == PORT_ARG) {
                    int val = getNum(argv[i]);
                    if (val == INVALID_NUM_ARG || val < 0 || val > 65535) {
                        valid = false;
                        return;
                    }
                    port = val;
                    i++;
                }
                // -c 120
                else if (token == MAX_NUMBER_OF_CLIENTS_ARG) {
                    int val = getNum(argv[i]);
                    if (val == INVALID_NUM_ARG) {
                        valid = false;
                        return;
                    }
                    maxNumberOfClients = val;
                    i++;
                }
                else {
                    valid = false;
                    return;
                }
            }
        }
        valid = true;
    }
    else valid = false;
}

int InputShell::getNum(std::string str) const {
    try {
        // check if the string contains
        // digits only
        for (char c : str)
            if (c < '0' || c > '9')
                return INVALID_NUM_ARG;

        int x = stoi(str);
        if (x < 0)
            return INVALID_NUM_ARG;
        return x;
    }
    catch(std::invalid_argument& e){
        return INVALID_NUM_ARG;
    }
    catch(std::out_of_range& e) {
        return INVALID_NUM_ARG;
    }
    catch(...) {
        return INVALID_NUM_ARG;
    }
}

int InputShell::getPort() const {
    return port;
}

int InputShell::getMaxNumberOfClients() {
    return maxNumberOfClients;
}

void InputShell::printHelp() const {
    std::cout << PORT_ARG << " Port on which the server will be running.\n";
    std::cout << "   Default value is " + std::to_string(Server::PORT_DEFAULT) << ".\n";
    std::cout << MAX_NUMBER_OF_CLIENTS_ARG << " Maximum number of clients that can be\n";
    std::cout << "   connected to the server at a time.\n";
    std::cout << "   Default value is " + std::to_string(Server::MAX_CLIENTS_DEFAULT) << ".\n";
}

bool InputShell::isValid() const {
    return valid;
}
