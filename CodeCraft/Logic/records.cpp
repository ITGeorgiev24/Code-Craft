#include "records.h"
#include "fileStorage.h"
#include "gameLogic.h"
#include "validation.h"

using namespace std;

const int maxFileEntries = 200;

bool isValidEntry(const ScoreEntry& entry) {
    return isValidPlayerName(entry.playerName) && isValidMazeSize(entry.mazeSize)
        && entry.moves >= 0 && entry.score >= 0;
}

bool isBetterScore(const ScoreEntry& first, const ScoreEntry& second) {
    if (first.score != second.score) {
        return first.score > second.score;
    }
    return first.moves < second.moves;
}

int insertScore(ScoreTable& table, const ScoreEntry& entry) {
    int place = 0;
    while (place < table.count && !isBetterScore(entry, table.entries[place])) {
        place++;
    }
    if (place >= table.capacity) {
        return -1;
    }

    int last = table.count < table.capacity ? table.count : table.capacity - 1;
    for (int i = last; i > place; i--) {
        table.entries[i] = table.entries[i - 1];
    }
    table.entries[place] = entry;
    if (table.count < table.capacity) {
        table.count++;
    }
    return place + 1;
}

void loadLeaderboard(ScoreTable& table, const string& fileName) {
    createScoreTable(table, leaderboardSize);

    ScoreTable fromFile;
    createScoreTable(fromFile, maxFileEntries);
    loadScores(fileName, fromFile);
    for (int i = 0; i < fromFile.count; i++) {
        if (isValidEntry(fromFile.entries[i])) {
            insertScore(table, fromFile.entries[i]);
        }
    }
    deleteScoreTable(fromFile);
}

void deleteLeaderboard(ScoreTable& table) {
    deleteScoreTable(table);
}

ScoreEntry makeScoreEntry(const string& playerName, const GameState& game) {
    ScoreEntry entry;
    entry.playerName = trim(playerName);
    entry.mazeSize = getMazeSize(game);
    entry.difficulty = getDifficulty(game);
    entry.moves = getPlayer(game).moves;
    entry.score = calculateScore(game);
    return entry;
}

bool qualifiesForLeaderboard(int score, int moves, const string& fileName) {
    if (score <= 0) {
        return false;
    }
    ScoreTable table;
    loadLeaderboard(table, fileName);

    bool qualifies = table.count < table.capacity;
    if (!qualifies) {
        ScoreEntry candidate;
        candidate.score = score;
        candidate.moves = moves;
        qualifies = isBetterScore(candidate, table.entries[table.count - 1]);
    }
    deleteLeaderboard(table);
    return qualifies;
}

bool submitScore(const string& playerName, const GameState& game, int& place, const string& fileName) {
    place = -1;
    if (!isValidPlayerName(playerName)) {
        return false;
    }
    ScoreTable table;
    loadLeaderboard(table, fileName);
    place = insertScore(table, makeScoreEntry(playerName, game));
    bool saved = place > 0 && saveScores(fileName, table);
    deleteLeaderboard(table);
    return saved;
}
