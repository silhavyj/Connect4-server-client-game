#include "Logger.h"

Logger *Logger::instance = NULL;

Logger *Logger::getInstance() {
    if (instance == NULL)
        instance = new Logger;
    return instance;
}

Logger::Logger() {
    logFileName = getCurrentDateTime();
    // creates a folder 'log' where
    // all the log files will be stored
    mkdir(logDirectory.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
}

void Logger::log(int lineNumber, Type type, std::string msg) {
    // attaches the appropriate color to
    // the message given as a parameter by its type
    switch (type) {
        case ERROR:     log(lineNumber, "ERROR_LOG", RED, msg);        break;
        case INFO:      log(lineNumber, "INFO_LOG", GREEN, msg);       break;
        case COUNTDOWN: log(lineNumber, "COUNTDOWN_LOG", CYAN, msg);   break;
        case BOOTING:   log(lineNumber, "BOOTING_LOG", MAGENTA, msg);  break;
        case WARNING:   log(lineNumber, "WARNING_LOG", YELLOW, msg);   break;
        case GAME:      log(lineNumber, "GAME_LOG", WHITE, msg);       break;
        case MSG:       log(lineNumber, "MSG_LOG", BLUE, msg);         break;
    }
}

void Logger::log(int lineNumber, std::string type, std::string color, std::string msg) {
    // prints out the message
    std::stringstream  ss;
    ss << "[#" << lineNumber << "]";
    ss << "[" << getCurrentDateTime() << "]";
    ss << "[" << type << "]";
    ss << " " << msg;
    std::cout << color << ss.str() << RESET << std::endl;

    // appends it to the end of the file
    addToLogFile(ss.str());
}

std::string Logger::getCurrentDateTime() const {
    time_t current_time;
    struct tm *time_info;
    char buffer[80];

    time(&current_time);
    time_info = localtime(&current_time);

    strftime(buffer, sizeof(buffer),"%d-%m-%Y_%H-%M-%S", time_info);
    return std::string(buffer);
}

void Logger::addToLogFile(std::string log) const {
    std::ofstream logFile;
    std::string file = logDirectory + "/" + logFileName + logFileType;
    logFile.open(file, std::ios::out | std::ios::app);
    if (logFile.fail())
        throw std::ios_base::failure(std::strerror(errno));
    logFile.exceptions(logFile.exceptions() | std::ios::failbit | std::ifstream::badbit);
    logFile << log << std::endl;
    logFile.close();
}

void Logger::testAllTypes() {
    LOG_ERR("This is an error message");
    LOG_INFO("This is an info message");
    LOG_COUNTDOWN("This is a countdown message");
    LOG_BOOTING("This is a booting message");
    LOG_WARNING("This is a warning message");
    LOG_GAME("This is a game message");
    LOG_MSG("This is a received message");
}