#include "screens.h"
#include "artwork.h"
#include "console.h"
#include "input.h"
#include "layout.h"
#include "../Logic/records.h"
#include "renderer.h"
#include "story.h"
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
    return paint(padRight(label, 16), muted) + value + "\n";
}

bool configureNewGame(int& size, Difficulty& difficulty, bool& warden) {
    const string difficultyItems[5] = {
        "Easy    - many loops, extra key, 2 compasses     x1",
        "Normal  - a few loops, a key for every door      x2",
        "Hard    - only one true path, fewer keys         x3",
        "Expert  - darkness: you see only around you      x4",
        "Back",
    };

    while (true) {
        int chosen = runMenu("NEW GAME - choose difficulty", difficultyItems, 5, (int)difficulty, "");
        if (chosen < 0 || chosen == 4) {
            return false;    
        }
        difficulty = (Difficulty)chosen;

        int selected = 2;
        bool backToDifficulty = false;
        string typed;            
        string error;
        while (!backToDifficulty) {
            string sizeText = typed == "" ? to_string(size) + " x " + to_string(size) : typed + "_";
            const string setupItems[4] = {
                "Maze size       < " + sizeText + " >",
                string("Shadow Warden   < ") + (warden ? "ON - it hunts you" : "OFF") + " >",
                "Start game",
                "Back",
            };
            string footer = warden ? paint("The Warden " + chaserSymbol() + paint("wakes up 10 s after your first move and follows you.", red), red)
                                   : "";
            if (error != "") {
                footer = paint(error, red) + (footer == "" ? "" : "\n" + footer);
            }
            MenuLook look;
            look.sideKeys = true;
            look.digitKeys = true;
            int choice = runMenu("NEW GAME - " + difficultyName(difficulty), setupItems, 4, selected, footer, 0, &look);

            if (look.typedKey != 0) {
                if (look.typedKey == backspaceKey) {
                    if (typed != "") {
                        typed.erase(typed.length() - 1);
                    }
                } else if (typed.length() < 2) {
                    typed += (char)look.typedKey;
                }
                error = "";
                selected = 0;
                continue;
            }
            if (typed != "" && choice < 0) {
                typed = "";                          
                error = "";
                selected = 0;
                continue;
            }
            if (typed != "" && look.sideMove == 0 && (choice == 0 || choice == 2)) {
                int number = 0;
                bool correct = textToInt(typed, number) && isInRange(number, minMazeSize, maxMazeSize);
                typed = "";
                if (!correct) {
                    error = "The maze size must be between " + to_string(minMazeSize) + " and "
                          + to_string(maxMazeSize) + ".";
                    selected = 0;
                    continue;
                }
                size = number;
                error = "";
                if (choice == 0) {
                    selected = 0;
                    continue;
                }
            }
            if (look.sideMove != 0) {
                typed = "";                          
            }

            if (choice == 0) {
                size += look.sideMove;
                if (size < minMazeSize) size = minMazeSize;
                if (size > maxMazeSize) size = maxMazeSize;
                error = "";
                selected = 0;
            } else if (choice == 1) {
                warden = !warden;                      
                selected = 1;
            } else if (choice == 2) {
                if (look.sideMove == 0) {
                    return true;                
                }
                selected = 2;
            } else if (choice == 3 && look.sideMove != 0) {
                selected = 3;                      
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
          + itemSymbol(ITEM_COMPASS) + "= one free hint. H = the next " + to_string(hintVisibleSteps) + " steps.\n";
    text += "  - Every " + to_string(coinsPerReward) + " coins = a treasure: a compass, a key, then +"
          + to_string(rewardPoints) + " points. All coins = +" + to_string(allCoinsBonus) + ".\n";
    text += "  - The Shadow Warden " + chaserSymbol() + "hunts you in some chapters. A treasure freezes it.\n\n";
    text += paint("Score and time", bold + cyan) + "\n";
    text += "  size x 40 x difficulty (x1..x4) - " + to_string(extraMovePenalty) + " per move above the shortest route\n";
    text += "  + coins and treasures - " + to_string(paidHintCost) + " per paid hint. The clock starts with your first move.\n\n";
    text += paint("Keys", bold + cyan) + "\n";
    text += "  Arrows / W A S D  move     H  hint     I  info about your location\n";
    text += "  L  legend     R  restart     ?  help     Esc  menu\n";
    showPage("HOW TO PLAY", text);
}

void showLegendScreen() {
    string text;
    text += playerSymbol() + "  You       Your current position.\n";
    text += chaserSymbol() + "  Warden    The shadow that hunts you. Do not let it touch you!\n";
    text += wallSymbol() + "  Wall      Solid stone. Nobody can pass through.\n";
    text += doorSymbol() + "  Door      A secret door - a shortcut towards the exit.\n";
    text += "            Needs one key. After opening it becomes a corridor.\n";
    text += exitSymbol() + "  Exit      Step on it to win (bottom-right corner).\n";
    text += itemSymbol(ITEM_KEY) + "  Key       Opens one secret door.\n";
    text += itemSymbol(ITEM_COIN) + "  Coin      +" + to_string(coinValue) + " points. Every "
          + to_string(coinsPerReward) + " coins open a treasure.\n";
    text += itemSymbol(ITEM_COMPASS) + "  Compass   Your next hint is free.\n";
    text += trailSymbol() + "  Trail     Squares you have already visited.\n";
    text += hintSymbol() + "  Hint      The next steps of the shortest route.\n";
    text += fogSymbol() + "  Darkness  Unexplored area (Expert). Walk closer to see it.\n";
    text += arrowSymbol() + "  Arrows    Entrance (top-left) and way out (bottom-right).\n";
    text += edgeSymbol() + "  Edge      The maze continues there - the camera follows you.\n";
    showPage("LEGEND - what the symbols mean", text);
}

string trophyPicture(int , int ) {
    return trophyArt();
}

void showLeaderboardScreen() {
    ScoreTable table;
    loadLeaderboard(table);

    string text;
    if (table.count == 0) {
        text = "No results yet. Win a game to get on the board!\n";
    } else {
        text += paint(padRight("#", 5) + padRight("Name", 18) + padRight("Maze", 8) + padRight("Level", 9)
                      + padRight("Moves", 8) + padRight("Time", 9) + "Score", bold + white) + "\n";
        text += paint(string(63, '-'), dark) + "\n";
        const string placeColors[3] = { bold + yellow, "\x1b[1;38;5;252m", "\x1b[1;38;5;173m" };      
        for (int i = 0; i < table.count; i++) {
            ScoreEntry entry = table.entries[i];
            string place = i == 0 ? "1st" : i == 1 ? "2nd" : i == 2 ? "3rd" : to_string(i + 1) + ".";
            string time = entry.seconds > 0 ? formatTime(entry.seconds) : "--:--";
            string row = padRight(place, 5) + padRight(entry.playerName, 18)
                       + padRight(to_string(entry.mazeSize) + "x" + to_string(entry.mazeSize), 8)
                       + padRight(difficultyName(entry.difficulty), 9)
                       + padRight(to_string(entry.moves), 8) + padRight(time, 9) + to_string(entry.score);
            text += paint(row, i < 3 ? placeColors[i] : "\x1b[38;5;252m") + "\n";
        }
    }
    deleteLeaderboard(table);

    MenuLook look;
    look.left = trophyPicture;
    look.right = trophyPicture;
    showPage("LEADERBOARD - top " + to_string(leaderboardSize), text, "Press any key to go back...", &look);
}

void showErrorScreen(const string& text) {
    showPage("ERROR", paint("! " + text, red) + "\n");
}

string sparkleLine(int width, int frame) {
    string line;
    for (int x = 0; x < width; x++) {
        int value = (x * 7 + frame * 3) % 23;
        if (value == 0) {
            line += paint("*", bold + yellow);
        } else if (value == 11) {
            line += paint("+", white);
        } else if (value == 5) {
            line += paint(".", cyan);
        } else {
            line += " ";
        }
    }
    return line;
}

VictoryChoice showVictoryScreen(const GameState& game, const CampaignResult* campaign, const string& username) {
    Player player = getPlayer(game);
    int size = getMazeSize(game);
    int score = calculateScore(game);
    int stars = calculateStars(player.moves, getOptimalMoves(game));
    int seconds = getElapsedSeconds(game);

    string recordLine;
    if (qualifiesForLeaderboard(score, player.moves)) {
        int place = -1;
        if (submitScore(username, game, place)) {
            recordLine = paint("NEW HIGH SCORE! " + username + " is #" + to_string(place) + " on the leaderboard.",
                               bold + yellow);
        } else {
            recordLine = paint("! The result could not be saved.", red);
        }
    }

    const string escapedBanner[6] = {
        "__   __            _____                              _ _ ",
        "\\ \\ / /__  _   _  | ____|___  ___ __ _ _ __   ___  __| | |",
        " \\ V / _ \\| | | | |  _| / __|/ __/ _` | '_ \\ / _ \\/ _` | |",
        "  | | (_) | |_| | | |___\\__ \\ (_| (_| | |_) |  __/ (_| |_|",
        "  |_|\\___/ \\__,_| |_____|___/\\___\\__,_| .__/ \\___|\\__,_(_)",
        "                                      |_|                 ",
    };
    const string chapterBanner[6] = {
        "  ____ _                 _              ____                   _ ",
        " / ___| |__   __ _ _ __ | |_ ___ _ __  |  _ \\  ___  _ __   ___| |",
        "| |   | '_ \\ / _` | '_ \\| __/ _ \\ '__| | | | |/ _ \\| '_ \\ / _ \\ |",
        "| |___| | | | (_| | |_) | ||  __/ |    | |_| | (_) | | | |  __/_|",
        " \\____|_| |_|\\__,_| .__/ \\__\\___|_|    |____/ \\___/|_| |_|\\___(_)",
        "                  |_|                                             ",
    };

    string inside;
    if (campaign != nullptr) {
        inside += paint("Chapter " + to_string(campaign->level) + ": " + levelName(campaign->level), bold + white) + "\n";
        inside += paint(chapterOutro(campaign->level), yellow) + "\n\n";
    }
    string timeText = paint(formatTime(seconds), bold + white);
    if (campaign != nullptr && campaign->newBestTime) {
        timeText += "  " + paint("new best time!", bold + green);
    }
    inside += statLine("Maze", to_string(size) + "x" + to_string(size) + "  " + difficultyName(getDifficulty(game)));
    inside += statLine("Time", timeText);
    inside += statLine("Moves", to_string(player.moves) + "  (shortest route: " + to_string(getOptimalMoves(game)) + ")");
    inside += statLine("Coins", to_string(player.coins) + " / " + to_string(getTotalCoins(game))
                       + (player.coins == getTotalCoins(game) && player.coins > 0 ? paint("  all found!", green) : ""));
    inside += statLine("Treasures", to_string(player.rewards) + "  (+" + to_string(player.bonusPoints) + " bonus points)");
    inside += statLine("Doors opened", to_string(player.doorsOpened));
    inside += statLine("Hints used", to_string(getHintsUsed(game)));
    inside += "\n" + paint(padRight("Stars", 16), muted) + paint("[ " + starsText(stars) + " ]", bold + yellow) + "\n";
    inside += paint(padRight("SCORE", 16), muted) + paint(to_string(score), bold + yellow) + "\n";
    string box = buildMazeBox(inside);

    string notes;
    if (campaign != nullptr && campaign->campaignComplete && campaign->level == levelCount) {
        notes += paint("All ten shards are together! Stars: " + to_string(campaign->totalStars) + " / "
                       + to_string(3 * levelCount), bold + magenta) + "\n";
    } else if (campaign != nullptr && campaign->newLevelUnlocked) {
        LevelInfo next = getLevel(campaign->level + 1);
        notes += paint("Unlocked: Chapter " + to_string(next.number) + " - " + levelName(next.number) + " ("
                       + to_string(next.size) + "x" + to_string(next.size) + " " + difficultyName(next.difficulty) + ")",
                       magenta) + "\n";
    }
    if (recordLine != "") {
        notes += recordLine + "\n";
    }

    const int greens[6] = { 46, 82, 118, 154, 118, 82 };
    clearKeyBuffer();
    int frame = 0;
    while (true) {
        int width = getScreenWidth();
        string banner;
        const string* bannerLines = campaign != nullptr ? chapterBanner : escapedBanner;
        for (int i = 0; i < 6; i++) {
            banner += paint(bannerLines[i], "\x1b[1;38;5;" + to_string(greens[(frame + i) % 6]) + "m") + "\n";
        }
        string body = withSideArt(box, campaign != nullptr ? levelArt(campaign->level, 0) : torchArt(frame),
                                  campaign != nullptr ? levelArt(campaign->level, 1) : torchArt(frame + 2), 3, width);

        string screen;
        int needed = 8 + blockHeight(body) + blockHeight(notes) + 3;
        if (needed <= getScreenHeight()) {
            screen += centerLine(sparkleLine(blockWidth(banner), frame), width) + "\n";
            screen += centerBlock(banner, width);
        } else {
            screen += centerLine(paint(campaign != nullptr ? "*** CHAPTER DONE! ***" : "*** YOU ESCAPED! ***",
                                       bold + green), width) + "\n";
        }
        screen += centerBlock(body, width);
        if (notes != "") {
            screen += centerBlock(notes, width);
        }
        screen += "\n" + centerLine(paint("Press any key to continue...", frame % 6 < 4 ? muted : dark), width) + "\n";
        showScreen(screen);

        if (readKeyTimeout(150) != noKey) {
            break;
        }
        frame++;
    }

    if (campaign != nullptr && campaign->level == levelCount) {
        showEnding();
    }

    if (campaign != nullptr && isValidLevel(campaign->level + 1)) {
        const string menuOptions[4] = { "Next chapter", "Play this chapter again (new maze)", "Level select", "Main menu" };
        int chosen = runMenu("What next?", menuOptions, 4, 0, "");
        if (chosen == 0) return VICTORY_NEXT_LEVEL;
        if (chosen == 1) return VICTORY_NEW_MAZE;
        if (chosen == 2) return VICTORY_LEVEL_SELECT;
        return VICTORY_MAIN_MENU;
    }
    if (campaign != nullptr) {
        const string menuOptions[3] = { "Play this chapter again (new maze)", "Level select", "Main menu" };
        int chosen = runMenu("What next?", menuOptions, 3, 1, "");
        if (chosen == 0) return VICTORY_NEW_MAZE;
        if (chosen == 1) return VICTORY_LEVEL_SELECT;
        return VICTORY_MAIN_MENU;
    }
    const string menuOptions[3] = { "Play again (new maze)", "Replay this maze", "Main menu" };
    int chosen = runMenu("What next?", menuOptions, 3, 0, "");
    if (chosen == 0) return VICTORY_NEW_MAZE;
    if (chosen == 1) return VICTORY_REPLAY;
    return VICTORY_MAIN_MENU;
}

string wardenPicture(int , int ) {
    return levelArt(7, 1);        
}

VictoryChoice showCaughtScreen(const GameState& game, int level) {
    Player player = getPlayer(game);
    string footer = paint("The Shadow Warden caught you after " + formatTime(getElapsedSeconds(game)) + " and "
                          + to_string(player.moves) + " moves.", red) + "\n"
                  + paint("Tip: treasures (every " + to_string(coinsPerReward) + " coins) freeze the Warden for "
                          + to_string(chaserFreezeMs / 1000) + " seconds.", muted);
    MenuLook look;
    look.left = wardenPicture;
    if (level == 0) {
        const string options[3] = { "Try again (new maze)", "Replay this maze", "Main menu" };
        int chosen = runMenu("CAUGHT!", options, 3, 0, footer, 0, &look);
        if (chosen == 0) return VICTORY_NEW_MAZE;
        if (chosen == 1) return VICTORY_REPLAY;
        return VICTORY_MAIN_MENU;
    }
    const string options[3] = { "Try again (new maze)", "Level select", "Main menu" };
    int chosen = runMenu("CAUGHT!  Chapter " + to_string(level) + ": " + levelName(level), options, 3, 0, footer, 0, &look);
    if (chosen == 0) return VICTORY_NEW_MAZE;
    if (chosen == 1) return VICTORY_LEVEL_SELECT;
    return VICTORY_MAIN_MENU;
}

const CampaignProgress* shownProgress = nullptr;

string chapterLeftPicture(int frame, int selected) {
    if (selected >= levelCount) return torchArt(frame);
    if (shownProgress != nullptr && !isLevelUnlocked(*shownProgress, selected + 1)) return lockArt();
    return levelArt(selected + 1, 0);
}

string chapterRightPicture(int frame, int selected) {
    if (selected >= levelCount) return torchArt(frame + 2);
    if (shownProgress != nullptr && !isLevelUnlocked(*shownProgress, selected + 1)) return lockArt();
    return levelArt(selected + 1, 1);
}

string chapterNote(int , int selected) {
    if (selected >= levelCount || shownProgress == nullptr) {
        return "\n";
    }
    int level = selected + 1;
    if (!isLevelUnlocked(*shownProgress, level)) {
        return paint("Locked - finish chapter " + to_string(level - 1) + " first.", red) + "\n";
    }
    string note;
    if (levelHasChaser(level)) {
        note = paint("Beware: the Shadow Warden hunts you here!", bold + red);
    } else if (getLevel(level).difficulty == EXPERT) {
        note = paint("Darkness: you see only a few steps around you.", cyan);
    }
    int bestTime = shownProgress->bestTimes[level - 1];
    if (bestTime > 0) {
        note += (note == "" ? "" : "   ") + paint("Best time " + formatTime(bestTime), orange);
    }
    return note + "\n";
}

int selectLevel(const CampaignProgress& progress, int suggestedLevel) {
    string items[levelCount + 1];       
    for (int level = 1; level <= levelCount; level++) {
        LevelInfo info = getLevel(level);
        string item = padRight("Ch." + to_string(level), 6) + padRight(levelName(level), 20)
                    + padRight(to_string(info.size) + "x" + to_string(info.size), 7)
                    + padRight(difficultyName(info.difficulty), 8);
        if (!isLevelUnlocked(progress, level)) {
            item += "[locked]";
        } else if (isLevelCompleted(progress, level)) {
            item += "[" + starsText(progress.bestStars[level - 1]) + "]";
        } else {
            item += "new!";
        }
        items[level - 1] = item;
    }
    items[levelCount] = "Back";

    int shards = 0;
    for (int level = 1; level <= levelCount; level++) {
        if (isLevelCompleted(progress, level)) {
            shards++;
        }
    }

    shownProgress = &progress;
    MenuLook look;
    look.left = chapterLeftPicture;
    look.right = chapterRightPicture;
    look.below = chapterNote;

    int selected = suggestedLevel - 1;
    while (true) {
        string footer = paint("Star shards: " + to_string(shards) + " / " + to_string(levelCount), cyan) + "     "
                      + paint("Stars: " + to_string(getTotalStars(progress)) + " / " + to_string(3 * levelCount), yellow);
        int chosen = runMenu("THE STAR COMPASS - choose a chapter", items, levelCount + 1, selected, footer, 0, &look);
        if (chosen < 0 || chosen == levelCount) {
            shownProgress = nullptr;
            return 0;    
        }
        if (isLevelUnlocked(progress, chosen + 1)) {
            shownProgress = nullptr;
            return chosen + 1;
        }
        selected = chosen;               
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
