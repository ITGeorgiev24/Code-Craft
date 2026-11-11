#pragma once
#include "gameLogic.h"

#include <string>

using namespace std;

struct DrawOptions {
    bool showFullMap = false;
    string caption;   
};

string difficultyName(Difficulty difficulty);

string wallSymbol();
string doorSymbol();
string exitSymbol();
string playerSymbol();
string itemSymbol(ItemType item);
string trailSymbol();
string hintSymbol();
string fogSymbol();
string edgeSymbol();
string arrowSymbol();

string buildGameFrame(const GameState& game, const DrawOptions& options, const string& message,
                      int columns, int rows);

void drawGame(const GameState& game, const DrawOptions& options, const string& message);
