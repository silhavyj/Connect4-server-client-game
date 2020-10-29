import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Date;

/**
 * @author silhavyj A147B0362P
 *
 * This class is used for logging the
 * communication between the client and the server,
 * so it could be analyzed later on if required.
 * This class is written as a singlton.
 * */
public class Logger {

    /** the instance of the class */
    private static Logger instance = null;
    /** name of the directory where all the log files are stored */
    private static final String logDirectory = "log";
    /** type of the log file*/
    private static final String logFileType = ".txt";
    /** name of the log file - current date-time */
    private static String fileName = null;

    /** Type of a message that can be logged */
    public enum Type {
        ERROR,   // error message (for example, when the server sends an invalid message)
        INFO,    // information message (for example, a message received from the server)
        WARNING, // warning message (for example, when the server goes offline)
        MSG      // "message" message (for example, when a message is sent to the client from the server)
    }

    /** Constructor of the class - creates an instance of it */
    private Logger() {
        // creates the log file name - current date-time
        fileName = logDirectory + "/" + getCurrentDateTime() + logFileType;
        // creates the log director if it does not exist yet
        new File(logDirectory).mkdir();
    }

    /**
     * Returns the current date-time in the format dd-MM-yyyy_HH-mm-ss.
     * The current date time is used for logging as well as for
     * the name of the log file (when the application started)
     * @return current date-time as a String
     * */
    private String getCurrentDateTime() {
        SimpleDateFormat formatter = new SimpleDateFormat("dd-MM-yyyy_HH-mm-ss");
        Date date = new Date();
        return formatter.format(date);
    }

    /**
     * Returns the instance of the class. If the instance
     * does not exist yet, it will be created first.
     * @return the instance of the class*/
    public static final Logger getInstance() {
        if (instance == null)
            instance = new Logger();
        return instance;
    }

    /** Logs a message into the log file.
     *  The message is going to be printed out into the terminal as well
     *  @param type type of the message that is going to be logged
     *  @param message message itself that is going to be logged
     *  */
    public void log(Type type, final String message) {
        BufferedWriter bufferedWriter = null;
        FileWriter fileWriter = null;

        try {
            fileWriter = new FileWriter(new File(fileName), true);
            bufferedWriter = new BufferedWriter(fileWriter);

            StringBuilder sb = new StringBuilder();
            sb.append("[").append(getCurrentDateTime()).append("]");
            sb.append("[").append(type).append("] ");
            sb.append(message).append("\n");

            bufferedWriter.write(sb.toString());
            bufferedWriter.flush();
            System.out.print(sb.toString());
        } catch (IOException e) {
            e.printStackTrace();
        }
        finally {
            try {
                if (fileWriter != null)
                    fileWriter.close();
                if (bufferedWriter != null)
                    bufferedWriter.close();
            } catch(Exception e) {
                e.printStackTrace();
            }
        }
    }
}