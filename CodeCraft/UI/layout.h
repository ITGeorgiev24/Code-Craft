#pragma once
#include <string>

using namespace std;

int visibleLength(const string& text);

int countLines(const string& text);

string leftMargin(int textWidth, int screenWidth);

string padRight(const string& text, int width);

string centerLine(const string& line, int screenWidth);

string centerBlock(const string& text, int screenWidth);

int getScreenWidth();
int getScreenHeight();

string buildMazeBox(const string& content);

void showScreen(const string& content, int linesBelow = 0);