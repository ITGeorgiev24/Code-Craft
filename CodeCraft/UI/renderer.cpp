
#include "renderer.h"
#include "console.h"
#include "layout.h"

#include <sstream>

using namespace std;

const int reservedRows = 9;
const int minView = 5;


string wallSymbol()   { return paint("##", "\x1b[38;5;110m"); }
string doorSymbol()   { return paint("[]", magenta); }
string exitSymbol()   { return paint("EX", exitColor); }
string playerSymbol() { return paint("@ ", bold + yellow); }
string trailSymbol()  { return paint(". ", dark); }
string hintSymbol()   { return paint("* ", cyan); }
string fogSymbol()    { return paint("~~", dark); }
string edgeSymbol()   { return paint("::", dark); }
string arrowSymbol()  { return paint("->", green); }

string itemSymbol(ItemType item) {
    if (item == ITEM_KEY) return paint("k ", yellow);
    if (item == ITEM_COIN) return paint("$ ", green);
    if (item == ITEM_COMPASS) return paint("c ", cyan);
    return "  ";
}

string difficultyName(Difficulty difficulty) {
    if (difficulty == EASY) return "Easy";
    if (difficulty == NORMAL) return "Normal";
    if (difficulty == HARD) return "Hard";
    return "Expert";
}

string squareSymbol(const GameState& game, int row, int col) {
    if (isPlayerAt(game, row, col)) {
        return playerSymbol();
    }
    if (!isTileVisible(game, row, col)) {
        return isHintTile(game, row, col) ? hintSymbol() : fogSymbol();
    }
    TileType type = getTileType(game, row, col);
    if (type == TILE_WALL) return wallSymbol();
    if (type == TILE_DOOR) return doorSymbol();
    if (type == TILE_EXIT) return exitSymbol();
    if (getItemAt(game, row, col) != ITEM_NONE) return itemSymbol(getItemAt(game, row, col));
    if (isHintTile(game, row, col)) return hintSymbol();
    if (isVisited(game, row, col)) return trailSymbol();
    return "  ";
}

string frameSymbol(bool mazeContinues) {
    return mazeContinues ? edgeSymbol() : wallSymbol();
}

int limitValue(int value, int minValue, int maxValue) {
    if (value < minValue) return minValue;
    if (value > maxValue) return maxValue;
    return value;
}

string buildHeader(const GameState& game, const DrawOptions& options) {
    int size = getMazeSize(game);
    string header = paint(" MAZEBOUND ", titleTag);
    if (options.caption != "") {
        header += "  " + paint(options.caption, bold + white);
    }
    header += "  " + to_string(size) + "x" + to_string(size) + "  " + difficultyName(getDifficulty(game));
    if (hasFog(game)) {
        header += "  " + paint("[fog]", muted);
    }
    if (isHintVisible(game)) {
        header += "  " + paint("[hint]", cyan);
    }
    return header;
}

string buildStats(const GameState& game) {
    Player player = getPlayer(game);
    return "Moves " + paint(to_string(player.moves), bold + white)
         + " (best " + to_string(getOptimalMoves(game)) + ")"
         + "   " + paint("Keys", yellow) + " " + to_string(player.keys)
         + "   " + paint("Coins", green) + " " + to_string(player.coins) + "/" + to_string(getTotalCoins(game))
         + "   " + paint("Compass", cyan) + " " + to_string(player.compasses)
         + "   Score " + paint(to_string(calculateScore(game)), bold + yellow);
}

string buildLegend(bool fog) {
    string legend;
    if (fog) {
        legend += fogSymbol() + " Fog  ";
    }
    legend += playerSymbol() + "You  " + wallSymbol() + " Wall  " + doorSymbol() + " Door  "
            + exitSymbol() + " Exit  " + itemSymbol(ITEM_KEY) + "Key  " + itemSymbol(ITEM_COIN) + "Coin  "
            + itemSymbol(ITEM_COMPASS) + "Compass  " + hintSymbol() + "Hint";
    return legend;
}


string buildGameFrame(const GameState& game, const DrawOptions& options, const string& message,
                      int columns, int rows) {
    int size = getMazeSize(game);
    Position player = getPlayer(game).position;

    int viewRows = size;
    int viewCols = size;
    int top = 0;
    int left = 0;
    if (!options.showFullMap) {
        viewRows = limitValue(rows - reservedRows, minView, size);
        viewCols = limitValue((columns - 6) / 2, minView, size);
        top = limitValue(player.row - viewRows / 2, 0, size - viewRows);
        left = limitValue(player.col - viewCols / 2, 0, size - viewCols);
    }
    bool cutTop = top > 0;
    bool cutBottom = top + viewRows < size;
    bool cutLeft = left > 0;
    bool cutRight = left + viewCols < size;

    string maze = wallSymbol();
    for (int col = 0; col < viewCols; col++) {
        maze += frameSymbol(cutTop);
    }
    maze += wallSymbol() + "\n";

    for (int row = top; row < top + viewRows; row++) {
        maze += (!cutLeft && row == 0) ? arrowSymbol() : frameSymbol(cutLeft);
        for (int col = left; col < left + viewCols; col++) {
            maze += squareSymbol(game, row, col);
        }
        maze += (!cutRight && row == size - 1) ? arrowSymbol() : frameSymbol(cutRight);
        maze += "\n";
    }

    maze += wallSymbol();
    for (int col = 0; col < viewCols; col++) {
        maze += frameSymbol(cutBottom);
    }
    maze += wallSymbol() + "\n";

    string content;
    content += centerLine(buildHeader(game, options), columns) + "\n";
    content += centerLine(buildStats(game), columns) + "\n";
    content += centerBlock(maze, columns);
    content += centerLine(buildLegend(hasFog(game)), columns) + "\n";
    content += centerLine(paint("Arrows/WASD move  H hint  I info  L legend  M map  R restart  ? help  Q quit",
                                muted), columns) + "\n";

    stringstream messageStream(message);
    string messageLine;
    for (int i = 0; i < 2; i++) {
        if (getline(messageStream, messageLine)) {
            content += centerLine(messageLine, columns);
        }
        content += "\n";
    }

    string frame;
    if (!options.showFullMap) {
        int topSpace = (rows - 1 - countLines(content)) / 2;
        for (int i = 0; i < topSpace; i++) {
            frame += "\n";
        }
    }
    frame += content;

    string result;
    stringstream frameStream(frame);
    string line;
    int lineCount = 0;
    while (getline(frameStream, line)) {
        result += padRight(line, columns - 1) + "\n";
        lineCount++;
    }
    for (int i = lineCount; i < rows - 1; i++) {
        result += string(columns - 1, ' ') + "\n";
    }
    return result;
}

void drawGame(const GameState& game, const DrawOptions& options, const string& message) {
    int columns = 0;
    int rows = 0;
    getConsoleSize(columns, rows);
    string frame = buildGameFrame(game, options, message, columns, rows);
    if (options.showFullMap) {
        clearScreen();
    } else {
        moveCursorHome();
    }
    writeText(frame);
}
