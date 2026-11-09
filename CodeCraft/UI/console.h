#pragma once
#include <string>

using namespace std;

const string background = "\x1b[48;5;17m";                
const string reset      = "\x1b[0m\x1b[48;5;17m";             
const string bold       = "\x1b[1m";
const string red        = "\x1b[91m";
const string green      = "\x1b[92m";
const string yellow     = "\x1b[93m";
const string blue       = "\x1b[94m";
const string magenta    = "\x1b[95m";
const string cyan       = "\x1b[96m";
const string white      = "\x1b[97m";
const string muted      = "\x1b[38;5;146m";                   
const string dark       = "\x1b[38;5;61m";                    
const string orange     = "\x1b[38;5;215m";
const string exitColor = "\x1b[1;92m";                     
const string titleTag  = "\x1b[1;96m";                     

const int arrowUp = 1001;
const int arrowDown = 1002;
const int arrowLeft = 1003;
const int arrowRight = 1004;
const int enterKey = 13;
const int escapeKey = 27;
const int backspaceKey = 8;

void initializeConsole();

void restoreConsole();

int readKey();

string readLine();

string readHiddenLine();

void getConsoleSize(int& columns, int& rows);

void clearScreen();

void moveCursorHome();

void showCursor(bool visible);

void writeText(const string& text);

string paint(const string& text, const string& color);
