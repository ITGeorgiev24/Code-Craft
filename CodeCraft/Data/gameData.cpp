// DATA LAYER - gameData.cpp
// Creating and deleting the dynamic arrays.

#include "gameData.h"

using namespace std;

// ----- Maze -----------------------------------------------------------

void createMaze(Maze& maze, int size) {
    deleteMaze(maze);

    // First an array of row pointers, then every row.
    maze.tiles = new Tile*[size];
    for (int row = 0; row < size; row++) {
        maze.tiles[row] = new Tile[size];   // every Tile starts as a wall
    }
    maze.size = size;
}

void deleteMaze(Maze& maze) {
    if (maze.tiles != nullptr) {
        for (int row = 0; row < maze.size; row++) {
            delete[] maze.tiles[row];
        }
        delete[] maze.tiles;
    }
    maze.tiles = nullptr;
    maze.size = 0;
}

bool isInside(const Maze& maze, int row, int col) {
    return maze.tiles != nullptr && row >= 0 && col >= 0 && row < maze.size && col < maze.size;
}

// ----- Position list --------------------------------------------------

void createList(PositionList& list, int capacity) {
    deleteList(list);
    list.items = new Position[capacity];
    list.capacity = capacity;
    list.count = 0;
}

void deleteList(PositionList& list) {
    delete[] list.items;
    list.items = nullptr;
    list.count = 0;
    list.capacity = 0;
}

bool addToList(PositionList& list, Position position) {
    if (list.items == nullptr || list.count >= list.capacity) {
        return false;
    }
    list.items[list.count] = position;
    list.count++;
    return true;
}

bool samePosition(Position first, Position second) {
    return first.row == second.row && first.col == second.col;
}

// ----- Score table ----------------------------------------------------

void createScoreTable(ScoreTable& table, int capacity) {
    deleteScoreTable(table);
    table.entries = new ScoreEntry[capacity];
    table.capacity = capacity;
    table.count = 0;
}

void deleteScoreTable(ScoreTable& table) {
    delete[] table.entries;
    table.entries = nullptr;
    table.count = 0;
    table.capacity = 0;
}

bool addScore(ScoreTable& table, const ScoreEntry& entry) {
    if (table.entries == nullptr || table.count >= table.capacity) {
        return false;
    }
    table.entries[table.count] = entry;
    table.count++;
    return true;
}

// ----- Account list ---------------------------------------------------

void createAccountList(AccountList& list, int capacity) {
    deleteAccountList(list);
    list.accounts = new Account[capacity];
    list.capacity = capacity;
    list.count = 0;
}

void deleteAccountList(AccountList& list) {
    delete[] list.accounts;
    list.accounts = nullptr;
    list.count = 0;
    list.capacity = 0;
}

bool addAccount(AccountList& list, const Account& account) {
    if (list.accounts == nullptr || list.count >= list.capacity) {
        return false;
    }
    list.accounts[list.count] = account;
    list.count++;
    return true;
}
