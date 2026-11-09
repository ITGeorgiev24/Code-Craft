#pragma once
#include "gameData.h"
#include <string>

using namespace std;
void buildMaze(Maze& maze, const string rows[], int size);
void buildGame(GameState& game, const string rows[], int size);
Position makePosition(int row, int col);
extern const string sampleMaze[10];