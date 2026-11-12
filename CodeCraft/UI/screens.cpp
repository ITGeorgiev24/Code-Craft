#include "screens.h"
#include "console.h"
#include "input.h"
#include "layout.h"
#include "../Logic/records.h"
#include "renderer.h"
#include "../Logic/validation.h"

using namespace std;

string starsText(int stars) {
    string text;
    for (int i = 0; i < 3; i++) {
        text += i < stars ? "*" : "-";
        if (i < 2) {
            text += " ";
        }
    }
    return text;
}

string statLine(const string& label, const string& value) {
    return padRight(label, 16) + value + "\n";
}


bool configureNewGame(int& size, Difficulty& difficulty) {
    const string difficultyItems[5] = {
        "Easy    - many loops, extra key, 2 compasses           x1 score",
        "Normal  - a few loops, a key for every door, 1 compass x2 score",
        "Hard    - only one true path, fewer keys, no compass   x3 score",
        "Expert  - FOG OF WAR: you see only what you explore    x4 score",
        "Back",
    };

    while (true) {
        int chosen = runMenu("NEW GAME - choose difficulty", difficultyItems, 5, (int)difficulty, "");
        if (chosen < 0 || chosen == 4) {
            return false; 
        }
        difficulty = (Difficulty)chosen;

        int selected = 1;
        bool backToDifficulty = false;
        while (!backToDifficulty) {
            const string sizeItems[3] = {
                "Maze size: " + to_string(size) + " x " + to_string(size) + "   (choose to change)",
                "Start game",
                "Back",
            };
            string footer = paint("Every game is a brand-new random maze. Big mazes are shown with a camera"
                                  " (M = full map).", muted);
            int choice = runMenu("NEW GAME - " + difficultyName(difficulty), sizeItems, 3, selected, footer, 3);
            if (choice == 0) {
                size = readNumberInRange("Maze size (N x N)", minMazeSize, maxMazeSize, size);
                selected = 1;
            } else if (choice == 1) {
                return true;            
            } else {
                backToDifficulty = true; 
            }
        }
    }
}


void showHelpScreen() {
    string text;
    text += paint("Goal", bold + cyan) + "\n";
    text += "  Start at the top-left " + playerSymbol() + "and reach the exit " + exitSymbol() + " in the bottom-right corner.\n\n";
    text += paint("Rules", bold + cyan) + "\n";
    text += "  - Walls " + wallSymbol() + " block the way. Doors " + doorSymbol() + " are shortcuts - a key "
          + itemSymbol(ITEM_KEY) + "opens one.\n";
    text += "  - Coin " + itemSymbol(ITEM_COIN) + "= +" + to_string(coinValue) + " points. Compass "
          + itemSymbol(ITEM_COMPASS) + "= one free hint. H = the next " + to_string(hintVisibleSteps) + " steps.\n\n";
    text += paint("Difficulty", bold + cyan) + "\n";
    text += "  Easy, Normal, Hard, Expert - fewer loops, keys and compasses each time.\n";
    text += "  Expert hides the maze in fog " + fogSymbol() + ". Campaign: " + to_string(levelCount)
          + " levels, each unlocks the next.\n\n";
    text += paint("Score (depends on your moves)", bold + cyan) + "\n";
    text += "  size x 40 x difficulty (x1..x4) - " + to_string(extraMovePenalty) + " per move above the shortest route\n";
    text += "  + " + to_string(coinValue) + " per coin - " + to_string(paidHintCost) + " per paid hint.   Stars: * * * = shortest route.\n\n";
    text += paint("Keys", bold + cyan) + "\n";
    text += "  Arrows / W A S D  move     H  hint     I  info about your location\n";
    text += "  L  legend     M  full map     R  restart     ?  help     Q / Esc  menu\n";
    showPage("HOW TO PLAY", text);
}

void showLegendScreen() {
    string text;
    text += playerSymbol() + "  You       Your current position.\n";
    text += wallSymbol() + "  Wall      Solid stone. Nobody can pass through.\n";
    text += doorSymbol() + "  Door      A secret door - a shortcut towards the exit.\n";
    text += "            Needs one key. After opening it becomes a corridor.\n";
    text += exitSymbol() + "  Exit      Step on it to win (bottom-right corner).\n";
    text += itemSymbol(ITEM_KEY) + "  Key       Opens one secret door.\n";
    text += itemSymbol(ITEM_COIN) + "  Coin      +" + to_string(coinValue) + " points.\n";
    text += itemSymbol(ITEM_COMPASS) + "  Compass   Your next hint is free.\n";
    text += trailSymbol() + "  Trail     Squares you have already visited.\n";
    text += hintSymbol() + "  Hint      The next steps of the shortest route.\n";
    text += fogSymbol() + "  Fog       Unexplored area (Expert). Walk closer to see it.\n";
    text += arrowSymbol() + "  Arrows    Entrance (top-left) and way out (bottom-right).\n";
    text += edgeSymbol() + "  Edge      The maze continues there - the camera follows you.\n";
    showPage("LEGEND - what the symbols mean", text);
}


void showLeaderboardScreen() {
    ScoreTable table;
    loadLeaderboard(table);

    string text;
    if (table.count == 0) {
        text = "No results yet. Win a game to get on the board!\n";
    } else {
        text += paint(padRight("#", 4) + padRight("Name", 18) + padRight("Size", 8) + padRight("Level", 9)
                      + padRight("Moves", 8) + "Score", bold + white) + "\n";
        text += paint(string(52, '-'), dark) + "\n";
        for (int i = 0; i < table.count; i++) {
            ScoreEntry entry = table.entries[i];
            string row = padRight(to_string(i + 1), 4) + padRight(entry.playerName, 18)
                       + padRight(to_string(entry.mazeSize) + "x" + to_string(entry.mazeSize), 8)
                       + padRight(difficultyName(entry.difficulty), 9)
                       + padRight(to_string(entry.moves), 8) + to_string(entry.score);
            if (i == 0) {
                row = paint(row, bold + yellow);      
            } else if (i < 3) {
                row = paint(row, white);
            }
            text += row + "\n";
        }
    }
    deleteLeaderboard(table);
    showPage("LEADERBOARD - top " + to_string(leaderboardSize), text);
}


void showErrorScreen(const string& text) {
    showPage("ERROR", paint("! " + text, red) + "\n");
}


VictoryChoice showVictoryScreen(const GameState& game, const CampaignResult* campaign, bool newMazeAllowed,
                                const string& username) {
    int width = getScreenWidth();
    Player player = getPlayer(game);
    int size = getMazeSize(game);
    int score = calculateScore(game);
    int stars = calculateStars(player.moves, getOptimalMoves(game));

    const string escapedBanner[6] = {
        "__   __            _____                              _ _ ",
        "\\ \\ / /__  _   _  | ____|___  ___ __ _ _ __   ___  __| | |",
        " \\ V / _ \\| | | | |  _| / __|/ __/ _` | '_ \\ / _ \\/ _` | |",
        "  | | (_) | |_| | | |___\\__ \\ (_| (_| | |_) |  __/ (_| |_|",
        "  |_|\\___/ \\__,_| |_____|___/\\___\\__,_| .__/ \\___|\\__,_(_)",
        "                                      |_|                 ",
    };
    const string levelCompleteBanner[6] = {
        " _                   _    ____                      _      _       _ ",
        "| |    _____   _____| |  / ___|___  _ __ ___  _ __ | | ___| |_ ___| |",
        "| |   / _ \\ \\ / / _ \\ | | |   / _ \\| '_ ` _ \\| '_ \\| |/ _ \\ __/ _ \\ |",
        "| |__|  __/\\ V /  __/ | | |__| (_) | | | | | | |_) | |  __/ ||  __/_|",
        "|_____\\___| \\_/ \\___|_|  \\____\\___/|_| |_| |_| .__/|_|\\___|\\__\\___(_)",
        "                                             |_|                     ",
    };

    string banner;
    for (int i = 0; i < 6; i++) {
        banner += paint(campaign != nullptr ? levelCompleteBanner[i] : escapedBanner[i], bold + green) + "\n";
    }

    string inside;
    if (campaign != nullptr) {
        inside += paint("Level " + to_string(campaign->level) + ": " + levelName(campaign->level), bold + white) + "\n\n";
    }
    inside += statLine("Maze", to_string(size) + "x" + to_string(size) + "  " + difficultyName(getDifficulty(game)));
    inside += statLine("Moves", to_string(player.moves) + "  (shortest route: " + to_string(getOptimalMoves(game)) + ")");
    inside += statLine("Coins", to_string(player.coins) + " / " + to_string(getTotalCoins(game)));
    inside += statLine("Doors opened", to_string(player.doorsOpened));
    inside += statLine("Hints used", to_string(getHintsUsed(game)));
    inside += "\n" + padRight("Stars", 16) + paint("[ " + starsText(stars) + " ]", bold + yellow) + "\n";
    inside += padRight("SCORE", 16) + paint(to_string(score), bold + yellow) + "\n";

    string screen = centerBlock(banner, width) + "\n" + centerBlock(buildMazeBox(inside), width);
    if (campaign != nullptr && campaign->campaignComplete) {
        screen += centerLine(paint("You conquered every level of MAZEBOUND! Stars: " + to_string(campaign->totalStars)
                                   + " / " + to_string(3 * levelCount), bold + magenta), width) + "\n";
    } else if (campaign != nullptr && campaign->newLevelUnlocked) {
        LevelInfo next = getLevel(campaign->level + 1);
        screen += centerLine(paint("Unlocked: Level " + to_string(next.number) + " - " + levelName(next.number) + " ("
                                   + to_string(next.size) + "x" + to_string(next.size) + " "
                                   + difficultyName(next.difficulty) + ")", magenta), width) + "\n";
    }
    showScreen(screen, 4);

    if (qualifiesForLeaderboard(score, player.moves)) {
        int place = -1;
        if (submitScore(username, game, place)) {
            printCentered(paint("New high score! " + username + " is #" + to_string(place) + " on the leaderboard.", bold + yellow));
        } else {
            printError("The result could not be saved.");
        }
    }
    waitForKey();

    if (campaign != nullptr && isValidLevel(campaign->level + 1)) {
        const string menuOptions[4] = { "Next level", "Play this level again (new maze)", "Level select", "Main menu" };
        int chosen = runMenu("What next?", menuOptions, 4, 0, "");
        if (chosen == 0) return VICTORY_NEXT_LEVEL;
        if (chosen == 1) return VICTORY_NEW_MAZE;
        if (chosen == 2) return VICTORY_LEVEL_SELECT;
        return VICTORY_MAIN_MENU;
    }
    if (campaign != nullptr) {
        const string menuOptions[3] = { "Play this level again (new maze)", "Level select", "Main menu" };
        int chosen = runMenu("What next?", menuOptions, 3, 1, "");
        if (chosen == 0) return VICTORY_NEW_MAZE;
        if (chosen == 1) return VICTORY_LEVEL_SELECT;
        return VICTORY_MAIN_MENU;
    }
    if (newMazeAllowed) {
        const string menuOptions[3] = { "Play again (new maze)", "Replay this maze", "Main menu" };
        int chosen = runMenu("What next?", menuOptions, 3, 0, "");
        if (chosen == 0) return VICTORY_NEW_MAZE;
        if (chosen == 1) return VICTORY_REPLAY;
        return VICTORY_MAIN_MENU;
    }
    const string menuOptions[2] = { "Play the daily maze again", "Main menu" };
    int chosen = runMenu("What next?", menuOptions, 2, 0, "");
    return chosen == 0 ? VICTORY_REPLAY : VICTORY_MAIN_MENU;
}


string levelName(int level) {
    const string levelNames[levelCount] = {
        "First Steps", "Twisting Halls", "Hidden Doors", "Coin Cellar", "Deep Corridors",
        "Stone Fortress", "Forgotten Keep", "Dark Passage", "Shadow Depths", "MAZEBOUND",
    };
    if (!isValidLevel(level)) {
        return "Unknown";
    }
    return levelNames[level - 1];
}

int selectLevel(const CampaignProgress& progress, int suggestedLevel) {
    string items[levelCount + 1];  
    for (int level = 1; level <= levelCount; level++) {
        LevelInfo info = getLevel(level);
        string item = padRight(levelName(level), 16)
                    + padRight(to_string(info.size) + "x" + to_string(info.size), 7)
                    + padRight(difficultyName(info.difficulty), 8);
        if (!isLevelUnlocked(progress, level)) {
            item += "[locked]";
        } else if (isLevelCompleted(progress, level)) {
            item += "[" + starsText(progress.bestStars[level - 1]) + "] best " + to_string(progress.bestScores[level - 1]);
        } else {
            item += "new!";
        }
        items[level - 1] = item;
    }
    items[levelCount] = "Back";

    string notice;
    int selected = suggestedLevel - 1;
    while (true) {
        string footer = paint("Stars collected: " + to_string(getTotalStars(progress)) + " / " + to_string(3 * levelCount), yellow);
        if (notice != "") {
            footer += "\n" + paint(notice, red);
        }
        int chosen = runMenu("CAMPAIGN - choose a level", items, levelCount + 1, selected, footer);
        if (chosen < 0 || chosen == levelCount) {
            return 0; 
        }
        if (isLevelUnlocked(progress, chosen + 1)) {
            return chosen + 1;
        }
        selected = chosen;
        notice = "Level " + to_string(chosen + 1) + " is locked. Complete level " + to_string(chosen) + " first.";
    }
}


string buildLocationMessage(const GameState& game) {
    LocationInfo info = describeLocation(game);
    const string directionNames[4] = { "Up", "Down", "Left", "Right" };

    string line1 = paint("Position", cyan) + " row " + to_string(info.position.row) + ", col "
                 + to_string(info.position.col) + (info.isStart ? " (start)" : "") + " |";
    for (int i = 0; i < 4; i++) {
        string what = "open";
        if (info.neighbors[i] == NEAR_WALL) what = "wall";
        if (info.neighbors[i] == NEAR_DOOR) what = "locked door";
        if (info.neighbors[i] == NEAR_EDGE) what = "edge";
        if (info.neighbors[i] == NEAR_EXIT) what = "EXIT";
        line1 += " " + directionNames[i] + ": " + what + (i < 3 ? "," : "");
    }

    string line2 = paint("Exit", cyan) + " is " + to_string(info.stepsToExit) + " steps away | items within "
                 + to_string(nearbyRadius) + " steps: " + to_string(info.itemsNearby)
                 + " | doors nearby: " + to_string(info.doorsNearby);
    return line1 + "\n" + line2;
}
