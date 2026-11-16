#include "mainMenu.h"
#include "../Logic/accounts.h"
#include "../Logic/campaign.h"
#include "console.h"
#include "../Logic/gameLogic.h"
#include "gameSession.h"
#include "input.h"
#include "layout.h"
#include "loginMenu.h"
#include "renderer.h"
#include "screens.h"
#include "../Logic/validation.h"

using namespace std;

bool showPlayerMenu(const string& username) {
    GameState game;
    int quickSize = defaultMazeSize;                 
    Difficulty quickDifficulty = NORMAL;
    bool quickWarden = false;                     

    const string menuItems[7] = {
        "Campaign: The Star Compass",
        "New game",
        "Quick game",
        "How to play",
        "Leaderboard",
        "Log out",
        "Exit",
    };

    int selected = 0;
    while (true) {
        CampaignProgress progress = loadPlayerProgress(username);
        string footer = paint("Player: " + username, bold + cyan) + "     "
                      + paint("Chapter " + to_string(progress.unlockedLevels) + "/" + to_string(levelCount), orange)
                      + "     " + paint("Stars " + to_string(getTotalStars(progress)) + "/" + to_string(3 * levelCount),
                                        yellow);

        int choice = runMenu("MAIN MENU", menuItems, 7, selected, footer);
        if (choice >= 0) {
            selected = choice;
        }

        if (choice == 0) {
            playCampaign(game, username);
        } else if (choice == 1) {
            if (configureNewGame(quickSize, quickDifficulty, quickWarden)) {
                playFreeGames(game, quickSize, quickDifficulty, quickWarden, "", username);
            }
        } else if (choice == 2) {
            playFreeGames(game, quickSize, quickDifficulty, quickWarden, "Quick game", username);
        } else if (choice == 3) {
            showHelpScreen();
        } else if (choice == 4) {
            showLeaderboardScreen();
        } else if (choice == 5) {
            endGame(game);
            return true;     
        } else {              
            if (askConfirm("Leave MAZEBOUND?", "Yes, leave", "No, stay")) {
                endGame(game);
                return false;
            }
        }
    }
}

void showMainMenu() {
    initializeConsole();
    initializeRandom();
    clearScreen();

    bool running = true;
    while (running) {
        string username = showLoginMenu();
        if (username == "") {
            running = false;                            
        } else {
            running = showPlayerMenu(username);            
        }
    }
    restoreConsole();
}
