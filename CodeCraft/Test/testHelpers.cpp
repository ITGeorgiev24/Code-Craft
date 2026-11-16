#include "testHelpers.h"
#include "../Logic/gameLogic.h"

using namespace std;

const string sampleMaze[10] = {
    "..k.$.c...",
    "D########.",
    ".########.",
    ".########.",
    ".########.",
    ".########.",
    ".########.",
    ".########.",
    ".########.",
    ".........E",
};

Position makePosition(int row, int col) {
    Position position;
    position.row = row;
    position.col = col;
    return position;
}

void buildMaze(Maze& maze, const string rows[], int size) {
    createMaze(maze, size);
    for (int row = 0; row < size; row++) {
        for (int col = 0; col < size; col++) {
            Tile& tile = maze.tiles[row][col];
            char symbol = rows[row][col];
            tile.type = TILE_PATH;
            tile.item = ITEM_NONE;
            if (symbol == '#') tile.type = TILE_WALL;
            if (symbol == 'D') tile.type = TILE_DOOR;
            if (symbol == 'E') tile.type = TILE_EXIT;
            if (symbol == 'k') tile.item = ITEM_KEY;
            if (symbol == '$') tile.item = ITEM_COIN;
            if (symbol == 'c') tile.item = ITEM_COMPASS;
        }
    }
}

void buildGame(GameState& game, const string rows[], int size) {
    endGame(game);
    buildMaze(game.maze, rows, size);
    game.player = Player();
    game.status = PLAYING;
    game.difficulty = NORMAL;
    game.fogOfWar = false;
    game.optimalMoves = 0;
    game.hintPenalty = 0;
    game.hintsUsed = 0;
    game.hintVisible = false;
    game.elapsedMs = 0;
    game.chaser = Chaser();
    game.totalCoins = 0;
    for (int row = 0; row < size; row++) {
        for (int col = 0; col < size; col++) {
            if (game.maze.tiles[row][col].item == ITEM_COIN) {
                game.totalCoins++;
            }
        }
    }
}
