#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <sstream>
#include <fstream>
#include <cstring>
#include <ctime>
#include <sys/stat.h>

/// logs error message
#define LOG_ERR(msg)       (Logger::getInstance()->log(__LINE__, Logger::ERROR,     (msg)))
/// logs info message
#define LOG_INFO(msg)      (Logger::getInstance()->log(__LINE__, Logger::INFO,      (msg)))
/// logs countdown message
#define LOG_COUNTDOWN(msg) (Logger::getInstance()->log(__LINE__, Logger::COUNTDOWN, (msg)))
/// logs booting message
#define LOG_BOOTING(msg)   (Logger::getInstance()->log(__LINE__, Logger::BOOTING,   (msg)))
/// logs warning message
#define LOG_WARNING(msg)   (Logger::getInstance()->log(__LINE__, Logger::WARNING,   (msg)))
/// logs game message
#define LOG_GAME(msg)      (Logger::getInstance()->log(__LINE__, Logger::GAME,      (msg)))
/// logs "msg from a client" message
#define LOG_MSG(msg)       (Logger::getInstance()->log(__LINE__, Logger::MSG,       (msg)))

/// \author silhavyj A17B0362P
///
/// The purpose of this class is to log every activity
/// happening on the server so it could be used for
/// analysis later on. Example of what is logged could
/// be: when a client gets connected/disconnected, when
/// they send a message, when they start playing game,
/// when they not following the protocol, and so on.
/// This class also stores everything the is printed out
/// into a file named with the datetime when the server
/// was run. This class is written as a singleton.
class Logger {
public:
    /// reset color used when printing out into the terminal
    const std::string RESET   = "\033[0m";
    /// black color used when printing out into the terminal
    const std::string BLACK   = "\033[30m";
    /// red color used when printing out into the terminal
    const std::string RED     = "\033[31m";
    /// green color used when printing out into the terminal
    const std::string GREEN   = "\033[32m";
    /// yellow color used when printing out into the terminal
    const std::string YELLOW  = "\033[33m";
    /// blue color used when printing out into the terminal
    const std::string BLUE    = "\033[34m";
    /// magenta color used when printing out into the terminal
    const std::string MAGENTA = "\033[35m";
    /// cyan color used when printing out into the terminal
    const std::string CYAN    = "\033[36m";
    /// white color used when printing out into the terminal
    const std::string WHITE   = "\033[37m";

    /// Type of a logged message
    enum Type {
        ERROR,     ///< error message (for example, when a client attempts to log with a nick that's already taken)
        INFO,      ///< info message (for example, when a client gets connected to the server)
        COUNTDOWN, ///< countdown message (for example, when waiting for a client to enter their nick)
        BOOTING,   ///< booting message used when the server boots up
        WARNING,   ///< warning message (for example, if neither of the players play within 30s or so)
        GAME,      ///< game message (when two clients play a game)
        MSG        ///< "msg" message (when a client sends a message to the server)
    };

private:
    /// directory where all the log files are stored
    const std::string logDirectory = "log";
    /// type of a log file
    const std::string logFileType  = ".txt";

    /// the instance of the class
    static Logger* instance;
    /// the name of the log file (current datetime)
    std::string logFileName;

public:
    /// Copy constructor of the class.
    /// It was deleted because there is no need to use it
    /// within this project.
    Logger(Logger &) = delete;

    /// Assignment operator of the the class.
    /// It was deleted because there is no need to use it
    /// within this project.
    void operator=(Logger const &) = delete;

    /// Returns the instance of the class
    /// \return instance of the class
    static Logger *getInstance();

    /// Logs a message (prints it out and stores it into the log file)
    ///
    /// This method also keeps track of the number of the line
    /// from which the method was called so it could be added
    /// to the log message - makes the debugging process easier.
    ///
    /// \param lineNumber number of the line from which this log method was called
    /// \param type of the log message (#Type)
    /// \param msg the log message itself
    void log(int lineNumber, Type type, std::string msg);

    /// Prints out all different types of log messages
    ///
    /// This method is not used within this project. Its purpose
    /// is to show the user what the message look like when they
    /// are printed out, so they could change the colors if they
    /// want to.
    void testAllTypes();

private:
    /// Constructor of the class - creates an instance of it
    Logger();

    /// Logs a message (prints it out in color and stores it into the log file)
    ///
    /// This method is directly called by method log in which the message gets
    /// the appropriate color depending on its type.
    ///
    /// \param lineNumber number of the line from which this log method was called
    /// \param type of the log message (#Type)
    /// \param color in which the message is going to be printed out
    /// \param msg the log message itself
    void log(int lineNumber, std::string type, std::string color, std::string msg);

    /// Returns current datetime.
    ///
    /// This method is used when creating the log file as well
    /// as when a log method is called so the current date
    /// could be added to the log message.
    ///
    /// \return current datetime
    std::string getCurrentDateTime() const;

    /// Appends the log message to the end of the log file
    /// \param log the log message
    void addToLogFile(std::string log) const;
};

#endif