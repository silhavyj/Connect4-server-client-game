/**
 * @author silhavyj A147B0362P
 *
 * This enumeration represents all messages that
 * can be received from the server. Each message whithin
 * this enumeration comes with a method for validation.
 * */
public enum ReceivedMsg {

    /** Adds a new client to the list of clients */
    ADD_CLIENT {
        /**
         * Validates the ADD_CLIENT message
         * @param tokens message split up into tokens
         * @return true if the message is valid, false otherwise
         * */
        @Override
        public boolean isValid(String[] tokens) {
            if (tokens.length != 2)
                return false;
            return true;
        }
    },
    /** Removes a client from the list of clients */
    REMOVE_CLIENT {
        /**
         * Validates the REMOVE_CLIENT message
         * @param tokens message split up into tokens
         * @return true if the message is valid, false otherwise
         * */
        @Override
        public boolean isValid(String[] tokens) {
            if (tokens.length != 2)
                return false;
            return true;
        }
    },
    /** Changes the sate of a client */
    GAME_PLAYER_STATE {
        /**
         * Validates the GAME_PLAYER_STATE message
         * @param tokens message split up into tokens
         * @return true if the message is valid, false otherwise
         * */
        @Override
        public boolean isValid(String[] tokens) {
            if (tokens.length != 3)
                return false;
            return tokens[2].equals("ON") || tokens[2].equals("OFF");
        }
    },
    /** Receives a game request from another client */
    RQ {
        /**
         * Validates the RQ message
         * @param tokens message split up into tokens
         * @return true if the message is valid, false otherwise
         * */
        @Override
        public boolean isValid(String[] tokens) {
            if (tokens.length != 2)
                return false;
            return true;
        }
    },
    /** The client was not following the protocol */
    INVALID_PROTOCOL {
        /**
         * Validates the INVALID_PROTOCOL message
         * @param tokens message split up into tokens
         * @return true if the message is valid, false otherwise
         * */
        @Override
        public boolean isValid(String[] tokens) {
            return true;
        }
    },
    /** Acknowledge message */
    OK {
        /**
         * Validates the OK message
         * @param tokens message split up into tokens
         * @return true if the message is valid, false otherwise
         * */
        @Override
        public boolean isValid(String[] tokens) {
            if (tokens.length != 1)
                return false;
            return true;
        }
    },
    /** Cancels a received game request */
    RQ_CANCELED {
        /**
         * Validates the RQ_CANCELED message
         * @param tokens message split up into tokens
         * @return true if the message is valid, false otherwise
         * */
        @Override
        public boolean isValid(String[] tokens) {
            if (tokens.length != 2)
                return false;
            return true;
        }
    },
    /** Cancels a received game request */
    GAME_START {
        /**
         * Validates the GAME_START message
         * @param tokens message split up into tokens
         * @return true if the message is valid, false otherwise
         * */
        @Override
        public boolean isValid(String[] tokens) {
            if (!(tokens.length == 2 || tokens.length == 3))
                return false;
            return true;
        }
    },
    /** Plays game (info on where to place the disk) */
    GAME_PLAY {
        /**
         * Validates the GAME_PLAY message
         * @param tokens message split up into tokens
         * @return true if the message is valid, false otherwise
         * */
        @Override
        public boolean isValid(String[] tokens) {
            if (tokens.length != 4)
                return false;
            try {
                int y = Integer.parseInt(tokens[2]);
                int x = Integer.parseInt(tokens[3]);
                if (y < 0 || y >= Connect4.ROWS || x < 0 || x >= Connect4.COLUMNS)
                    return false;
            } catch (Exception e) {
                return false;
            }
            return true;
        }
    },
    /** Cancels a game */
    GAME_CANCELED,
    /** Received a game message */
    GAME_MSG,
    /** Received a result of the game */
    GAME_RESULT,
    /** Received a lit of winning disks (tails) */
    GAME_WINNING_TAILS {
        /**
         * Validates the GAME_WINNING_TAILS message
         * @param tokens message split up into tokens
         * @return true if the message is valid, false otherwise
         * */
        @Override
        public boolean isValid(String[] tokens) {
            if (tokens.length != 9)
                return false;
            try {
                for (int i = 1; i < 9; i++) {
                    int y = Integer.parseInt(tokens[i]);
                    int x = Integer.parseInt(tokens[i+1]);
                    if (y < 0 || y >= Connect4.ROWS || x < 0 || x >= Connect4.COLUMNS)
                        return false;
                    i++;
                }
            } catch (Exception e) {
                return false;
            }
            return true;
        }
    },
    /**
     * Recovers the game after the client gets
     * connected back to the server
     * */
    GAME_RECOVERY {
        /**
         * Validates the GAME_RECOVERY message
         * @param tokens message split up into tokens
         * @return true if the message is valid, false otherwise
         * */
        @Override
        public boolean isValid(String[] tokens) {
            if (tokens.length != Connect4.ROWS * Connect4.COLUMNS + 1)
                return false;
            try {
                for (int i = 1; i < Connect4.ROWS * Connect4.COLUMNS + 1; i++) {
                    int value = Integer.parseInt(tokens[i]);
                    if (!(value == 0 || value == 1 || value == 2))
                        return false;
                }
            } catch (Exception e) {
                return false;
            }
            return true;
        }
    };

    /**
     * Validation method of every message
     * @param tokens message split up into tokens
     * @return true if the message is valid, false otherwise
     * */
    public boolean isValid(String[] tokens) {
        return true;
    }
}