#include "mazeGenerator.h"
#include "pathfinder.h"
#include "validation.h"

#include <cstdlib>

using namespace std;

int randomNumber(int minValue, int maxValue) {
    return minValue + rand() % (maxValue - minValue + 1);
}

bool isFloor(const Maze& maze, int row, int col) {
    return isInside(maze, row, col)
        && (maze.tiles[row][col].type == TILE_PATH || maze.tiles[row][col].type == TILE_EXIT);
}

bool isSolid(const Maze& maze, int row, int col) {
    return !isInside(maze, row, col) || maze.tiles[row][col].type == TILE_WALL;
}

bool isSeparatingWall(const Maze& maze, int row, int col, Position& first, Position& second) {
    if (!isInside(maze, row, col) || maze.tiles[row][col].type != TILE_WALL) {
        return false;
    }
    if (isFloor(maze, row, col - 1) && isFloor(maze, row, col + 1)
        && isSolid(maze, row - 1, col) && isSolid(maze, row + 1, col)) {
        first.row = row;      first.col = col - 1;
        second.row = row;     second.col = col + 1;
        return true;
    }
    if (isFloor(maze, row - 1, col) && isFloor(maze, row + 1, col)
        && isSolid(maze, row, col - 1) && isSolid(maze, row, col + 1)) {
        first.row = row - 1;  first.col = col;
        second.row = row + 1; second.col = col;
        return true;
    }
    return false;
}

void shufflePositions(Position* items, int count) {
    for (int i = count - 1; i > 0; i--) {
        int j = randomNumber(0, i);
        Position temp = items[i];
        items[i] = items[j];
        items[j] = temp;
    }
}

void carvePerfectMaze(Maze& maze, int limit) {
    const int rowStep[4] = { -2, 2, 0, 0 };
    const int colStep[4] = { 0, 0, -2, 2 };

    PositionList stack;
    createList(stack, limit * limit);

    Position start;
    maze.tiles[0][0].type = TILE_PATH;
    addToList(stack, start);

    while (stack.count > 0) {
        Position current = stack.items[stack.count - 1];

        Position options[4];
        int optionCount = 0;
        for (int direction = 0; direction < 4; direction++) {
            int row = current.row + rowStep[direction];
            int col = current.col + colStep[direction];
            if (row >= 0 && col >= 0 && row < limit && col < limit && maze.tiles[row][col].type == TILE_WALL) {
                options[optionCount].row = row;
                options[optionCount].col = col;
                optionCount++;
            }
        }

        if (optionCount == 0) {
            stack.count--;        
            continue;
        }

        Position next = options[randomNumber(0, optionCount - 1)];
        maze.tiles[(current.row + next.row) / 2][(current.col + next.col) / 2].type = TILE_PATH;     
        maze.tiles[next.row][next.col].type = TILE_PATH;
        addToList(stack, next);
    }

    deleteList(stack);
}

void fillLastRowAndColumn(Maze& maze) {
    int last = maze.size - 1;
    for (int i = 0; i < last; i += 2) {
        if (randomNumber(0, 2) == 0) {
            maze.tiles[last][i].type = TILE_PATH;
        }
        if (randomNumber(0, 2) == 0) {
            maze.tiles[i][last].type = TILE_PATH;
        }
    }
    for (int i = 1; i < last; i += 2) {
        if (maze.tiles[last][i - 1].type == TILE_PATH && maze.tiles[last][i + 1].type == TILE_PATH
            && randomNumber(0, 1) == 0) {
            maze.tiles[last][i].type = TILE_PATH;
        }
        if (maze.tiles[i - 1][last].type == TILE_PATH && maze.tiles[i + 1][last].type == TILE_PATH
            && randomNumber(0, 1) == 0) {
            maze.tiles[i][last].type = TILE_PATH;
        }
    }
    maze.tiles[last][last - 1].type = TILE_PATH;
}

int addLoops(Maze& maze, int chance) {
    int loops = 0;
    Position first;
    Position second;
    for (int row = 0; row < maze.size; row++) {
        for (int col = 0; col < maze.size; col++) {
            if (chance > 0 && isSeparatingWall(maze, row, col, first, second) && randomNumber(1, 100) <= chance) {
                maze.tiles[row][col].type = TILE_PATH;
                loops++;
            }
        }
    }
    return loops;
}

bool hasDoorNearby(const Maze& maze, int centerRow, int centerCol) {
    for (int row = centerRow - 2; row <= centerRow + 2; row++) {
        for (int col = centerCol - 2; col <= centerCol + 2; col++) {
            if (isInside(maze, row, col) && maze.tiles[row][col].type == TILE_DOOR) {
                return true;
            }
        }
    }
    return false;
}

int placeDoors(Maze& maze, int doorCount) {
    int size = maze.size;
    Position exitPosition;
    exitPosition.row = size - 1;
    exitPosition.col = size - 1;

    int* distances = new int[size * size];
    findDistances(maze, exitPosition, false, distances);

    Position* candidates = new Position[size * size];
    int* savings = new int[size * size];
    int candidateCount = 0;
    int minimumSaving = size / 2 > 6 ? size / 2 : 6;

    Position first;
    Position second;
    for (int row = 0; row < size; row++) {
        for (int col = 0; col < size; col++) {
            if (!isSeparatingWall(maze, row, col, first, second)) {
                continue;
            }
            int firstDistance = distances[first.row * size + first.col];
            int secondDistance = distances[second.row * size + second.col];
            if (firstDistance < 0 || secondDistance < 0) {
                continue;
            }
            int difference = firstDistance - secondDistance;
            if (difference < 0) {
                difference = -difference;
            }
            int saving = difference - 2;
            if (saving >= minimumSaving) {
                candidates[candidateCount].row = row;
                candidates[candidateCount].col = col;
                savings[candidateCount] = saving;
                candidateCount++;
            }
        }
    }

    for (int i = 0; i < candidateCount - 1; i++) {
        int best = i;
        for (int j = i + 1; j < candidateCount; j++) {
            if (savings[j] > savings[best]) {
                best = j;
            }
        }
        Position tempPosition = candidates[i];
        candidates[i] = candidates[best];
        candidates[best] = tempPosition;
        int tempSaving = savings[i];
        savings[i] = savings[best];
        savings[best] = tempSaving;
    }

    int pool = candidateCount < doorCount * 4 ? candidateCount : doorCount * 4;
    shufflePositions(candidates, pool);

    int placed = 0;
    for (int i = 0; i < pool && placed < doorCount; i++) {
        if (!hasDoorNearby(maze, candidates[i].row, candidates[i].col)) {
            maze.tiles[candidates[i].row][candidates[i].col].type = TILE_DOOR;
            placed++;
        }
    }

    delete[] distances;
    delete[] candidates;
    delete[] savings;
    return placed;
}

void placeItems(Maze& maze, const DifficultySettings& settings, GenerationReport& report) {
    int size = maze.size;

    Position* freeSquares = new Position[size * size];
    int freeCount = 0;
    for (int row = 0; row < size; row++) {
        for (int col = 0; col < size; col++) {
            if (maze.tiles[row][col].type == TILE_PATH && !(row == 0 && col == 0)) {
                freeSquares[freeCount].row = row;
                freeSquares[freeCount].col = col;
                freeCount++;
            }
        }
    }
    shufflePositions(freeSquares, freeCount);

    int keys = 0;
    if (report.doors > 0) {
        keys = report.doors + settings.keyBonus;
        if (keys < 1) {
            keys = 1;
        }
    }
    int coins = size * size / settings.tilesPerCoin;

    int next = 0;
    for (int i = 0; i < keys && next < freeCount; i++, next++) {
        maze.tiles[freeSquares[next].row][freeSquares[next].col].item = ITEM_KEY;
        report.keys++;
    }
    for (int i = 0; i < settings.compasses && next < freeCount; i++, next++) {
        maze.tiles[freeSquares[next].row][freeSquares[next].col].item = ITEM_COMPASS;
        report.compasses++;
    }
    for (int i = 0; i < coins && next < freeCount; i++, next++) {
        maze.tiles[freeSquares[next].row][freeSquares[next].col].item = ITEM_COIN;
        report.coins++;
    }

    delete[] freeSquares;
}

DifficultySettings getDifficultySettings(Difficulty difficulty) {
    DifficultySettings settings;
    if (difficulty == EASY) {
        settings = { 10, 1, 1, 30, 2, 1, false };
    } else if (difficulty == HARD) {
        settings = { 0, 2, -1, 50, 0, 3, false };
    } else if (difficulty == EXPERT) {
        settings = { 0, 3, -1, 60, 1, 4, true };
    } else {    
        settings = { 4, 2, 0, 40, 1, 2, false };
    }
    return settings;
}

bool generateMaze(Maze& maze, int size, Difficulty difficulty, unsigned int seed, GenerationReport& report) {
    report = GenerationReport();
    if (!isValidMazeSize(size)) {
        return false;
    }

    srand(seed);               
    createMaze(maze, size);
    DifficultySettings settings = getDifficultySettings(difficulty);

    if (size % 2 == 1) {
        carvePerfectMaze(maze, size);
    } else {
        carvePerfectMaze(maze, size - 1);
        fillLastRowAndColumn(maze);
    }
    maze.tiles[size - 1][size - 1].type = TILE_EXIT;

    report.loops = addLoops(maze, settings.loopChance);
    report.doors = placeDoors(maze, settings.baseDoors + size / 15);
    placeItems(maze, settings, report);

    maze.tiles[0][0].visited = true;
    return true;
}
