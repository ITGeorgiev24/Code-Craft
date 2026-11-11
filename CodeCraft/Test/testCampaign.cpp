#include "CppUnitTest.h"
#include "../Logic/campaign.h"
#include "../Logic/gameLogic.h"
#include "../Logic/validation.h"

#include <cstdio>

using namespace std;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

TEST_CLASS(CampaignTests) {
public:
    TEST_METHOD(campaignHasTenLevels) {
        Assert::AreEqual(10, levelCount, L"10 levels");
        Assert::IsTrue(!isValidLevel(0) && isValidLevel(1) && isValidLevel(10) && !isValidLevel(11), L"valid numbers");
        for (int level = 1; level <= levelCount; level++) {
            Assert::IsTrue(getLevel(level).number == level && isValidMazeSize(getLevel(level).size), (L"level " + to_wstring(level)).c_str());
        }
        Assert::IsTrue(getLevel(1).size == 10 && getLevel(10).size == 50 && getLevel(10).difficulty == EXPERT, L"10x10 ... 50x50 Expert");
    }

    TEST_METHOD(levelsNeverGetEasier) {
        for (int level = 2; level <= levelCount; level++) {
            Assert::IsTrue(getLevel(level).size > getLevel(level - 1).size, L"bigger maze");
            Assert::IsTrue(getLevel(level).difficulty >= getLevel(level - 1).difficulty, L"not easier");
        }
    }

    TEST_METHOD(everyLevelCanBePlayed) {
        for (int level = 1; level <= levelCount; level++) {
            GameState game;
            Assert::IsTrue(startNewGame(game, getLevel(level).size, getLevel(level).difficulty), L"level created");
            Assert::IsTrue(getOptimalMoves(game) >= 2 * (getLevel(level).size - 1), L"the exit can be reached");
            endGame(game);
        }
    }

    TEST_METHOD(onlyLevelOneIsOpenAtTheStart) {
        CampaignProgress progress;
        Assert::IsTrue(isLevelUnlocked(progress, 1) && !isLevelUnlocked(progress, 2), L"only level 1");
        Assert::IsFalse(isLevelCompleted(progress, 1), L"nothing completed");
        Assert::IsTrue(getTotalStars(progress) == 0 && !isCampaignComplete(progress), L"0 stars");
    }

    TEST_METHOD(winningUnlocksTheNextLevel) {
        CampaignProgress progress;
        Assert::IsTrue(saveLevelResult(progress, 1, 2, 500), L"level 2 unlocked");
        Assert::IsTrue(isLevelUnlocked(progress, 2) && isLevelCompleted(progress, 1), L"level 1 completed");
        Assert::IsFalse(saveLevelResult(progress, 1, 1, 300), L"playing again unlocks nothing new");
        Assert::IsTrue(progress.bestStars[0] == 2 && progress.bestScores[0] == 500, L"the best result is kept");
        saveLevelResult(progress, 1, 3, 800);
        Assert::IsTrue(progress.bestStars[0] == 3 && progress.bestScores[0] == 800, L"a better result replaces it");
    }

    TEST_METHOD(lockedLevelsCannotBeSaved) {
        CampaignProgress progress;
        Assert::IsFalse(saveLevelResult(progress, 5, 3, 1000), L"level 5 is locked");
        Assert::AreEqual(0, progress.bestStars[4], L"nothing saved");
        Assert::IsTrue(!saveLevelResult(progress, 0, 3, 1000) && !saveLevelResult(progress, 11, 3, 1000), L"wrong numbers");
    }

    TEST_METHOD(allLevelsCompleteTheCampaign) {
        CampaignProgress progress;
        for (int level = 1; level <= levelCount; level++) {
            saveLevelResult(progress, level, 3, 100);
        }
        Assert::IsTrue(isCampaignComplete(progress), L"campaign complete");
        Assert::IsTrue(getTotalStars(progress) == 30 && progress.unlockedLevels == 10, L"30 stars");
    }

    TEST_METHOD(progressIsFixed) {
        CampaignProgress broken;
        broken.unlockedLevels = 99;
        broken.bestStars[0] = 7;
        broken.bestScores[1] = -50;
        fixProgress(broken);
        Assert::IsTrue(broken.unlockedLevels == 10 && broken.bestStars[0] == 3 && broken.bestScores[1] == 0, L"values fixed");

        CampaignProgress lowUnlock;
        lowUnlock.bestStars[3] = 2;
        fixProgress(lowUnlock);
        Assert::AreEqual(5, lowUnlock.unlockedLevels, L"level 5 opened");
    }
};
