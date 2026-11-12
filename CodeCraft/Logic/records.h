#pragma once
#include "../Data/gameData.h"

#include <string>

using namespace std;

const int leaderboardSize = 10;
const string scoresFile = "mazeboundScores.txt";

bool isBetterScore(const ScoreEntry& first, const ScoreEntry& second);

int insertScore(ScoreTable& table, const ScoreEntry& entry);

void loadLeaderboard(ScoreTable& table, const string& fileName = scoresFile);

void deleteLeaderboard(ScoreTable& table);

ScoreEntry makeScoreEntry(const string& playerName, const GameState& game);

bool qualifiesForLeaderboard(int score, int moves, const string& fileName = scoresFile);

bool submitScore(const string& playerName, const GameState& game, int& place,
    const string& fileName = scoresFile);