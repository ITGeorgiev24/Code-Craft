#include "renderer.h"
#include "artwork.h"
#include "console.h"
#include "layout.h"

#include <sstream>

using namespace std;

const int reservedRows = 9;
const int minView = 5;
const int artGap = 3;                     
const int minViewWithArt = 16;                  

int statusRow = -1;

string wallSymbol()   { return paint("##", "\x1b[38;5;110m"); }
string doorSymbol()   { return paint("[]", magenta); }
string exitSymbol()   { return paint("EX", exitColor); }
string playerSymbol() { return paint("@ ", bold + yellow); }
string chaserSymbol() { return paint("& ", bold + red); }
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

string formatTime(int seconds) {
    if (seconds < 0) {
        seconds = 0;
    }
    int hours = seconds / 3600;
    int minutes = (seconds / 60) % 60;
    int rest = seconds % 60;
    string text = (rest < 10 ? "0" : "") + to_string(rest);
    if (hours > 0) {
        return to_string(hours) + ":" + (minutes < 10 ? "0" : "") + to_string(minutes) + ":" + text;
    }
    return to_string(minutes) + ":" + text;
}

string squareSymbol(const GameState& game, int row, int col) {
    if (isPlayerAt(game, row, col)) {
        return playerSymbol();
    }
    if (isChaserAt(game, row, col)) {
        return isChaserFrozen(game) ? paint("& ", bold + blue) : chaserSymbol();
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
    header += "  " + paint(to_string(size) + "x" + to_string(size) + " " + difficultyName(getDifficulty(game)), muted);
    if (hasFog(game)) {
        header += "  " + paint("[darkness]", muted);
    }
    if (isHintVisible(game)) {
        header += "  " + paint("[hint]", cyan);
    }
    return header;
}

string buildStats(const GameState& game) {
    Player player = getPlayer(game);
    string stats = paint("Time", orange) + " " + paint(formatTime(getElapsedSeconds(game)), bold + white)
                 + "   Moves " + paint(to_string(player.moves), bold + white)
                 + " (best " + to_string(getOptimalMoves(game)) + ")"
                 + "   " + paint("Keys", yellow) + " " + to_string(player.keys)
                 + "   " + paint("Coins", green) + " " + to_string(player.coins) + "/" + to_string(getTotalCoins(game))
                 + "   " + paint("Compass", cyan) + " " + to_string(player.compasses)
                 + "   Score " + paint(to_string(calculateScore(game)), bold + yellow);

    if (hasChaser(game)) {
        string warden;
        if (!isChaserActive(game)) {
            warden = player.moves == 0 ? paint("Warden: sleeping", muted)
                                       : paint("Warden wakes in " + to_string(getChaserWakeSeconds(game)) + "s", orange);
        } else if (isChaserFrozen(game)) {
            warden = paint("Warden: FROZEN", bold + blue);
        } else {
            int distance = getChaserDistance(game);
            warden = distance <= 6 ? paint("Warden: " + to_string(distance) + " steps! RUN!", bold + red)
                                   : paint("Warden: " + to_string(distance) + " steps", red);
        }
        stats += "   " + warden;
    }
    return stats;
}

string buildLegend(const GameState& game) {
    string legend;
    if (hasFog(game)) {
        legend += fogSymbol() + " Dark  ";
    }
    if (hasChaser(game)) {
        legend += chaserSymbol() + "Warden  ";
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

    string leftArt;
    string rightArt;
    int artSpace = 0;
    if (options.level > 0) {
        leftArt = levelArt(options.level, 0);
        rightArt = levelArt(options.level, 1);
        artSpace = blockWidth(leftArt) + blockWidth(rightArt) + 2 * artGap;
    }

    int viewRows = limitValue(rows - reservedRows, minView, size);
    int viewCols = limitValue((columns - 6 - artSpace) / 2, minView, size);
    if (artSpace > 0) {
        int artHeight = blockHeight(leftArt) > blockHeight(rightArt) ? blockHeight(leftArt) : blockHeight(rightArt);
        bool tooNarrow = viewCols < size && viewCols < minViewWithArt;
        bool tooLow = artHeight > rows - reservedRows + 2;
        if (tooNarrow || tooLow) {
            leftArt = "";         
            rightArt = "";
            viewCols = limitValue((columns - 6) / 2, minView, size);
        }
    }
    int top = limitValue(player.row - viewRows / 2, 0, size - viewRows);
    int left = limitValue(player.col - viewCols / 2, 0, size - viewCols);
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
    int statsLine = countLines(content);
    content += centerLine(buildStats(game), columns) + "\n";
    if (leftArt != "") {
        content += centerBlock(besideEachOther(leftArt, maze, rightArt, artGap), columns);
    } else {
        content += centerBlock(maze, columns);
    }
    content += centerLine(buildLegend(game), columns) + "\n";
    content += centerLine(paint("Arrows/WASD move   H hint   I info   L legend   R restart   ? help   Esc menu",
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
    int topSpace = (rows - 1 - countLines(content)) / 2;
    if (topSpace < 0) {
        topSpace = 0;
    }
    for (int i = 0; i < topSpace; i++) {
        frame += "\n";
    }
    frame += content;
    statusRow = topSpace + statsLine + 1;          

    string result;
    stringstream frameStream(frame);
    string line;
    int lineCount = 0;
    while (getline(frameStream, line)) {
        result += padRight(line + reset, columns - 1) + "\n";
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

    static int lastColumns = -1;
    static int lastRows = -1;
    if (columns != lastColumns || rows != lastRows) {
        clearScreen();       
        lastColumns = columns;
        lastRows = rows;
    }

    string frame = buildGameFrame(game, options, message, columns, rows);
    moveCursorHome();
    writeText(frame);
}

void drawStatusLine(const GameState& game) {
    if (statusRow < 1) {
        return;
    }
    int columns = getScreenWidth();
    string line = padRight(centerLine(buildStats(game), columns) + reset, columns - 1);
    writeText("\x1b[" + to_string(statusRow) + ";1H" + line);
}
