#pragma once
#include <string>

using namespace std;

const int minMazeSize = 10;
const int maxMazeSize = 50;
const int defaultMazeSize = 15;
const int maxNameLength = 16;

string trim(const string& text);

bool textToInt(const string& text, int& number);

bool isInRange(int value, int minValue, int maxValue);

bool isValidMazeSize(int size);

bool isValidPlayerName(const string& name);
