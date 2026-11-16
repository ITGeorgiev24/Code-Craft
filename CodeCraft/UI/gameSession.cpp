#include "gameSession.h"
#include "../Logic/accounts.h"
#include "../Logic/campaign.h"
#include "console.h"
#include "input.h"
#include "renderer.h"
#include "screens.h"
#include "story.h"

using namespace std;

string rewardMessage(RewardType reward) {
    string text = "TREASURE for " + to_string(coinsPerReward) + " coins: ";
    if (reward == REWARD_COMPASS) {
        text += "a magic compass - your next hint is free!";
    } else if (reward == REWARD_KEY) {
        text += "a golden key - it opens a secret door!";
    } else {
        text += "a bag of gold - +" + to_string(rewardPoints) + " points!";
    }
    return paint(text, bold + yellow);
}

string moveMessage(const MoveResult& result, const GameState& game) {
    Player player = getPlayer(game);
    if (result.status == MOVE_WALL) {
        return paint("Bump! That is a wall.", muted);
    }
    if (result.status == MOVE_LOCKED_DOOR) {
        return paint("A locked secret door. Find a key (k) to open this shortcut.", magenta);
    }
    if (result.status == MOVE_EDGE) {
        return paint("This is the edge of the maze - the exit is in the bottom-right corner.", muted);
    }
    if (result.caught) {
        return paint("You ran straight into the Shadow Warden!", bold + red);
    }

    string message;
    if (result.openedDoor) {
        message += paint("Click! The secret door is open. Keys left: " + to_string(player.keys) + ". ", magenta);
    }
    if (result.itemFound == ITEM_KEY) {
        message += paint("You found a key! It opens one secret door.", yellow);
    } else if (result.itemFound == ITEM_COIN) {
        message += paint("+" + to_string(coinValue) + " points! Coins: " + to_string(player.coins) + "/"
                         + to_string(getTotalCoins(game)), green);
        int left = coinsPerReward - player.coins % coinsPerReward;
        if (result.reward == REWARD_NONE && player.coins < getTotalCoins(game)) {
            message += paint("  (treasure in " + to_string(left) + " more)", muted);
        }
    } else if (result.itemFound == ITEM_COMPASS) {
        message += paint("A compass! Your next hint (H) is free.", cyan);
    }

    string second;
    if (result.reward != REWARD_NONE) {
        second = rewardMessage(result.reward);
        if (isChaserActive(game)) {
            second += paint("  The Warden is frozen!", bold + blue);
        }
    }
    if (result.allCoins) {
        second += (second == "" ? "" : "  ") + paint("ALL coins found! +" + to_string(allCoinsBonus) + " points!",
                                                     bold + green);
    }
    return second == "" ? message : message + "\n" + second;
}

string hintMessage(HintResult result) {
    if (result == HINT_FREE) {
        return paint("The compass shows the way (free hint). Follow the stars!", cyan);
    }
    if (result == HINT_PAID) {
        return paint("Hint shown. Follow the stars! (-" + to_string(paidHintCost) + " points)", cyan);
    }
    if (result == HINT_NO_PATH) {
        return paint("No route found from here.", red);
    }
    return "";
}

bool confirm(const GameState& game, const DrawOptions& options, const string& question) {
    bool yes = false;
    clearKeyBuffer();
    while (true) {
        string choice = yes ? paint("[ Yes ]", bold + yellow) + "    No  "
                            : "  Yes    " + paint("[ No ]", bold + yellow);
        drawGame(game, options, paint(question, yellow) + "   " + choice + "\n"
                                + paint("Left/Right - choose   Enter - confirm   Esc - cancel", muted));
        int key = readKey();
        if (key == arrowLeft || key == arrowRight || key == arrowUp || key == arrowDown) {
            yes = !yes;
        } else if (key == enterKey) {
            return yes;
        } else if (key == escapeKey) {
            return false;
        }
    }
}

void pauseAndWaitForKey() {
    readKeyTimeout(700);
    clearKeyBuffer();
    readKey();
}

GameStatus playMaze(GameState& game, const DrawOptions& options) {
    string message;
    if (hasChaser(game)) {
        message = paint("The Shadow Warden sleeps... it wakes up soon after your first step. RUN!", bold + red);
    } else if (hasFog(game)) {
        message = paint("Darkness! Explore carefully - the exit is in the bottom-right corner.", muted);
    } else {
        message = paint("Reach the exit in the bottom-right corner. Press ? for help.", muted);
    }

    showCursor(false);
    clearScreen();
    clearKeyBuffer();

    long long lastTime = currentTimeMs();
    bool redraw = true;
    int shownSecond = -1;
    int shownWake = -1;

    while (getStatus(game) == PLAYING) {
        if (redraw) {
            drawGame(game, options, message);
            redraw = false;
            shownSecond = getElapsedSeconds(game);
            shownWake = getChaserWakeSeconds(game);
        }

        int key = readKeyTimeout(hasChaser(game) ? 40 : 100);
        long long now = currentTimeMs();
        TimeEvent event = advanceTime(game, (int)(now - lastTime));
        lastTime = now;

        if (event == TIME_CAUGHT) {
            break;
        }
        if (event == TIME_CHASER_APPEARED) {
            message = paint("A cold wind... The Shadow Warden has entered the maze! It follows you - RUN!", bold + red);
            redraw = true;
        } else if (event == TIME_CHASER_MOVED) {
            redraw = true;
        }

        if (key == noKey) {
            if (!redraw && (getElapsedSeconds(game) != shownSecond || getChaserWakeSeconds(game) != shownWake)) {
                drawStatusLine(game);
                shownSecond = getElapsedSeconds(game);
                shownWake = getChaserWakeSeconds(game);
            }
            continue;
        }

        redraw = true;
        if (key == arrowUp || key == 'w') {
            message = moveMessage(movePlayer(game, DIR_UP), game);
        } else if (key == arrowDown || key == 's') {
            message = moveMessage(movePlayer(game, DIR_DOWN), game);
        } else if (key == arrowLeft || key == 'a') {
            message = moveMessage(movePlayer(game, DIR_LEFT), game);
        } else if (key == arrowRight || key == 'd') {
            message = moveMessage(movePlayer(game, DIR_RIGHT), game);
        } else if (key == 'h') {
            message = hintMessage(requestHint(game));
        } else if (key == 'i') {
            message = buildLocationMessage(game);
        } else if (key == 'l') {
            showLegendScreen();
            clearScreen();
            lastTime = currentTimeMs();           
        } else if (key == '?') {
            showHelpScreen();
            clearScreen();
            lastTime = currentTimeMs();
        } else if (key == 'r') {
            if (confirm(game, options, "Restart this maze from the beginning?")) {
                restartGame(game);
                message = paint("Same maze, fresh start. Good luck!", muted);
            } else {
                message = "";
            }
            lastTime = currentTimeMs();
        } else if (key == 'q' || key == escapeKey) {
            if (confirm(game, options, "Give up and return to the menu?")) {
                quitGame(game);
                return QUIT;
            }
            message = "";
            lastTime = currentTimeMs();
        }
    }

    if (getStatus(game) == CAUGHT) {
        drawGame(game, options, paint("The Shadow Warden caught you!", bold + red) + "\n"
                                + paint("Press any key...", muted));
        pauseAndWaitForKey();
        return CAUGHT;
    }

    drawGame(game, options, paint("You found the exit in " + formatTime(getElapsedSeconds(game)) + "! Press any key...",
                                  bold + green));
    pauseAndWaitForKey();
    return WON;
}

void startFreeMaze(GameState& game, int size, Difficulty difficulty, bool warden) {
    startNewGame(game, size, difficulty);
    if (warden) {
        enableChaser(game, freeGameChaserWakeMs, chaserStepForDifficulty(difficulty));
    }
}

void playFreeGames(GameState& game, int size, Difficulty difficulty, bool warden, const string& caption,
                   const string& username) {
    DrawOptions options;
    options.caption = caption;
    startFreeMaze(game, size, difficulty, warden);
    while (true) {
        GameStatus outcome = playMaze(game, options);
        if (outcome == QUIT) {
            endGame(game);
            return;
        }
        VictoryChoice choice = outcome == CAUGHT ? showCaughtScreen(game, 0) : showVictoryScreen(game, nullptr, username);
        if (choice == VICTORY_NEW_MAZE) {
            startFreeMaze(game, size, difficulty, warden);
        } else if (choice == VICTORY_REPLAY) {
            restartGame(game);
        } else {
            endGame(game);
            return;
        }
    }
}

void playCampaign(GameState& game, const string& username) {
    CampaignProgress progress = loadPlayerProgress(username);
    int level = selectLevel(progress, progress.unlockedLevels);
    bool showIntro = true;

    while (level != 0) {
        if (showIntro && !showChapterIntro(level)) {
            level = selectLevel(progress, level);        
            continue;
        }
        showIntro = true;

        LevelInfo info = getLevel(level);
        startNewGame(game, info.size, info.difficulty);         
        if (info.chaserStepMs > 0) {
            enableChaser(game, info.chaserWakeMs, info.chaserStepMs);
        }

        DrawOptions options;
        options.caption = "Chapter " + to_string(level) + ": " + levelName(level);
        options.level = level;
        GameStatus outcome = playMaze(game, options);

        if (outcome == QUIT) {
            endGame(game);
            level = selectLevel(progress, level);           
            continue;
        }
        if (outcome == CAUGHT) {
            VictoryChoice choice = showCaughtScreen(game, level);
            endGame(game);
            if (choice == VICTORY_NEW_MAZE) {
                showIntro = false;        
            } else if (choice == VICTORY_LEVEL_SELECT) {
                level = selectLevel(progress, level);
            } else {
                return;
            }
            continue;
        }

        int stars = calculateStars(getPlayer(game).moves, getOptimalMoves(game));
        int seconds = getElapsedSeconds(game) > 0 ? getElapsedSeconds(game) : 1;
        int oldBestTime = progress.bestTimes[level - 1];
        CampaignResult result;
        result.level = level;
        result.newLevelUnlocked = saveLevelResult(progress, level, stars, calculateScore(game), seconds);
        result.newBestTime = oldBestTime > 0 && seconds < oldBestTime;
        result.campaignComplete = isCampaignComplete(progress);
        result.totalStars = getTotalStars(progress);
        savePlayerProgress(username, progress);

        VictoryChoice choice = showVictoryScreen(game, &result, username);
        endGame(game);
        if (choice == VICTORY_NEXT_LEVEL) {
            level++;
        } else if (choice == VICTORY_NEW_MAZE) {
            showIntro = false;           
        } else if (choice == VICTORY_LEVEL_SELECT) {
            level = selectLevel(progress, level);
        } else {
            return;
        }
    }
}
