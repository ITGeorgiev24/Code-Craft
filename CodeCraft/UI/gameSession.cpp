#include "gameSession.h"
#include "../Logic/accounts.h"
#include "../Logic/campaign.h"
#include "console.h"
#include "input.h"
#include "renderer.h"
#include "screens.h"

using namespace std;

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

    string message;
    if (result.openedDoor) {
        message += paint("Click! The secret door is open. Keys left: " + to_string(player.keys) + ". ", magenta);
    }
    if (result.itemFound == ITEM_KEY) {
        message += paint("You found a key! It opens one secret door.", yellow);
    } else if (result.itemFound == ITEM_COIN) {
        message += paint("+" + to_string(coinValue) + " points! Coins: " + to_string(player.coins) + "/"
                         + to_string(getTotalCoins(game)), green);
    } else if (result.itemFound == ITEM_COMPASS) {
        message += paint("A compass! Your next hint (H) is free.", cyan);
    }
    return message;
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
    drawGame(game, options, paint(question + " (Y/N)", yellow));
    return readKey() == 'y';
}

bool playMaze(GameState& game, const string& caption) {
    DrawOptions options;
    options.caption = caption;
    string message = hasFog(game) ? "Fog of war! Explore carefully - the exit is in the bottom-right corner."
                                  : "Reach the exit in the bottom-right corner. Press ? for help.";
    message = paint(message, muted);

    showCursor(false);
    clearScreen();

    while (getStatus(game) == PLAYING) {
        drawGame(game, options, message);
        int key = readKey();

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
        } else if (key == '?') {
            showHelpScreen();
            clearScreen();
        } else if (key == 'm') {
            options.showFullMap = !options.showFullMap;
            clearScreen();
            message = paint(options.showFullMap ? "Full map view. Press M again for the camera view."
                                                : "Camera view - it follows you around.", muted);
        } else if (key == 'r') {
            message = "";
            if (confirm(game, options, "Restart this maze from the beginning?")) {
                restartGame(game);
                message = paint("Same maze, fresh start. Good luck!", muted);
            }
        } else if (key == 'q' || key == escapeKey) {
            message = "";
            if (confirm(game, options, "Give up and return to the menu?")) {
                quitGame(game);
                return false;
            }
        }
    }

    drawGame(game, options, paint("You found the exit! Press any key...", bold + green));
    readKey();
    return true;
}

void playFreeGames(GameState& game, int size, Difficulty difficulty, const string& caption, const string& username) {
    startNewGame(game, size, difficulty);
    while (true) {
        if (!playMaze(game, caption)) {
            endGame(game);
            return;
        }
        VictoryChoice choice = showVictoryScreen(game, nullptr, true, username);
        if (choice == VICTORY_NEW_MAZE) {
            startNewGame(game, size, difficulty);
        } else if (choice == VICTORY_REPLAY) {
            restartGame(game);
        } else {
            endGame(game);
            return;
        }
    }
}

void playDailyChallenge(GameState& game, const string& username) {
    startGameWithSeed(game, dailyMazeSize, NORMAL, dailySeed);
    while (true) {
        if (!playMaze(game, "Daily challenge")) {
            endGame(game);
            return;
        }
        if (showVictoryScreen(game, nullptr, false, username) == VICTORY_REPLAY) {
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

    while (level != 0) {
        LevelInfo info = getLevel(level);
        startNewGame(game, info.size, info.difficulty);         

        if (!playMaze(game, "Level " + to_string(level) + ": " + levelName(level))) {
            endGame(game);
            level = selectLevel(progress, level);           
            continue;
        }

        int stars = calculateStars(getPlayer(game).moves, getOptimalMoves(game));
        CampaignResult result;
        result.level = level;
        result.newLevelUnlocked = saveLevelResult(progress, level, stars, calculateScore(game));
        result.campaignComplete = isCampaignComplete(progress);
        result.totalStars = getTotalStars(progress);
        savePlayerProgress(username, progress);

        VictoryChoice choice = showVictoryScreen(game, &result, true, username);
        endGame(game);
        if (choice == VICTORY_NEXT_LEVEL) {
            level++;
        } else if (choice == VICTORY_LEVEL_SELECT) {
            level = selectLevel(progress, level);
        } else if (choice == VICTORY_MAIN_MENU) {
            return;
        }
    }
}
