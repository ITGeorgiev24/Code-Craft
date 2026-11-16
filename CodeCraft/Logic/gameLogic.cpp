#include "gameLogic.h"
#include "mazeGenerator.h"
#include "pathfinder.h"
#include "validation.h"

#include <cstdlib>
#include <ctime>

using namespace std;

Position exitPositionOf(const GameState& game) {
    Position exitPosition;
    exitPosition.row = game.maze.size - 1;
    exitPosition.col = game.maze.size - 1;
    return exitPosition;
}

Position nextPosition(Position from, Direction direction) {
    Position next = from;
    if (direction == DIR_UP) {
        next.row--;
    } else if (direction == DIR_DOWN) {
        next.row++;
    } else if (direction == DIR_LEFT) {
        next.col--;
    } else {
        next.col++;
    }
    return next;
}

void revealAroundPlayer(GameState& game) {
    Position center = game.player.position;
    for (int row = center.row - fogVision; row <= center.row + fogVision; row++) {
        for (int col = center.col - fogVision; col <= center.col + fogVision; col++) {
            if (isInside(game.maze, row, col)) {
                game.maze.tiles[row][col].seen = true;
            }
        }
    }
}

void updateHint(GameState& game) {
    if (!game.hintVisible) {
        return;
    }
    int next = game.hintStep + 1;
    if (next < game.hintPath.count && samePosition(game.hintPath.items[next], game.player.position)) {
        game.hintStep = next;
        if (game.hintStep >= game.hintPath.count - 1) {
            game.hintVisible = false;
        }
    } else {
        game.hintVisible = false;
    }
}

RewardType giveReward(GameState& game) {
    RewardType reward = REWARD_GOLD;
    int kind = game.player.rewards % 3;
    if (kind == 0) {
        reward = REWARD_COMPASS;
        game.player.compasses++;
    } else if (kind == 1) {
        reward = REWARD_KEY;
        game.player.keys++;
    } else {
        game.player.bonusPoints += rewardPoints;
    }
    game.player.rewards++;
    if (game.chaser.active) {
        game.chaser.frozenMs = chaserFreezeMs;
    }
    return reward;
}

void moveChaserOneStep(GameState& game) {
    int size = game.maze.size;
    int* distances = new int[size * size];
    if (findDistances(game.maze, game.player.position, false, distances)) {
        Position from = game.chaser.position;
        int current = distances[from.row * size + from.col];
        for (int direction = 0; direction < 4 && current > 0; direction++) {
            Position next = nextPosition(from, (Direction)direction);
            if (isInside(game.maze, next.row, next.col)) {
                int distance = distances[next.row * size + next.col];
                if (distance >= 0 && distance < current) {
                    game.chaser.position = next;
                    break;
                }
            }
        }
    }
    delete[] distances;
}

int countDoorsOnPath(const GameState& game, const PositionList& path) {
    int doors = 0;
    for (int i = 0; i < path.count; i++) {
        if (game.maze.tiles[path.items[i].row][path.items[i].col].type == TILE_DOOR) {
            doors++;
        }
    }
    return doors;
}

void initializeRandom() {
    srand((unsigned int)time(0));
}

unsigned int generateRandomSeed() {
    return (unsigned int)time(0) * 31u + (unsigned int)rand();
}

bool startNewGame(GameState& game, int size, Difficulty difficulty) {
    return startGameWithSeed(game, size, difficulty, generateRandomSeed());
}

bool startGameWithSeed(GameState& game, int size, Difficulty difficulty, unsigned int seed) {
    endGame(game);
    GenerationReport report;
    if (!generateMaze(game.maze, size, difficulty, seed, report)) {
        return false;
    }

    game.player = Player();
    game.difficulty = difficulty;
    game.seed = seed;
    game.fogOfWar = getDifficultySettings(difficulty).fogOfWar;
    game.totalCoins = report.coins;
    game.optimalMoves = shortestPathLength(game.maze, game.player.position, exitPositionOf(game), false);
    game.hintStep = 0;
    game.hintVisible = false;
    game.hintsUsed = 0;
    game.hintPenalty = 0;
    game.elapsedMs = 0;
    game.chaser = Chaser();         
    game.status = PLAYING;
    revealAroundPlayer(game);
    return true;
}

bool restartGame(GameState& game) {
    Chaser oldChaser = game.chaser;
    if (!startGameWithSeed(game, game.maze.size, game.difficulty, game.seed)) {
        return false;
    }
    if (oldChaser.enabled) {
        enableChaser(game, oldChaser.wakeAfterMs, oldChaser.stepMs);
    }
    return true;
}

void enableChaser(GameState& game, int wakeAfterMs, int stepMs) {
    game.chaser = Chaser();
    game.chaser.enabled = true;
    game.chaser.wakeAfterMs = wakeAfterMs < 0 ? 0 : wakeAfterMs;
    game.chaser.stepMs = stepMs < 50 ? 50 : stepMs;          
}

TimeEvent advanceTime(GameState& game, int deltaMs) {
    if (game.status != PLAYING || game.player.moves == 0 || deltaMs <= 0) {
        return TIME_NOTHING;
    }
    game.elapsedMs += deltaMs;

    Chaser& chaser = game.chaser;
    if (!chaser.enabled) {
        return TIME_NOTHING;
    }

    if (!chaser.active) {
        int distanceFromStart = game.player.position.row + game.player.position.col;
        if (game.elapsedMs >= chaser.wakeAfterMs && distanceFromStart >= chaserSafeDistance) {
            chaser.active = true;
            chaser.position = Position();
            chaser.stepTimerMs = 0;
            return TIME_CHASER_APPEARED;
        }
        return TIME_NOTHING;
    }

    if (chaser.frozenMs > 0) {
        chaser.frozenMs -= deltaMs;
        if (chaser.frozenMs > 0) {
            return TIME_NOTHING;
        }
        deltaMs = -chaser.frozenMs;           
        chaser.frozenMs = 0;
    }

    chaser.stepTimerMs += deltaMs;
    bool moved = false;
    while (chaser.stepTimerMs >= chaser.stepMs) {
        chaser.stepTimerMs -= chaser.stepMs;
        moveChaserOneStep(game);
        moved = true;
        if (samePosition(chaser.position, game.player.position)) {
            game.status = CAUGHT;
            game.hintVisible = false;
            return TIME_CAUGHT;
        }
    }
    return moved ? TIME_CHASER_MOVED : TIME_NOTHING;
}

void endGame(GameState& game) {
    deleteMaze(game.maze);
    deleteList(game.hintPath);
    game.hintVisible = false;
    game.status = NOT_STARTED;
}

void quitGame(GameState& game) {
    if (game.status == PLAYING) {
        game.status = QUIT;
    }
}

MoveResult movePlayer(GameState& game, Direction direction) {
    MoveResult result;
    if (game.status != PLAYING) {
        result.status = MOVE_NOT_PLAYING;
        return result;
    }

    Position target = nextPosition(game.player.position, direction);
    if (!isInside(game.maze, target.row, target.col)) {
        result.status = MOVE_EDGE;
        return result;
    }

    Tile& tile = game.maze.tiles[target.row][target.col];
    if (tile.type == TILE_WALL) {
        result.status = MOVE_WALL;
        return result;
    }
    if (tile.type == TILE_DOOR) {
        if (game.player.keys == 0) {
            result.status = MOVE_LOCKED_DOOR;
            return result;
        }
        game.player.keys--;
        game.player.doorsOpened++;
        tile.type = TILE_PATH;
        result.openedDoor = true;
    }

    game.player.position = target;
    game.player.moves++;
    tile.visited = true;

    if (tile.item == ITEM_KEY) {
        game.player.keys++;
    } else if (tile.item == ITEM_COIN) {
        game.player.coins++;
        if (game.player.coins % coinsPerReward == 0) {
            result.reward = giveReward(game);
        }
        if (game.player.coins == game.totalCoins) {
            game.player.bonusPoints += allCoinsBonus;
            result.allCoins = true;
        }
    } else if (tile.item == ITEM_COMPASS) {
        game.player.compasses++;
    }
    result.itemFound = tile.item;
    tile.item = ITEM_NONE;

    updateHint(game);
    revealAroundPlayer(game);

    if (game.chaser.active && samePosition(game.chaser.position, game.player.position)) {
        game.status = CAUGHT;
        game.hintVisible = false;
        result.caught = true;
        result.status = MOVE_OK;
        return result;
    }

    if (tile.type == TILE_EXIT) {
        game.status = WON;
        game.hintVisible = false;
        result.reachedExit = true;
    }
    result.status = MOVE_OK;
    return result;
}

HintResult requestHint(GameState& game) {
    if (game.status != PLAYING) {
        return HINT_NOT_PLAYING;
    }

    Position from = game.player.position;
    Position to = exitPositionOf(game);
    PositionList path;
    bool found = false;

    if (game.player.keys > 0) {
        found = findShortestPath(game.maze, from, to, true, path);
        if (found && countDoorsOnPath(game, path) > game.player.keys) {
            found = false;
        }
    }
    if (!found) {
        found = findShortestPath(game.maze, from, to, false, path);
    }
    if (!found) {
        deleteList(path);
        return HINT_NO_PATH;
    }

    deleteList(game.hintPath);
    game.hintPath = path;
    game.hintStep = 0;
    game.hintVisible = game.hintPath.count > 1;
    game.hintsUsed++;

    if (game.player.compasses > 0) {
        game.player.compasses--;
        return HINT_FREE;
    }
    game.hintPenalty += paidHintCost;
    return HINT_PAID;
}

int getMazeSize(const GameState& game) {
    return game.maze.size;
}

TileType getTileType(const GameState& game, int row, int col) {
    if (!isInside(game.maze, row, col)) {
        return TILE_WALL;
    }
    return game.maze.tiles[row][col].type;
}

ItemType getItemAt(const GameState& game, int row, int col) {
    if (!isInside(game.maze, row, col)) {
        return ITEM_NONE;
    }
    return game.maze.tiles[row][col].item;
}

bool isVisited(const GameState& game, int row, int col) {
    return isInside(game.maze, row, col) && game.maze.tiles[row][col].visited;
}

bool isPlayerAt(const GameState& game, int row, int col) {
    return game.player.position.row == row && game.player.position.col == col;
}

bool isHintTile(const GameState& game, int row, int col) {
    if (!game.hintVisible) {
        return false;
    }
    int last = game.hintStep + hintVisibleSteps;
    if (last > game.hintPath.count - 1) {
        last = game.hintPath.count - 1;
    }
    for (int i = game.hintStep + 1; i <= last; i++) {
        if (game.hintPath.items[i].row == row && game.hintPath.items[i].col == col) {
            return true;
        }
    }
    return false;
}

bool isHintVisible(const GameState& game) {
    return game.hintVisible;
}

bool isTileVisible(const GameState& game, int row, int col) {
    if (!game.fogOfWar) {
        return true;
    }
    if (!isInside(game.maze, row, col)) {
        return false;
    }
    return game.maze.tiles[row][col].seen || game.maze.tiles[row][col].type == TILE_EXIT;
}

bool hasFog(const GameState& game) {
    return game.fogOfWar;
}

Player getPlayer(const GameState& game) {
    return game.player;
}

GameStatus getStatus(const GameState& game) {
    return game.status;
}

Difficulty getDifficulty(const GameState& game) {
    return game.difficulty;
}

int getOptimalMoves(const GameState& game) {
    return game.optimalMoves;
}

int getTotalCoins(const GameState& game) {
    return game.totalCoins;
}

int getHintsUsed(const GameState& game) {
    return game.hintsUsed;
}

int getElapsedSeconds(const GameState& game) {
    return game.elapsedMs / 1000;
}

bool hasChaser(const GameState& game) {
    return game.chaser.enabled;
}

bool isChaserActive(const GameState& game) {
    return game.chaser.enabled && game.chaser.active;
}

bool isChaserAt(const GameState& game, int row, int col) {
    return isChaserActive(game) && game.chaser.position.row == row && game.chaser.position.col == col;
}

bool isChaserFrozen(const GameState& game) {
    return isChaserActive(game) && game.chaser.frozenMs > 0;
}

int getChaserWakeSeconds(const GameState& game) {
    if (!game.chaser.enabled || game.chaser.active) {
        return 0;
    }
    int left = game.chaser.wakeAfterMs - game.elapsedMs;
    return left <= 0 ? 0 : (left + 999) / 1000;
}

int getChaserDistance(const GameState& game) {
    if (!isChaserActive(game) || game.maze.tiles == nullptr) {
        return -1;
    }
    return shortestPathLength(game.maze, game.chaser.position, game.player.position, false);
}

LocationInfo describeLocation(const GameState& game) {
    LocationInfo info;
    if (game.maze.tiles == nullptr) {
        return info;
    }
    Position position = game.player.position;
    info.position = position;
    info.isStart = position.row == 0 && position.col == 0;

    for (int direction = 0; direction < 4; direction++) {
        Position next = nextPosition(position, (Direction)direction);
        if (!isInside(game.maze, next.row, next.col)) {
            info.neighbors[direction] = NEAR_EDGE;
        } else if (game.maze.tiles[next.row][next.col].type == TILE_WALL) {
            info.neighbors[direction] = NEAR_WALL;
        } else if (game.maze.tiles[next.row][next.col].type == TILE_DOOR) {
            info.neighbors[direction] = NEAR_DOOR;
        } else if (game.maze.tiles[next.row][next.col].type == TILE_EXIT) {
            info.neighbors[direction] = NEAR_EXIT;
        } else {
            info.neighbors[direction] = NEAR_OPEN;
        }
    }

    info.stepsToExit = shortestPathLength(game.maze, position, exitPositionOf(game), false);

    for (int row = position.row - nearbyRadius; row <= position.row + nearbyRadius; row++) {
        for (int col = position.col - nearbyRadius; col <= position.col + nearbyRadius; col++) {
            int rowDifference = row > position.row ? row - position.row : position.row - row;
            int colDifference = col > position.col ? col - position.col : position.col - col;
            int steps = rowDifference + colDifference;
            if (steps == 0 || steps > nearbyRadius || !isInside(game.maze, row, col)) {
                continue;
            }
            if (game.maze.tiles[row][col].item != ITEM_NONE) {
                info.itemsNearby++;
            }
            if (game.maze.tiles[row][col].type == TILE_DOOR) {
                info.doorsNearby++;
            }
        }
    }
    return info;
}

int getScoreMultiplier(Difficulty difficulty) {
    return getDifficultySettings(difficulty).scoreMultiplier;
}

int computeScore(int mazeSize, Difficulty difficulty, int moves, int optimalMoves, int coins, int hintPenalty,
                 int bonusPoints) {
    int score = mazeSize * 40 * getScoreMultiplier(difficulty);
    if (moves > optimalMoves) {
        score -= (moves - optimalMoves) * extraMovePenalty;
    }
    score += coins * coinValue;
    score -= hintPenalty;
    score += bonusPoints;
    if (score < 0) {
        score = 0;
    }
    return score;
}

int calculateScore(const GameState& game) {
    return computeScore(game.maze.size, game.difficulty, game.player.moves, game.optimalMoves,
                        game.player.coins, game.hintPenalty, game.player.bonusPoints);
}

int calculateStars(int moves, int optimalMoves) {
    if (optimalMoves <= 0 || moves * 100 <= optimalMoves * 115) {
        return 3;            
    }
    if (moves * 100 <= optimalMoves * 160) {
        return 2;       
    }
    return 1;
}

int chaserStepForDifficulty(Difficulty difficulty) {
    if (difficulty == EASY) return 850;
    if (difficulty == NORMAL) return 750;
    if (difficulty == HARD) return 650;
    return 600;
}
