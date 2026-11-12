#include "input.h"
#include "console.h"
#include "layout.h"
#include "renderer.h"
#include "../Logic/validation.h"

using namespace std;

const int answerWidth = 10;

void printQuestion(const string& question) {
    writeText(leftMargin(visibleLength(question) + answerWidth, getScreenWidth()) + question);
}

string buildTitle(int linesBelow) {
    int width = getScreenWidth();

    if (8 + linesBelow > getScreenHeight()) {
        return centerLine(paint(" M A Z E B O U N D ", titleTag), width) + "\n\n";
    }

    const string titleLines[5] = {
        " __  __    _     __________ ____   ___  _   _ _   _ ____  ",
        "|  \\/  |  / \\   |__  / ____| __ ) / _ \\| | | | \\ | |  _ \\ ",
        "| |\\/| | / _ \\    / /|  _| |  _ \\| | | | | | |  \\| | | | |",
        "| |  | |/ ___ \\  / /_| |___| |_) | |_| | |_| | |\\  | |_| |",
        "|_|  |_/_/   \\_\\/____|_____|____/ \\___/ \\___/|_| \\_|____/ ",
    };
    const string lineColors[5] = { "\x1b[38;5;51m", "\x1b[38;5;45m", "\x1b[38;5;39m", "\x1b[38;5;99m", "\x1b[38;5;171m" };

    string title;
    for (int i = 0; i < 5; i++) {
        title += paint(titleLines[i], bold + lineColors[i]) + "\n";
    }
    string result = centerBlock(title, width);

    string runner = playerSymbol();
    for (int i = 0; i < 18; i++) {
        runner += trailSymbol();
    }
    runner += arrowSymbol() + " " + exitSymbol();
    result += centerLine(runner, width) + "\n";
    result += centerLine(paint("Every maze is new. Every level is harder. Find the way out.", muted), width) + "\n\n";
    return result;
}

int runMenu(const string& caption, const string items[], int itemCount, int selected, const string& footer,
            int linesBelow) {
    int itemWidth = 0;
    for (int i = 0; i < itemCount; i++) {
        if (visibleLength(items[i]) > itemWidth) {
            itemWidth = visibleLength(items[i]);
        }
    }

    showCursor(false);
    while (true) {
        int width = getScreenWidth();

        string inside = paint(caption, bold + white) + "\n" + paint(string(caption.length(), '='), dark) + "\n\n";
        for (int i = 0; i < itemCount; i++) {
            string number = to_string(i + 1) + ". ";
            if (itemCount >= 10 && i < 9) {
                number = " " + number;         
            }
            string text = padRight(number + items[i], itemWidth + 4);
            if (i == selected) {
                inside += paint("> " + text + " <", bold + yellow) + "\n";      
            } else {
                inside += "  " + text + "  \n";
            }
        }

        string box = buildMazeBox(inside);
        int footerLines = footer == "" ? 0 : countLines(footer) + 2;
        string screen = buildTitle(countLines(box) + 2 + footerLines + linesBelow);
        screen += centerBlock(box, width);
        screen += centerLine(paint("Up/Down or W/S - move   Enter - choose   1-9 - shortcut   Esc - back", muted), width) + "\n";
        if (footer != "") {
            screen += "\n" + centerBlock(footer, width);
        }
        showScreen(screen, linesBelow);

        int key = readKey();
        if (key == arrowUp || key == 'w') {
            selected = (selected - 1 + itemCount) % itemCount;
        } else if (key == arrowDown || key == 's') {
            selected = (selected + 1) % itemCount;
        } else if (key == enterKey) {
            return selected;
        } else if (key == escapeKey || key == 'q') {
            return -1;
        } else if (key >= '1' && key <= '9' && key - '1' < itemCount) {
            return key - '1';
        }
    }
}

void showPage(const string& caption, const string& content) {
    string inside = paint(caption, bold + white) + "\n" + paint(string(caption.length(), '='), dark) + "\n\n" + content;
    string box = buildMazeBox(inside);
    string screen = buildTitle(countLines(box) + 2) + centerBlock(box, getScreenWidth());
    showScreen(screen, 2);
    waitForKey("Press any key to go back...");
}

int readNumberInRange(const string& question, int minValue, int maxValue, int defaultValue) {
    showCursor(true);
    while (true) {
        printQuestion(question + " " + paint("[" + to_string(minValue) + "-" + to_string(maxValue)
                      + ", Enter = " + to_string(defaultValue) + "]", muted) + ": ");
        string text = trim(readLine());
        if (text == "") {
            return defaultValue;
        }

        int number = 0;
        if (!textToInt(text, number)) {
            printError("'" + text.substr(0, 20) + "' is not a whole number. Please try again.");
        } else if (!isInRange(number, minValue, maxValue)) {
            printError("The number must be between " + to_string(minValue) + " and " + to_string(maxValue) + ".");
        } else {
            return number;
        }
    }
}

string readText(const string& question) {
    showCursor(true);
    printQuestion(question + ": ");
    return trim(readLine());
}

string readPassword(const string& question) {
    showCursor(true);
    printQuestion(question + ": ");
    return readHiddenLine();
}

bool askYesNo(const string& question, bool defaultAnswer) {
    showCursor(false);
    printQuestion(question + " " + paint(defaultAnswer ? "[Y/n]" : "[y/N]", muted) + " ");
    while (true) {
        int key = readKey();
        if (key == enterKey) {
            writeText("\n");
            return defaultAnswer;
        }
        if (key == 'y') {
            writeText("yes\n");
            return true;
        }
        if (key == 'n' || key == escapeKey) {
            writeText("no\n");
            return false;
        }
    }
}

void waitForKey(const string& text) {
    showCursor(false);
    writeText("\n" + centerLine(paint(text, muted), getScreenWidth()));
    readKey();
}

void printError(const string& text) {
    printCentered(paint("! " + text, red));
}

void printCentered(const string& text) {
    writeText(centerLine(text, getScreenWidth()) + "\n");
}
