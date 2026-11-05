#include "mainMenu.h"
#include "accounts.h"
#include "campaign.h"
#include "console.h"
#include "gameLogic.h"
#include "gameSession.h"
#include "input.h"
#include "layout.h"
#include "loginMenu.h"
#include "renderer.h"
#include "screens.h"
#include "validation.h"

using namespace std;

bool showPlayerMenu(const string& username) {
    GameState game;
    int quickSize = defaultMazeSize;       
    Difficulty quickDifficulty = NORMAL;

    const string menuItems[8] = {
        "Campaign (10 levels)",
        "New game",
        "Quick game",
        "Daily challenge",
        "How to play",
        "Leaderboard",
        "Log out",
        "Exit",
    };

    int selected = 0;
    while (true) {
        CampaignProgress progress = loadPlayerProgress(username);
        string footer = paint("Player: " + username, bold + cyan) + "   "
            + paint("Campaign: level " + to_string(progress.unlockedLevels) + "/" + to_string(levelCount)
                + " unlocked, " + to_string(getTotalStars(progress)) + " stars", yellow) + "\n"
            + paint("Quick game: random " + to_string(quickSize) + "x" + to_string(quickSize) + " "
                + difficultyName(quickDifficulty) + "  |  Daily: always the same "
                + to_string(dailyMazeSize) + "x" + to_string(dailyMazeSize) + " maze", muted);

        int choice = runMenu("MAIN MENU", menuItems, 8, selected, footer);
        if (choice >= 0) {
            selected = choice;
        }

        if (choice == 0) {
            playCampaign(game, username);
        }
        else if (choice == 1) {
            if (configureNewGame(quickSize, quickDifficulty)) {
                playFreeGames(game, quickSize, quickDifficulty, "", username);
            }
        }
        else if (choice == 2) {
            playFreeGames(game, quickSize, quickDifficulty, "Quick game", username);
        }
        else if (choice == 3) {
            playDailyChallenge(game, username);
        }
        else if (choice == 4) {
            showHelpScreen();
        }
        else if (choice == 5) {
            showLeaderboardScreen();
        }
        else if (choice == 6) {
            endGame(game);
            return true;  
        }
        else {           
            showScreen(buildTitle(2) + centerLine(paint("See you soon in the maze, " + username + "!", muted),
                getScreenWidth()) + "\n\n", 1);
            if (askYesNo("Do you really want to leave MAZEBOUND?", true)) {
                endGame(game);
                return false;
            }
        }
    }
}

void showMainMenu() {
    initializeConsole();
    initializeRandom();

    bool running = true;
    while (running) {
        string username = showLoginMenu();
        if (username == "") {
            running = false;                       
        }
        else {
            running = showPlayerMenu(username);    
        }
    }
    restoreConsole();
}
