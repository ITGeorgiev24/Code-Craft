#pragma once
#include <string>

using namespace std;

typedef string (*MenuArt)(int frame, int selected);

struct MenuLook {
    MenuArt left = nullptr;          
    MenuArt right = nullptr;         
    MenuArt below = nullptr;            
    bool sideKeys = false;               
    int sideMove = 0;                     
    bool digitKeys = false;                
    int typedKey = 0;                      
};

int runMenu(const string& caption, const string items[], int itemCount, int selected, const string& footer,
            int linesBelow = 0, MenuLook* look = nullptr);

bool askConfirm(const string& question, const string& yesText = "Yes", const string& noText = "No");

void showPage(const string& caption, const string& content, const string& prompt = "Press any key to go back...",
              MenuLook* look = nullptr);

string readText(const string& question);

string readPassword(const string& question);

void waitForKey(const string& text = "Press any key to continue...");

void printError(const string& text);

void printCentered(const string& text);
