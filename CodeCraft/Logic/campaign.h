#pragma once
#include "../Data/gameData.h"

#include <string>

using namespace std;

const int levelCount = maxLevels;

struct LevelInfo {
    int number;
    int size;
    Difficulty difficulty;
    int chaserWakeMs;             
    int chaserStepMs;               
};

bool isValidLevel(int level);

LevelInfo getLevel(int level);

bool isLevelUnlocked(const CampaignProgress& progress, int level);
bool isLevelCompleted(const CampaignProgress& progress, int level);

int getTotalStars(const CampaignProgress& progress);

bool isCampaignComplete(const CampaignProgress& progress);

bool levelHasChaser(int level);

bool saveLevelResult(CampaignProgress& progress, int level, int stars, int score, int seconds = 0);

void fixProgress(CampaignProgress& progress);

