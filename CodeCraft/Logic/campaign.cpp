#include "campaign.h"

using namespace std;

const LevelInfo levelList[levelCount] = {
    { 1, 10, EASY },
    { 2, 14, EASY },
    { 3, 18, NORMAL },
    { 4, 22, NORMAL },
    { 5, 26, NORMAL },
    { 6, 30, HARD },
    { 7, 35, HARD },
    { 8, 40, EXPERT },
    { 9, 45, EXPERT },
    { 10, 50, EXPERT },
};

bool isValidLevel(int level) {
    return level >= 1 && level <= levelCount;
}

LevelInfo getLevel(int level) {
    if (!isValidLevel(level)) {
        return levelList[0];
    }
    return levelList[level - 1];
}

bool isLevelUnlocked(const CampaignProgress& progress, int level) {
    return isValidLevel(level) && level <= progress.unlockedLevels;
}

bool isLevelCompleted(const CampaignProgress& progress, int level) {
    return isValidLevel(level) && progress.bestStars[level - 1] > 0;
}

int getTotalStars(const CampaignProgress& progress) {
    int total = 0;
    for (int i = 0; i < levelCount; i++) {
        total += progress.bestStars[i];
    }
    return total;
}

bool isCampaignComplete(const CampaignProgress& progress) {
    for (int level = 1; level <= levelCount; level++) {
        if (!isLevelCompleted(progress, level)) {
            return false;
        }
    }
    return true;
}

bool saveLevelResult(CampaignProgress& progress, int level, int stars, int score) {
    if (!isLevelUnlocked(progress, level)) {
        return false;
    }
    if (stars < 1) {
        stars = 1;
    }
    if (stars > 3) {
        stars = 3;
    }
    if (stars > progress.bestStars[level - 1]) {
        progress.bestStars[level - 1] = stars;
    }
    if (score > progress.bestScores[level - 1]) {
        progress.bestScores[level - 1] = score;
    }

    if (level < levelCount && progress.unlockedLevels < level + 1) {
        progress.unlockedLevels = level + 1;
        return true;
    }
    return false;
}

void fixProgress(CampaignProgress& progress) {
    if (progress.unlockedLevels < 1) {
        progress.unlockedLevels = 1;
    }
    if (progress.unlockedLevels > levelCount) {
        progress.unlockedLevels = levelCount;
    }
    for (int i = 0; i < levelCount; i++) {
        if (progress.bestStars[i] < 0) {
            progress.bestStars[i] = 0;
        }
        if (progress.bestStars[i] > 3) {
            progress.bestStars[i] = 3;
        }
        if (progress.bestScores[i] < 0) {
            progress.bestScores[i] = 0;
        }
        if (progress.bestStars[i] > 0 && i + 2 <= levelCount && progress.unlockedLevels < i + 2) {
            progress.unlockedLevels = i + 2;
        }
    }
}
