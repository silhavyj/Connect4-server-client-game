import javafx.animation.TranslateTransition;
import javafx.geometry.Insets;
import javafx.geometry.Pos;
import javafx.scene.Cursor;
import javafx.scene.Node;
import javafx.scene.Scene;
import javafx.scene.control.Button;
import javafx.scene.control.Label;
import javafx.scene.effect.Light;
import javafx.scene.effect.Lighting;
import javafx.scene.layout.HBox;
import javafx.scene.layout.Pane;
import javafx.scene.layout.VBox;
import javafx.scene.paint.Color;
import javafx.scene.shape.Circle;
import javafx.scene.shape.Rectangle;
import javafx.scene.shape.Shape;
import javafx.scene.text.Font;
import javafx.util.Duration;

import java.util.ArrayList;
import java.util.List;

/**
 * @author silhavyj A17B0362P
 *
 * This class represets the game window where the
 * client plays the a game. It is consis of all
 * graphical elements necessary to play the game.
 * This class extends class VBox (JavaFX).
 * */
public class Connect4 extends VBox {

    /** font size of the title of the window */
    private static final int TITLE_FONT_SIZE = 24;
    /** size of one tile on the grid (playing board) */
    private static final int TILE_SIZE = 80;
    /** number of rows on the grid */
    public static final int ROWS = 6;
    /** number of columns on the grid */
    public static final int COLUMNS = 7;
    /** size of a disk the user places on the board */
    private static final double CIRCLE_SIZE = TILE_SIZE / 2.5;
    /** total width of the form */
    public static final int FORM_WIDTH = COLUMNS * TILE_SIZE + 30;
    /** total height of the form */
    public static final int FORM_HEIGHT = 750;

    /** spacing between buttons in the first row (below the board) */
    private static final int ROW_OF_BUTTONS_SPACING = 20;
    /** font size of the message received from the server */
    private static final int MSG_LABEL_FONT_SIZE = 18;
    /** font size of the label holding information about the result of the game */
    private static final int GAME_RESULT_LABEL_FONT_SIZE = 22;
    /** title of the button to clear up the message */
    private static final String BUTTON_CLEAR_MSG_TXT = "Clear message";
    /** tile of the button to cancel the game */
    private static final String BUTTON_CANCEL_GAME_TXT = "Cancel game";
    /** title of the button to close the window (close up the game when it is over)*/
    private static final String BUTTON_CLOSE_WINDOW_TXT = "Close window";
    /** duration of the animation of a dropping disk */
    private static final double DROP_ANIMATION_DURATION = 0.5;
    /** color of the player1's disks */
    private static final Color PLAYER_1_COLOR = Color.GREEN;
    /** color of the player2's disks */
    private static final Color PLAYER_2_COLOR = Color.RED;
    /** color of the winning "row" of disks */
    private static final Color WINNING_TAILS_COLOR = Color.PINK;
    /** stroke color of the winning "row" of disks */
    private static final Color WINNING_TAILS_STROKE_COLOR = Color.RED;
    /** thickness of the edge of the winning "row" of disks */
    private static final double WINNING_TAILS_STROKE_THICKNESS = 3;

    /** instance of the main class */
    private Main main;
    /** instance of the main scene of the application */
    private Scene mainScene;

    /** pane containing players' disks */
    private Pane diskPane;
    /** 2D array of disks */
    private Circle[][] disks;
    /** label holding game messages from the server */
    private Label msgLabel;
    /** columns allowing the player to choose x position */
    private List<Rectangle> columns;
    /** button for closing the window (when the game is over) */
    private Button btnCloseWindow;
    /** button for canceling the game (on purpose) */
    private Button btnGameCancel;
    /** label holding the result of the game (won, lost, draw)*/
    private Label gameResultLbl;

    /**
     * Constructor of the class - creates an instance of it
     * @param main reference to the main class
     * @param mainScene main scene of the application
     * @param  nick player's nickname
     * */
    public Connect4(Main main, Scene mainScene, String nick) {
        super();

        this.main = main;
        this.mainScene = mainScene;

        disks = new Circle[ROWS][COLUMNS];

        // create the content of the window
        getChildren().add(createTitle(nick));
        getChildren().add(createGridPane());
        getChildren().add(createFirstRowOfButtons());
        getChildren().add(createSecondRowOfButtons());
        getChildren().add(createGameResultLabel());

        setSpacing(10);
        setPadding(new Insets(10,10,10,10));
        setAlignment(Pos.TOP_CENTER);
    }

    /**
     * Displays a message containing the result of the game
     * @param msg message that is going to be displayed in the label
     * */
    public void setGameResultLabel(String msg) {
        gameResultLbl.setText(msg);
    }

    /**
     * Creates a label containing the result of the game
     * @return label that is going to display the result of the game
     * */
    private Label createGameResultLabel() {
        gameResultLbl = new Label();
        gameResultLbl.setFont(new Font(GAME_RESULT_LABEL_FONT_SIZE));
        gameResultLbl.setPrefWidth(COLUMNS * TILE_SIZE);
        gameResultLbl.setAlignment(Pos.CENTER);
        gameResultLbl.setStyle("-fx-text-fill: blue;");
        return gameResultLbl;
    }

    /**
     * Disables all the columns (x positions) so the user can no
     * longer click on it and play the game
     * */
    private void disableColumns() {
        for (Node rect : columns)
            rect.setDisable(true);
    }

    /**
     * Cancels the game, meaning it disables
     * all the columns (x positions), displays
     * the appropriate message given as a parameter, etc.
     * @param msg message from the server that is going to be displayed
     * */
    public void gameCanceled(String msg) {
        disableColumns();
        setGameMessageLbl(msg);
        btnCloseWindow.setDisable(false);
        btnGameCancel.setDisable(true);
    }

    /**
     * Creates a new disk and places it on the grid
     * @param y position y of the disk
     * @param x position x of the disk
     * @param player1 true/false which player placed the disk
     * */
    public void createDisk(int y, int x, boolean player1) {
        Circle circle = new Circle(CIRCLE_SIZE);
        if (player1)
            circle.setFill(PLAYER_1_COLOR);
        else circle.setFill(PLAYER_2_COLOR);

        circle.setCenterX(TILE_SIZE / 2);
        circle.setCenterY(TILE_SIZE / 2);
        circle.setTranslateX(x * TILE_SIZE);
        diskPane.getChildren().add(circle);

        // creates the drop animation and plays it
        TranslateTransition fallAnimation = new TranslateTransition(Duration.seconds(DROP_ANIMATION_DURATION), circle);
        fallAnimation.setToY(y * TILE_SIZE);
        fallAnimation.play();
        disks[y][x] = circle;
    }

    /**
     * Creates the second row of buttons below the grid
     * (play board) - these are the close window button
     * and cancel game button
     * @return hBox holding the two buttons
     * */
    private HBox createSecondRowOfButtons() {
        HBox hBox = new HBox();
        hBox.setAlignment(Pos.CENTER_RIGHT);
        hBox.setSpacing(ROW_OF_BUTTONS_SPACING);

        hBox.getChildren().add(createButtonCloseWindow());
        hBox.getChildren().add(createButtonCancelGame());
        return hBox;
    }

    /**
     * Creates the button for canceling the game.
     * The game can be canceled on purpose. For example,
     * when the user gets bored playing.
     * @return cancel game button
     * */
    private Button createButtonCancelGame() {
        btnGameCancel = new Button(BUTTON_CANCEL_GAME_TXT);
        btnGameCancel.setOnMouseClicked(e -> main.cancelGame());
        return btnGameCancel;
    }

    /**
     * Highlights the winning disks (tails)
     * when the game is over.
     * @param tails [y,x] positions of all the winning tiles
     * */
    public void colorWinningTails(List<int[]> tails) {
        for (int[] coordinates : tails) {
            int y = coordinates[0];
            int x = coordinates[1];
            disks[y][x].setFill(WINNING_TAILS_COLOR);
            disks[y][x].setStroke(WINNING_TAILS_STROKE_COLOR);
            disks[y][x].setStrokeWidth(WINNING_TAILS_STROKE_THICKNESS);
        }
    }

    /**
     * Creates the button for closing the window.
     * The user can click on this button as soon
     * as the game is over.
     * @return close window button
     * */
    private Button createButtonCloseWindow() {
        btnCloseWindow = new Button(BUTTON_CLOSE_WINDOW_TXT);
        btnCloseWindow.setOnMouseClicked(e -> main.displayLobby());
        btnCloseWindow.setDisable(true);
        return btnCloseWindow;
    }

    /**
     * Creates the first row of buttons below the
     * grid (play board) - these are the button to
     * clear up the game message from the server
     * and the message itself.
     * @return hBox containing these elements
     * */
    private HBox createFirstRowOfButtons() {
        HBox hBox = new HBox();
        hBox.setAlignment(Pos.CENTER_RIGHT);
        hBox.setSpacing(ROW_OF_BUTTONS_SPACING);

        hBox.getChildren().add(createMsgLabel());
        hBox.getChildren().add(createButtonClearMsg());
        return hBox;
    }

    /**
     * Displays the game message received from the server
     * containing some additional information for the user
     * @param msg game message from the server
     * */
    public void setGameMessageLbl(String msg) {
        msgLabel.setText(msg);
    }

    /**
     * Creates the button to clear up the game message
     * received from the server.
     * @return button to clear the game message
     * */
    private Button createButtonClearMsg() {
        Button btn = new Button(BUTTON_CLEAR_MSG_TXT);
        btn.setOnMouseClicked(e -> setGameMessageLbl(""));
        return btn;
    }

    /**
     * Creates a label holding a game message
     * received from the server
     * @return label that is going to display a game message from the server
     * */
    private Label createMsgLabel() {
        msgLabel = new Label();
        msgLabel.setFont(new Font(MSG_LABEL_FONT_SIZE));
        msgLabel.setPrefWidth((COLUMNS - 2) * TILE_SIZE);
        msgLabel.setAlignment(Pos.CENTER);
        msgLabel.setStyle("-fx-text-fill: blue;");
        msgLabel.setWrapText(true);
        return msgLabel;
    }

    /**
     * Creates a grid pane holding all the elements
     * used to play the game (columns - x position,
     * disk pane, ...)
     * @return grid pane of the game
     * */
    private Pane createGridPane() {
        Pane gridPane = new Pane();
        gridPane.getChildren().add((diskPane = new Pane()));
        gridPane.getChildren().add(createGrid());
        gridPane.getChildren().addAll(createColumns());
        return gridPane;
    }

    /**
     * Creates all the columns of the game
     * the user use for choosing the x position
     * of where they want to plays their disk.
     * @return a lit of "columns" rectangles
     * */
    private List<Rectangle> createColumns() {
        columns = new ArrayList<>();
        for (int i = 0; i < COLUMNS; i++) {
            Rectangle rectangle = new Rectangle(TILE_SIZE,TILE_SIZE * ROWS);
            rectangle.setTranslateX(i * TILE_SIZE);
            rectangle.setFill(Color.TRANSPARENT);
            rectangle.setOnMouseEntered(e -> {
                rectangle.setFill(Color.rgb(0, 0, 0, 0.1));
                mainScene.setCursor(Cursor.HAND);
            });
            rectangle.setOnMouseExited(e -> {
                rectangle.setFill(Color.TRANSPARENT);
                mainScene.setCursor(Cursor.DEFAULT);
            });
            rectangle.setOnMouseClicked(e -> {
                for (int j = 0; j < columns.size(); j++)
                    if (columns.get(j) == rectangle) {
                        main.playGame(j);
                        break;
                    }
            });
            columns.add(rectangle);
        }
        return columns;
    }

    /**
     * Creates the grid of the game (rectangle with holes in it)
     * @return the grid of the game
     * */
    private Shape createGrid() {
        Shape shape = new Rectangle(COLUMNS * TILE_SIZE, ROWS * TILE_SIZE);
        for (int i = 0; i < ROWS; i++)
            for (int j = 0; j < COLUMNS; j++) {
                Circle circle = new Circle(CIRCLE_SIZE);
                circle.setCenterX(TILE_SIZE / 2);
                circle.setCenterY(TILE_SIZE / 2);
                circle.setTranslateX(j * TILE_SIZE);
                circle.setTranslateY(i * TILE_SIZE);
                shape = Shape.subtract(shape, circle);
            }

        Light.Distant light = new Light.Distant();
        light.setAzimuth(0.45);
        light.setElevation(45);

        Lighting lighting = new Lighting();
        lighting.setLight(light);
        lighting.setSurfaceScale(0.5);

        shape.setFill(Color.LIGHTBLUE);
        shape.setEffect(lighting);
        return shape;
    }

    /** Creates the tile of the window.
     * @param nick nick of the player's opponent
     * @return title of the game window
     * */
    private Label createTitle(String nick) {
        Label lbl = new Label("Connect 4 against '" + nick + "'");
        lbl.setFont(new Font(TITLE_FONT_SIZE));
        return lbl;
    }
}