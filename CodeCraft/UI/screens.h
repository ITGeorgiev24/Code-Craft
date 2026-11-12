#pragma once

#include "../Logic/campaign.h"
#include "../Logic/gameLogic.h"

#include <string>

using namespace std;

enum VictoryChoice { VICTORY_NEW_MAZE, VICTORY_NEXT_LEVEL, VICTORY_REPLAY, VICTORY_LEVEL_SELECT, VICTORY_MAIN_MENU };

struct CampaignResult {
    int level = 1;
    bool newLevelUnlocked = false;
    bool campaignComplete = false;
    int totalStars = 0;
};

bool configureNewGame(int& size, Difficulty& difficulty);

void showHelpScreen();
void showLegendScreen();
void showLeaderboardScreen();
void showErrorScreen(const string& text);

VictoryChoice showVictoryScreen(const GameState& game, const CampaignResult* campaign, bool newMazeAllowed, const string& username);

string levelName(int level);

int selectLevel(const CampaignProgress& progress, int suggestedLevel);

string buildLocationMessage(const GameState& game);
