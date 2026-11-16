#ifndef NOMINMAX
#define NOMINMAX                     
#endif
#define WIN32_LEAN_AND_MEAN         
#include <windows.h>
#include <conio.h>                 

#include "console.h"

#include <iostream>

using namespace std;

void initializeConsole() {
    HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;
    GetConsoleMode(output, &mode);
    SetConsoleMode(output, mode | ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    SetConsoleTitleA("MAZEBOUND");
}

void restoreConsole() {
    writeText("\x1b[0m\x1b[2J\x1b[H\x1b[?25h");        
}

int translateKey(const INPUT_RECORD& record) {
    if (record.EventType != KEY_EVENT || !record.Event.KeyEvent.bKeyDown) {
        return noKey;
    }

    int key = record.Event.KeyEvent.wVirtualKeyCode;
    if (key == VK_UP) return arrowUp;
    if (key == VK_DOWN) return arrowDown;
    if (key == VK_LEFT) return arrowLeft;
    if (key == VK_RIGHT) return arrowRight;
    if (key == VK_RETURN) return enterKey;
    if (key == VK_ESCAPE) return escapeKey;
    if (key == VK_BACK) return backspaceKey;
    if (key == VK_SPACE) return spaceKey;
    if (key >= 'A' && key <= 'Z') return key - 'A' + 'a';
    if (key >= '0' && key <= '9') return key;
    if (key >= VK_NUMPAD0 && key <= VK_NUMPAD9) return '0' + (key - VK_NUMPAD0);
    if (key == VK_OEM_2 || key == VK_F1) return '?';           
    return noKey;
}

int readKey() {
    cout.flush();
    HANDLE input = GetStdHandle(STD_INPUT_HANDLE);
    while (true) {
        INPUT_RECORD record;
        DWORD count = 0;
        ReadConsoleInputW(input, &record, 1, &count);
        int key = count == 0 ? noKey : translateKey(record);
        if (key != noKey) {
            return key;
        }
    }
}

int readKeyTimeout(int milliseconds) {
    cout.flush();
    HANDLE input = GetStdHandle(STD_INPUT_HANDLE);
    ULONGLONG end = GetTickCount64() + (ULONGLONG)milliseconds;
    while (true) {
        ULONGLONG now = GetTickCount64();
        if (now >= end) {
            return noKey;
        }
        if (WaitForSingleObject(input, (DWORD)(end - now)) != WAIT_OBJECT_0) {
            return noKey;
        }
        INPUT_RECORD record;
        DWORD count = 0;
        ReadConsoleInputW(input, &record, 1, &count);
        int key = count == 0 ? noKey : translateKey(record);
        if (key != noKey) {
            return key;
        }
    }
}

void clearKeyBuffer() {
    FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
}

long long currentTimeMs() {
    return (long long)GetTickCount64();
}

string readLine() {
    cout.flush();
    string line;
    getline(cin, line);
    return line;
}

string readHiddenLine() {
    string text;
    while (true) {
        int character = _getch();
        if (character == '\r') {                      
            writeText("\n");
            return text;
        }
        if (character == 27) {                       
            writeText("\n");
            return "";
        }
        if (character == 8) {                           
            if (text.length() > 0) {
                text.erase(text.length() - 1);
                writeText("\b \b");
            }
        } else if (character == 0 || character == 224) {
            _getch();                                      
        } else if (character >= 32 && character <= 126) {
            text += (char)character;
            writeText("*");
        }
    }
}

void getConsoleSize(int& columns, int& rows) {
    CONSOLE_SCREEN_BUFFER_INFO info;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info)) {
        columns = info.srWindow.Right - info.srWindow.Left + 1;
        rows = info.srWindow.Bottom - info.srWindow.Top + 1;
    } else {
        columns = 80;
        rows = 25;
    }
}

void clearScreen() {
    int columns = 0;
    int rows = 0;
    getConsoleSize(columns, rows);
    string screen = reset + "\x1b[2J\x1b[3J";
    for (int row = 1; row <= rows; row++) {
        screen += "\x1b[" + to_string(row) + ";1H" + string(columns, ' ');          
    }
    screen += "\x1b[H";        
    writeText(screen);
}

void moveCursorHome() {
    writeText("\x1b[H");
}

void showCursor(bool visible) {
    writeText(visible ? "\x1b[?25h" : "\x1b[?25l");
}

void writeText(const string& text) {
    cout << text;
    cout.flush();
}

string paint(const string& text, const string& color) {
    return color + text + reset;
}
