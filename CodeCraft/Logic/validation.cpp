#include "validation.h"

using namespace std;

string trim(const string& text) {
    int first = 0;
    int last = (int)text.length() - 1;
    while (first <= last && (text[first] == ' ' || text[first] == '\t' || text[first] == '\r')) {
        first++;
    }
    while (last >= first && (text[last] == ' ' || text[last] == '\t' || text[last] == '\r')) {
        last--;
    }
    return text.substr(first, last - first + 1);
}

bool textToInt(const string& text, int& number) {
    string cleaned = trim(text);
    int start = 0;
    bool negative = false;
    if (cleaned.length() > 0 && cleaned[0] == '-') {
        negative = true;
        start = 1;
    }

    int digits = (int)cleaned.length() - start;
    if (digits < 1 || digits > 9) {          
        return false;
    }

    int result = 0;
    for (int i = start; i < (int)cleaned.length(); i++) {
        if (cleaned[i] < '0' || cleaned[i] > '9') {
            return false;
        }
        result = result * 10 + (cleaned[i] - '0');
    }
    number = negative ? -result : result;
    return true;
}

bool isInRange(int value, int minValue, int maxValue) {
    return value >= minValue && value <= maxValue;
}

bool isValidMazeSize(int size) {
    return isInRange(size, minMazeSize, maxMazeSize);
}

bool isValidPlayerName(const string& name) {
    string cleaned = trim(name);
    if (cleaned.length() == 0 || (int)cleaned.length() > maxNameLength) {
        return false;
    }
    for (int i = 0; i < (int)cleaned.length(); i++) {
        char symbol = cleaned[i];
        bool letter = (symbol >= 'a' && symbol <= 'z') || (symbol >= 'A' && symbol <= 'Z');
        bool digit = symbol >= '0' && symbol <= '9';
        bool sign = symbol == ' ' || symbol == '_' || symbol == '-' || symbol == '.';
        if (!letter && !digit && !sign) {
            return false;
        }
    }
    return true;
}
