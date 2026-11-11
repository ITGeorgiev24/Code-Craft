#pragma once
#include "gameData.h"

#include <string>

using namespace std;

bool loadScores(const string& fileName, ScoreTable& table);

bool saveScores(const string& fileName, const ScoreTable& table);

bool loadAccounts(const string& fileName, AccountList& list);

bool saveAccounts(const string& fileName, const AccountList& list);