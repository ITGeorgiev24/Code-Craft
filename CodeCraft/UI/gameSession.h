#pragma once
#include "../Logic/gameLogic.h"

#include <string>

using namespace std;

void playFreeGames(GameState& game, int size, Difficulty difficulty, bool warden, const string& caption,
                   const string& username);

void playCampaign(GameState& game, const string& username);
