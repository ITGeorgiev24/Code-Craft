#include "input.h"
#include "artwork.h"
#include "console.h"
#include "layout.h"
#include "renderer.h"
#include "../Logic/validation.h"

using namespace std;

const int answerWidth = 10;

const int animationStep = 150;

const int sideGap = 4;

void printQuestion(const string& question) {
    writeText(leftMargin(visibleLength(question) + answerWidth, getScreenWidth()) + question);
}

string torchPicture(int frame, int ) {
    return torchArt(frame);
}

string otherTorchPicture(int frame, int ) {
    return torchArt(frame + 2);
}

string buildMenuBody(const string& inside, const MenuLook* look, int frame, int selected, int width) {
    MenuArt leftArt = (look != nullptr && look->left != nullptr) ? look->left : torchPicture;
    MenuArt rightArt = (look != nullptr && look->right != nullptr) ? look->right : otherTorchPicture;
    return withSideArt(buildMazeBox(inside), leftArt(frame, selected), rightArt(frame, selected), sideGap, width);
}

int runMenu(const string& caption, const string items[], int itemCount, int selected, const string& footer,
            int linesBelow, MenuLook* look) {
    int itemWidth = 0;
    for (int i = 0; i < itemCount; i++) {
        if (visibleLength(items[i]) > itemWidth) {
            itemWidth = visibleLength(items[i]);
        }
    }
    if (selected < 0 || selected >= itemCount) {
        selected = 0;
    }
    bool sideKeys = look != nullptr && look->sideKeys;
    bool digitKeys = look != nullptr && look->digitKeys;
    if (look != nullptr) {
        look->sideMove = 0;
        look->typedKey = 0;
    }

    showCursor(false);
    clearKeyBuffer();
    int frame = 0;
    while (true) {
        int width = getScreenWidth();

        string inside = paint(caption, bold + white) + "\n" + paint(string(caption.length(), '='), dark) + "\n\n";
        for (int i = 0; i < itemCount; i++) {
            string text = padRight(items[i], itemWidth);
            if (i == selected) {
                bool wide = (frame / 3) % 2 == 0;
                string color = wide ? bold + yellow : bold + orange;
                inside += paint(wide ? ">> " : " > ", color) + paint(text, bold + yellow)
                        + paint(wide ? " <<" : " < ", color) + "\n";
            } else {
                inside += "   " + paint(text, "\x1b[38;5;252m") + "   \n";
            }
        }

        string body = buildMenuBody(inside, look, frame, selected, width);
        string below = (look != nullptr && look->below != nullptr) ? look->below(frame, selected) : "";
        string keys = "Up/Down - move   Enter - select   Esc - back";
        if (digitKeys) {
            keys = "Up/Down - move   Left/Right or 0-9 - size   Enter - select   Esc - back";
        } else if (sideKeys) {
            keys = "Up/Down - move   Left/Right - change   Enter - select   Esc - back";
        }

        int extraLines = 2 + (below == "" ? 0 : blockHeight(below) + 1) + (footer == "" ? 0 : blockHeight(footer) + 1);
        string screen = buildTitle(blockHeight(body) + extraLines + linesBelow, frame);
        screen += centerBlock(body, width);
        if (below != "") {
            screen += centerBlock(below, width);
        }
        screen += "\n" + centerLine(paint(keys, muted), width) + "\n";
        if (footer != "") {
            screen += "\n" + centerBlock(footer, width);
        }
        showScreen(screen, linesBelow);

        int key = readKeyTimeout(animationStep);
        if (key == noKey) {
            frame++;            
        } else if (key == arrowUp) {
            selected = (selected - 1 + itemCount) % itemCount;
        } else if (key == arrowDown) {
            selected = (selected + 1) % itemCount;
        } else if (key == enterKey) {
            if (look != nullptr) {
                look->sideMove = 0;
            }
            return selected;
        } else if (sideKeys && (key == arrowLeft || key == arrowRight)) {
            look->sideMove = key == arrowLeft ? -1 : 1;
            return selected;
        } else if (digitKeys && ((key >= '0' && key <= '9') || key == backspaceKey)) {
            look->typedKey = key;
            return selected;
        } else if (key == escapeKey) {
            return -1;
        }
    }
}

bool askConfirm(const string& question, const string& yesText, const string& noText) {
    const string items[2] = { yesText, noText };
    return runMenu(question, items, 2, 1, "") == 0;
}

void showPage(const string& caption, const string& content, const string& prompt, MenuLook* look) {
    string inside = paint(caption, bold + white) + "\n" + paint(string(caption.length(), '='), dark) + "\n\n" + content;
    showCursor(false);
    clearKeyBuffer();
    int frame = 0;
    while (true) {
        int width = getScreenWidth();
        string body = buildMenuBody(inside, look, frame, 0, width);
        string screen = buildTitle(blockHeight(body) + 2, frame) + centerBlock(body, width);
        screen += "\n" + centerLine(paint(prompt, frame % 6 < 4 ? muted : dark), width) + "\n";
        showScreen(screen);
        if (readKeyTimeout(animationStep) != noKey) {
            return;
        }
        frame++;
    }
}

string readText(const string& question) {
    showCursor(true);
    printQuestion(question + ": ");
    string text = trim(readLine());
    showCursor(false);
    return text;
}

string readPassword(const string& question) {
    showCursor(true);
    printQuestion(question + ": ");
    string text = readHiddenLine();
    showCursor(false);
    return text;
}

void waitForKey(const string& text) {
    showCursor(false);
    clearKeyBuffer();
    writeText("\n" + centerLine(paint(text, muted), getScreenWidth()));
    readKey();
}

void printError(const string& text) {
    printCentered(paint("! " + text, red));
}

void printCentered(const string& text) {
    writeText(centerLine(text, getScreenWidth()) + "\n");
}
