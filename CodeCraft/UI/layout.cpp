#include "layout.h"
#include "console.h"

#include <sstream>

using namespace std;

int visibleLength(const string& text) {
    int length = 0;
    int i = 0;
    int size = (int)text.length();
    while (i < size) {
        if (text[i] == '\x1b') {
            while (i < size && text[i] != 'm') {
                i++;
            }
            i++;
        }
        else {
            length++;
            i++;
        }
    }
    return length;
}

int countLines(const string& text) {
    int lines = 0;
    for (int i = 0; i < (int)text.length(); i++) {
        if (text[i] == '\n') {
            lines++;
        }
    }
    return lines;
}

string leftMargin(int textWidth, int screenWidth) {
    int margin = (screenWidth - textWidth) / 2;
    if (margin < 0) {
        margin = 0;
    }
    return string(margin, ' ');
}

string padRight(const string& text, int width) {
    int missing = width - visibleLength(text);
    if (missing <= 0) {
        return text;
    }
    return text + string(missing, ' ');
}

string centerLine(const string& line, int screenWidth) {
    return leftMargin(visibleLength(line), screenWidth) + line;
}

string centerBlock(const string& text, int screenWidth) {
    stringstream firstPass(text);
    string line;
    int widest = 0;
    while (getline(firstPass, line)) {
        if (visibleLength(line) > widest) {
            widest = visibleLength(line);
        }
    }

    string margin = leftMargin(widest, screenWidth);
    stringstream secondPass(text);
    string result;
    while (getline(secondPass, line)) {
        result += margin + line + "\n";
    }
    return result;
}

int getScreenWidth() {
    int columns = 0;
    int rows = 0;
    getConsoleSize(columns, rows);
    return columns;
}

int getScreenHeight() {
    int columns = 0;
    int rows = 0;
    getConsoleSize(columns, rows);
    return rows;
}


string buildMazeWall(int pieces, int gapEvery, int gapAt) {
    string wall;
    for (int i = 0; i < pieces; i++) {
        bool opening = i > 0 && i < pieces - 1 && i % gapEvery == gapAt;
        wall += paint("+", cyan);
        wall += opening ? "  " : paint("--", blue);
    }
    wall += paint("+", cyan);
    return wall;
}

string buildMazeBox(const string& content) {

    stringstream firstPass(content);
    string line;
    int widest = 0;
    while (getline(firstPass, line)) {
        if (visibleLength(line) > widest) {
            widest = visibleLength(line);
        }
    }

  
    int pieces = (widest + 6 + 1 + 2) / 3;
    int inside = pieces * 3 - 1;
    int leftSpace = (inside - widest) / 2;

    string box = buildMazeWall(pieces, 6, 2) + "\n";
    box += paint("|", cyan) + string(inside, ' ') + paint("|", cyan) + "\n";

    stringstream secondPass(content);
    while (getline(secondPass, line)) {
        string inner = string(leftSpace, ' ') + line;
        box += paint("|", cyan) + padRight(inner, inside) + paint("|", cyan) + "\n";
    }

    box += paint("|", cyan) + string(inside, ' ') + paint("|", cyan) + "\n";
    box += buildMazeWall(pieces, 5, 3) + "\n";
    return box;
}

void showScreen(const string& content, int linesBelow) {
    int topSpace = (getScreenHeight() - countLines(content) - linesBelow) / 2;
    clearScreen();
    string screen;
    for (int i = 0; i < topSpace; i++) {
        screen += "\n";
    }
    writeText(screen + content);
}
