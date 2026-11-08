#pragma once
#include "gameData.h"

using namespace std;

bool isWalkable(const Maze& maze, int row, int col, bool doorsAllowed);

bool findDistances(const Maze& maze, Position start, bool doorsAllowed, int* distances);

int shortestPathLength(const Maze& maze, Position from, Position to, bool doorsAllowed);

bool findShortestPath(const Maze& maze, Position from, Position to, bool doorsAllowed, PositionList& path);