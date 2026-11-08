#pragma once
#include <string>

using namespace std;

string buildTitle(int linesBelow);

int runMenu(const string& caption, const string items[], int itemCount, int selected, const string& footer,
    int linesBelow = 0);

void showPage(const string& caption, const string& content);

int readNumberInRange(const string& question, int minValue, int maxValue, int defaultValue);

string readText(const string& question);

string readPassword(const string& question);

bool askYesNo(const string& question, bool defaultAnswer);

void waitForKey(const string& text = "Press any key to continue...");

void printError(const string& text);

void printCentered(const string& text);