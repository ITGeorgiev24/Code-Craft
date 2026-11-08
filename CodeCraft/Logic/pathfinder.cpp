#include "pathfinder.h"

using namespace std;

const int rowStep[4] = { -1, 1, 0, 0 };
const int colStep[4] = { 0, 0, -1, 1 };

bool isWalkable(const Maze& maze, int row, int col, bool doorsAllowed) {
    if (!isInside(maze, row, col)) {
        return false;
    }
    TileType type = maze.tiles[row][col].type;
    return type == TILE_PATH || type == TILE_EXIT || (doorsAllowed && type == TILE_DOOR);
}

bool findDistances(const Maze& maze, Position start, bool doorsAllowed, int* distances) {
    int size = maze.size;
    for (int i = 0; i < size * size; i++) {
        distances[i] = -1;
    }
    if (!isWalkable(maze, start.row, start.col, doorsAllowed)) {
        return false;
    }

    Position* queue = new Position[size * size];
    int head = 0;
    int tail = 0;

    distances[start.row * size + start.col] = 0;
    queue[tail] = start;
    tail++;

    while (head < tail) {
        Position current = queue[head];
        head++;
        int currentDistance = distances[current.row * size + current.col];

        for (int direction = 0; direction < 4; direction++) {
            int nextRow = current.row + rowStep[direction];
            int nextCol = current.col + colStep[direction];
            if (isWalkable(maze, nextRow, nextCol, doorsAllowed)
                && distances[nextRow * size + nextCol] == -1) {
                distances[nextRow * size + nextCol] = currentDistance + 1;
                queue[tail].row = nextRow;
                queue[tail].col = nextCol;
                tail++;
            }
        }
    }

    delete[] queue;
    return true;
}

int shortestPathLength(const Maze& maze, Position from, Position to, bool doorsAllowed) {
    if (!isInside(maze, to.row, to.col)) {
        return -1;
    }
    int* distances = new int[maze.size * maze.size];
    findDistances(maze, from, doorsAllowed, distances);
    int result = distances[to.row * maze.size + to.col];
    delete[] distances;
    return result;
}

bool findShortestPath(const Maze& maze, Position from, Position to, bool doorsAllowed, PositionList& path) {
    deleteList(path);
    if (!isInside(maze, from.row, from.col) || !isInside(maze, to.row, to.col)) {
        return false;
    }

    int size = maze.size;
    int* distances = new int[size * size];
    findDistances(maze, to, doorsAllowed, distances);

    int steps = distances[from.row * size + from.col];
    if (steps < 0) {
        delete[] distances;
        return false;
    }

    createList(path, steps + 1);
    Position current = from;
    addToList(path, current);
    for (int step = 0; step < steps; step++) {
        int currentDistance = distances[current.row * size + current.col];
        for (int direction = 0; direction < 4; direction++) {
            int nextRow = current.row + rowStep[direction];
            int nextCol = current.col + colStep[direction];
            if (isWalkable(maze, nextRow, nextCol, doorsAllowed)
                && distances[nextRow * size + nextCol] == currentDistance - 1) {
                current.row = nextRow;
                current.col = nextCol;
                break;
            }
        }
        addToList(path, current);
    }

    delete[] distances;
    return true;
}
