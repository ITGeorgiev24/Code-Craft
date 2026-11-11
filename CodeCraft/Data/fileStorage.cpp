
#include "fileStorage.h"

#include <fstream>
#include <sstream>

using namespace std;

bool readNumber(const string& text, int& number) {
    stringstream stream(text);
    if (!(stream >> number)) {
        return false;
    }
    char extra;
    return !(stream >> extra);
}

bool loadScores(const string& fileName, ScoreTable& table) {
    ifstream file(fileName);
    if (!file.is_open()) {
        return false;
    }

    string line;
    while (table.count < table.capacity && getline(file, line)) {
        
        stringstream lineStream(line);
        string parts[5];
        int partCount = 0;
        while (partCount < 5 && getline(lineStream, parts[partCount], ';')) {
            partCount++;
        }
        if (partCount != 5) {
            continue;   
        }

        ScoreEntry entry;
        int difficulty = 0;
        entry.playerName = parts[0];
        if (readNumber(parts[1], entry.mazeSize) && readNumber(parts[2], difficulty)
            && readNumber(parts[3], entry.moves) && readNumber(parts[4], entry.score)
            && difficulty >= EASY && difficulty <= EXPERT) {
            entry.difficulty = (Difficulty)difficulty;
            addScore(table, entry);
        }
    }
    return true;
}

bool saveScores(const string& fileName, const ScoreTable& table) {
    ofstream file(fileName);
    if (!file.is_open()) {
        return false;
    }
    for (int i = 0; i < table.count; i++) {
        const ScoreEntry& entry = table.entries[i];
        file << entry.playerName << ';' << entry.mazeSize << ';' << (int)entry.difficulty << ';'
             << entry.moves << ';' << entry.score << '\n';
    }
    return true;
}

bool loadAccounts(const string& fileName, AccountList& list) {
    ifstream file(fileName);
    if (!file.is_open()) {
        return false;
    }

    const int partCount = 3 + 2 * maxLevels;  
    string line;
    while (list.count < list.capacity && getline(file, line)) {
        stringstream lineStream(line);
        string parts[partCount];
        int found = 0;
        while (found < partCount && getline(lineStream, parts[found], ';')) {
            found++;
        }
        if (found != partCount || parts[0] == "") {
            continue;  
        }

        Account account;
        account.username = parts[0];
        int hash = 0;
        bool correct = readNumber(parts[1], hash) && readNumber(parts[2], account.progress.unlockedLevels);
        for (int level = 0; level < maxLevels; level++) {
            correct = correct && readNumber(parts[3 + level], account.progress.bestStars[level]);
            correct = correct && readNumber(parts[3 + maxLevels + level], account.progress.bestScores[level]);
        }
        if (correct) {
            account.passwordHash = (unsigned int)hash;
            addAccount(list, account);
        }
    }
    return true;
}

bool saveAccounts(const string& fileName, const AccountList& list) {
    ofstream file(fileName);
    if (!file.is_open()) {
        return false;
    }
    for (int i = 0; i < list.count; i++) {
        const Account& account = list.accounts[i];
        file << account.username << ';' << (int)account.passwordHash << ';' << account.progress.unlockedLevels;
        for (int level = 0; level < maxLevels; level++) {
            file << ';' << account.progress.bestStars[level];
        }
        for (int level = 0; level < maxLevels; level++) {
            file << ';' << account.progress.bestScores[level];
        }
        file << '\n';
    }
    return true;
}
