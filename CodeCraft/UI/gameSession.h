#pragma once
#include "../Logic/gameLogic.h"

#include <string>

using namespace std;

void playFreeGames(GameState& game, int size, Difficulty difficulty, const string& caption, const string& username);

void playDailyChallenge(GameState& game, const string& username);

void playCampaign(GameState& game, const string& username);
