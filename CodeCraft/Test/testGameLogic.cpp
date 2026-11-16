#include "CppUnitTest.h"
#include "testHelpers.h"
#include "../Logic/gameLogic.h"

using namespace std;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

TEST_CLASS(GameLogicTests) {
public:
    TEST_METHOD(newGameStartsAtTopLeft) {
        GameState game;
        Assert::IsTrue(startNewGame(game, 20, NORMAL), L"game started");
        Assert::IsTrue(getStatus(game) == PLAYING, L"status PLAYING");
        Assert::IsTrue(getPlayer(game).position.row == 0 && getPlayer(game).position.col == 0, L"player at (0, 0)");
        Assert::AreEqual(0, getPlayer(game).moves, L"0 moves");
        Assert::AreEqual(20, getMazeSize(game), L"size 20");
        Assert::IsTrue(getOptimalMoves(game) >= 38, L"the best way is at least 38 moves");
        endGame(game);
        Assert::IsTrue(game.maze.tiles == nullptr, L"memory deleted");
    }

    TEST_METHOD(newGameRejectsWrongSize) {
        GameState game;
        Assert::IsFalse(startNewGame(game, 5, EASY), L"5 is too small");
        Assert::IsFalse(startNewGame(game, 60, EASY), L"60 is too big");
        Assert::IsTrue(getStatus(game) == NOT_STARTED, L"no game");
    }

    TEST_METHOD(wallsAndEdgesBlockMoves) {
        GameState game;
        buildGame(game, sampleMaze, 10);
        Assert::IsTrue(movePlayer(game, DIR_UP).status == MOVE_EDGE, L"up = edge");
        Assert::IsTrue(movePlayer(game, DIR_LEFT).status == MOVE_EDGE, L"left = edge");
        movePlayer(game, DIR_RIGHT);
        Assert::IsTrue(movePlayer(game, DIR_DOWN).status == MOVE_WALL, L"down = wall");
        Assert::AreEqual(1, getPlayer(game).moves, L"blocked moves are not counted");
        endGame(game);
    }

    TEST_METHOD(lockedDoorNeedsAKey) {
        GameState game;
        buildGame(game, sampleMaze, 10);
        Assert::IsTrue(movePlayer(game, DIR_DOWN).status == MOVE_LOCKED_DOOR, L"the door is locked");
        Assert::AreEqual(0, getPlayer(game).position.row, L"the player did not move");
        endGame(game);
    }

    TEST_METHOD(keyOpensTheDoorForever) {
        GameState game;
        buildGame(game, sampleMaze, 10);
        movePlayer(game, DIR_RIGHT);
        MoveResult pickUp = movePlayer(game, DIR_RIGHT);       
        Assert::IsTrue(pickUp.itemFound == ITEM_KEY, L"key found");
        Assert::AreEqual(1, getPlayer(game).keys, L"1 key");
        Assert::IsTrue(getItemAt(game, 0, 2) == ITEM_NONE, L"the key is removed from the maze");

        movePlayer(game, DIR_LEFT);
        movePlayer(game, DIR_LEFT);
        MoveResult door = movePlayer(game, DIR_DOWN);
        Assert::IsTrue(door.status == MOVE_OK && door.openedDoor, L"the door is opened");
        Assert::AreEqual(0, getPlayer(game).keys, L"the key is used");
        Assert::IsTrue(getTileType(game, 1, 0) == TILE_PATH, L"the door became a path");
        Assert::IsTrue(movePlayer(game, DIR_UP).status == MOVE_OK && movePlayer(game, DIR_DOWN).status == MOVE_OK, L"it stays open");
        endGame(game);
    }

    TEST_METHOD(coinsAndCompassAreCollected) {
        GameState game;
        buildGame(game, sampleMaze, 10);
        for (int i = 0; i < 6; i++) {
            movePlayer(game, DIR_RIGHT);
        }
        Assert::AreEqual(1, getPlayer(game).coins, L"1 coin");
        Assert::AreEqual(1, getPlayer(game).compasses, L"1 compass");
        Assert::AreEqual(1, getPlayer(game).keys, L"1 key");
        Assert::IsTrue(isVisited(game, 0, 3), L"visited squares are remembered");
        endGame(game);
    }

    TEST_METHOD(reachingTheExitWins) {
        GameState game;
        buildGame(game, sampleMaze, 10);
        MoveResult result;
        for (int i = 0; i < 9; i++) {
            result = movePlayer(game, DIR_RIGHT);
        }
        Assert::IsFalse(result.reachedExit, L"not yet");
        for (int i = 0; i < 9; i++) {
            result = movePlayer(game, DIR_DOWN);
        }
        Assert::IsTrue(result.reachedExit, L"exit reached");
        Assert::IsTrue(getStatus(game) == WON, L"status WON");
        Assert::AreEqual(18, getPlayer(game).moves, L"18 moves");
        Assert::IsTrue(movePlayer(game, DIR_LEFT).status == MOVE_NOT_PLAYING, L"no moves after winning");
        endGame(game);
    }

    TEST_METHOD(hintUsesCompassThenCostsPoints) {
        GameState game;
        buildGame(game, sampleMaze, 10);
        game.player.compasses = 1;
        Assert::IsTrue(requestHint(game) == HINT_FREE, L"first hint is free (compass)");
        Assert::AreEqual(0, getPlayer(game).compasses, L"the compass is used");
        Assert::IsTrue(isHintVisible(game) && isHintTile(game, 0, 1), L"the next step is shown");
        Assert::IsFalse(isHintTile(game, 0, 0), L"not on the player's square");
        Assert::IsTrue(requestHint(game) == HINT_PAID, L"second hint costs points");
        Assert::IsTrue(game.hintPenalty == paidHintCost && getHintsUsed(game) == 2, L"penalty and counter");
        endGame(game);
    }

    TEST_METHOD(hintFollowsThePlayer) {
        GameState game;
        buildGame(game, sampleMaze, 10);
        requestHint(game);
        movePlayer(game, DIR_RIGHT);      
        Assert::IsTrue(isHintVisible(game) && isHintTile(game, 0, 2), L"the hint moves with the player");
        movePlayer(game, DIR_LEFT);        
        Assert::IsFalse(isHintVisible(game), L"the hint disappears");
        endGame(game);
    }

    TEST_METHOD(restartGivesTheSameMaze) {
        GameState game;
        startGameWithSeed(game, 15, EASY, 31337);
        int best = getOptimalMoves(game);
        TileType square = getTileType(game, 0, 1);
        movePlayer(game, DIR_RIGHT);
        movePlayer(game, DIR_DOWN);
        Assert::IsTrue(restartGame(game), L"restarted");
        Assert::AreEqual(0, getPlayer(game).moves, L"0 moves again");
        Assert::IsTrue(getOptimalMoves(game) == best && getTileType(game, 0, 1) == square, L"the same maze");
        endGame(game);
    }

    TEST_METHOD(sameSeedGivesTheSameMaze) {
        GameState first;
        GameState second;
        startGameWithSeed(first, 25, NORMAL, 2026u);
        startNewGame(second, 15, NORMAL);       
        startGameWithSeed(second, 25, NORMAL, 2026u);
        bool same = getOptimalMoves(first) == getOptimalMoves(second);
        for (int row = 0; row < 25; row++) {
            for (int col = 0; col < 25; col++) {
                if (getTileType(first, row, col) != getTileType(second, row, col)) {
                    same = false;
                }
            }
        }
        Assert::IsTrue(same, L"the same seed builds the same maze (replay, restart)");
        endGame(first);
        endGame(second);
    }

    TEST_METHOD(quitStopsTheGame) {
        GameState game;
        startNewGame(game, 10, NORMAL);
        quitGame(game);
        Assert::IsTrue(getStatus(game) == QUIT, L"status QUIT");
        Assert::IsTrue(movePlayer(game, DIR_RIGHT).status == MOVE_NOT_PLAYING, L"no moves");
        endGame(game);
    }

    TEST_METHOD(locationInfo) {
        GameState game;
        buildGame(game, sampleMaze, 10);
        LocationInfo info = describeLocation(game);
        Assert::IsTrue(info.isStart, L"at the start");
        Assert::IsTrue(info.neighbors[DIR_UP] == NEAR_EDGE && info.neighbors[DIR_LEFT] == NEAR_EDGE, L"edges");
        Assert::IsTrue(info.neighbors[DIR_DOWN] == NEAR_DOOR, L"a door below");
        Assert::IsTrue(info.neighbors[DIR_RIGHT] == NEAR_OPEN, L"open to the right");
        Assert::AreEqual(18, info.stepsToExit, L"18 steps to the exit");
        Assert::IsTrue(info.itemsNearby == 1 && info.doorsNearby == 1, L"1 item (the key) and 1 door nearby");
        endGame(game);
    }

    TEST_METHOD(expertHidesTheMazeInFog) {
        GameState game;
        startNewGame(game, 30, EXPERT);
        Assert::IsTrue(hasFog(game), L"Expert has fog");
        Assert::IsTrue(isTileVisible(game, 0, 0) && isTileVisible(game, 2, 2), L"near the player is visible");
        Assert::IsFalse(isTileVisible(game, 15, 15), L"far away is hidden");
        Assert::IsTrue(isTileVisible(game, 29, 29), L"the exit is always visible");
        endGame(game);

        startNewGame(game, 30, HARD);
        Assert::IsTrue(!hasFog(game) && isTileVisible(game, 15, 15), L"no fog below Expert");
        endGame(game);
    }

    TEST_METHOD(fogIsRevealedAndRemembered) {
        GameState game;
        buildGame(game, sampleMaze, 10);
        game.fogOfWar = true;
        Assert::IsFalse(isTileVisible(game, 0, 5), L"hidden at the start");
        for (int i = 0; i < 4; i++) {
            movePlayer(game, DIR_RIGHT);        
        }
        Assert::IsTrue(isTileVisible(game, 0, 6), L"2 squares ahead is visible");
        Assert::IsFalse(isTileVisible(game, 0, 7), L"3 squares ahead is hidden");
        for (int i = 0; i < 4; i++) {
            movePlayer(game, DIR_LEFT);
        }
        Assert::IsTrue(isTileVisible(game, 0, 6), L"seen squares stay visible");
        endGame(game);
    }

    TEST_METHOD(scoreDependsOnMoves) {
        Assert::AreEqual(838, computeScore(10, NORMAL, 22, 18, 2, 0), L"838");
        Assert::AreEqual(800, computeScore(10, NORMAL, 18, 18, 0, 0), L"the shortest way gives the full score");
        Assert::IsTrue(computeScore(10, NORMAL, 30, 18, 0, 0) < computeScore(10, NORMAL, 20, 18, 0, 0), L"more moves = fewer points");
        Assert::AreEqual(400, computeScore(10, EASY, 10, 18, 0, 0), L"fewer moves than the best (doors) are not punished");
        Assert::AreEqual(375, computeScore(10, EASY, 18, 18, 0, 25), L"a paid hint costs 25");
        Assert::AreEqual(0, computeScore(10, EASY, 5000, 18, 0, 500), L"never below 0");
        Assert::IsTrue(computeScore(30, EXPERT, 100, 100, 0, 0) > computeScore(30, HARD, 100, 100, 0, 0), L"harder = more points");
    }

    TEST_METHOD(starsDependOnMoves) {
        Assert::AreEqual(3, calculateStars(100, 100), L"the shortest way");
        Assert::AreEqual(3, calculateStars(115, 100), L"+15%");
        Assert::AreEqual(2, calculateStars(150, 100), L"+50%");
        Assert::AreEqual(1, calculateStars(300, 100), L"+200%");
    }

    TEST_METHOD(bonusPointsAreAddedToTheScore) {
        Assert::AreEqual(950, computeScore(10, NORMAL, 18, 18, 0, 0, 150), L"800 + 150 bonus");
        Assert::AreEqual(800, computeScore(10, NORMAL, 18, 18, 0, 0), L"no bonus by default");
    }

    TEST_METHOD(coinsGiveTreasureRewards) {
        const string coinMaze[10] = {
            ".$$$$$$$..",
            "#########.",
            "#########.",
            "#########.",
            "#########.",
            "#########.",
            "#########.",
            "#########.",
            "#########.",
            "#########E",
        };
        GameState game;
        buildGame(game, coinMaze, 10);
        Assert::AreEqual(7, getTotalCoins(game), L"7 coins in the maze");

        MoveResult result;
        result = movePlayer(game, DIR_RIGHT);
        Assert::IsTrue(result.reward == REWARD_NONE, L"1 coin - no treasure yet");
        movePlayer(game, DIR_RIGHT);
        result = movePlayer(game, DIR_RIGHT);
        Assert::IsTrue(result.reward == REWARD_COMPASS, L"3 coins - the first treasure is a compass");
        Assert::AreEqual(1, getPlayer(game).compasses, L"the compass is in the pocket");

        movePlayer(game, DIR_RIGHT);
        movePlayer(game, DIR_RIGHT);
        result = movePlayer(game, DIR_RIGHT);
        Assert::IsTrue(result.reward == REWARD_KEY, L"6 coins - the second treasure is a key");
        Assert::AreEqual(1, getPlayer(game).keys, L"the key is in the pocket");
        Assert::IsFalse(result.allCoins, L"one coin is still left");

        result = movePlayer(game, DIR_RIGHT);
        Assert::IsTrue(result.allCoins, L"the last coin");
        Assert::AreEqual(allCoinsBonus, getPlayer(game).bonusPoints, L"bonus for all coins");
        Assert::AreEqual(2, getPlayer(game).rewards, L"2 treasures");
        endGame(game);
    }

    TEST_METHOD(clockStartsWithTheFirstMove) {
        GameState game;
        buildGame(game, sampleMaze, 10);
        advanceTime(game, 5000);
        Assert::AreEqual(0, getElapsedSeconds(game), L"no move - no time");
        movePlayer(game, DIR_RIGHT);
        advanceTime(game, 2500);
        Assert::AreEqual(2, getElapsedSeconds(game), L"2.5 seconds after the first move");
        quitGame(game);
        advanceTime(game, 5000);
        Assert::AreEqual(2, getElapsedSeconds(game), L"the clock stops when the game ends");
        endGame(game);
    }

    TEST_METHOD(wardenHuntsThePlayer) {
        GameState game;
        buildGame(game, sampleMaze, 10);
        enableChaser(game, 1000, 200);
        Assert::IsTrue(hasChaser(game) && !isChaserActive(game), L"the Warden sleeps");

        for (int i = 0; i < 6; i++) {
            movePlayer(game, DIR_RIGHT);             
        }
        Assert::IsTrue(advanceTime(game, 1000) == TIME_CHASER_APPEARED, L"the Warden appears");
        Assert::IsTrue(isChaserAt(game, 0, 0), L"at the start");

        Assert::IsTrue(advanceTime(game, 200) == TIME_CHASER_MOVED, L"one step");
        Assert::IsTrue(isChaserAt(game, 0, 1), L"towards the player");
        Assert::AreEqual(5, getChaserDistance(game), L"5 steps away");

        Assert::IsTrue(advanceTime(game, 1000) == TIME_CAUGHT, L"5 more steps - caught");
        Assert::IsTrue(getStatus(game) == CAUGHT, L"status CAUGHT");
        endGame(game);
    }

    TEST_METHOD(frozenWardenWaits) {
        GameState game;
        buildGame(game, sampleMaze, 10);
        enableChaser(game, 0, 200);
        for (int i = 0; i < 8; i++) {
            movePlayer(game, DIR_RIGHT);
        }
        advanceTime(game, 10);     
        Assert::IsTrue(isChaserActive(game), L"the Warden is here");
        game.chaser.frozenMs = 1000;
        Assert::IsTrue(isChaserFrozen(game), L"frozen");
        advanceTime(game, 900);
        Assert::IsTrue(isChaserAt(game, 0, 0), L"it does not move while frozen");
        advanceTime(game, 300);            
        Assert::IsTrue(isChaserAt(game, 0, 1), L"it moves again");
        endGame(game);
    }

    TEST_METHOD(restartKeepsTheWarden) {
        GameState game;
        startNewGame(game, 15, EASY);
        enableChaser(game, 3000, 500);
        movePlayer(game, DIR_RIGHT);
        Assert::IsTrue(restartGame(game), L"restarted");
        Assert::IsTrue(hasChaser(game) && !isChaserActive(game), L"the Warden sleeps again");
        Assert::AreEqual(0, getElapsedSeconds(game), L"the clock is back to 0");
        endGame(game);

        startNewGame(game, 15, EASY);
        Assert::IsFalse(hasChaser(game), L"a new game has no Warden");
        endGame(game);
    }

    TEST_METHOD(wardenInFreeGamesIsFasterOnHarderLevels) {
        Assert::IsTrue(chaserStepForDifficulty(EASY) > chaserStepForDifficulty(NORMAL), L"Normal is faster than Easy");
        Assert::IsTrue(chaserStepForDifficulty(NORMAL) > chaserStepForDifficulty(HARD), L"Hard is faster than Normal");
        Assert::IsTrue(chaserStepForDifficulty(HARD) > chaserStepForDifficulty(EXPERT), L"Expert is the fastest");
        GameState game;
        startNewGame(game, 20, HARD);
        enableChaser(game, freeGameChaserWakeMs, chaserStepForDifficulty(HARD));
        Assert::IsTrue(hasChaser(game) && getChaserWakeSeconds(game) == 10, L"wakes up 10 s after the first move");
        endGame(game);
    }
};
