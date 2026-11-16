#pragma once
#include <string>

using namespace std;

string buildTitle(int linesBelow, int frame = 0);

string torchArt(int frame);

string levelArt(int level, int side);

string lockArt();

string trophyArt();

string endingArt(int frame);

int blockWidth(const string& block);
int blockHeight(const string& block);

string besideEachOther(const string& left, const string& middle, const string& right, int gap);

string withSideArt(const string& middle, const string& left, const string& right, int gap, int maxWidth);
