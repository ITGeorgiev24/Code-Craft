#include "CppUnitTest.h"
#include "testHelpers.h"
#include "../Logic/gameLogic.h"
#include "../Logic/records.h"

#include <cstdio>
#include <fstream>

using namespace std;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

ScoreEntry makeEntry(const string& name, int score, int moves) {
    ScoreEntry entry;
    entry.playerName = name;
    entry.mazeSize = 15;
    entry.score = score;
    entry.moves = moves;
    return entry;
}

TEST_CLASS(RecordsTests) {
public:
    TEST_METHOD(scoresAreSorted) {
        ScoreTable table;
        createScoreTable(table, 5);
        Assert::AreEqual(1, insertScore(table, makeEntry("B", 500, 50)), L"B first");
        Assert::AreEqual(1, insertScore(table, makeEntry("A", 900, 50)), L"A takes first place");
        Assert::AreEqual(3, insertScore(table, makeEntry("C", 100, 50)), L"C third");
        Assert::AreEqual(2, insertScore(table, makeEntry("D", 700, 50)), L"D second");
        Assert::IsTrue(table.entries[0].playerName == "A" && table.entries[1].playerName == "D" && table.entries[2].playerName == "B" && table.entries[3].playerName == "C", L"order A D B C");
        deleteScoreTable(table);
    }

    TEST_METHOD(equalScoresAreSortedByMoves) {
        ScoreTable table;
        createScoreTable(table, 3);
        insertScore(table, makeEntry("Slow", 500, 90));
        Assert::AreEqual(1, insertScore(table, makeEntry("Fast", 500, 30)), L"fewer moves first");
        deleteScoreTable(table);
    }

    TEST_METHOD(fullTableDropsTheWorst) {
        ScoreTable table;
        createScoreTable(table, 3);
        insertScore(table, makeEntry("A", 300, 50));
        insertScore(table, makeEntry("B", 200, 50));
        insertScore(table, makeEntry("C", 100, 50));
        Assert::AreEqual(-1, insertScore(table, makeEntry("Weak", 50, 50)), L"a weaker result does not enter");
        Assert::AreEqual(2, insertScore(table, makeEntry("Strong", 250, 50)), L"a stronger one enters");
        Assert::IsTrue(table.count == 3 && table.entries[2].playerName == "B", L"C dropped out");
        deleteScoreTable(table);
    }

    TEST_METHOD(wrongResultsInTheFileAreIgnored) {
        string fileName = "testLeaderboard.tmp";
        ofstream file(fileName);
        file << "Valid;20;1;80;1500\n" << "TooSmallMaze;5;1;80;9999\n" << "Negative;20;1;80;-10\n"
             << "Bad!Name;20;1;80;3000\n" << "Second;20;2;90;1200\n";
        file.close();

        ScoreTable table;
        loadLeaderboard(table, fileName);
        Assert::AreEqual(2, table.count, L"only the 2 valid results");
        Assert::IsTrue(table.entries[0].playerName == "Valid" && table.entries[1].playerName == "Second", L"in order");
        deleteLeaderboard(table);
        remove(fileName.c_str());
    }

    TEST_METHOD(winnerIsSaved) {
        string fileName = "testSubmit.tmp";
        remove(fileName.c_str());

        GameState game;
        buildGame(game, sampleMaze, 10);
        game.optimalMoves = 18;
        for (int i = 0; i < 9; i++) {
            movePlayer(game, DIR_RIGHT);
        }
        for (int i = 0; i < 9; i++) {
            movePlayer(game, DIR_DOWN);
        }
        Assert::IsTrue(getStatus(game) == WON, L"the game is won");
        Assert::IsTrue(qualifiesForLeaderboard(calculateScore(game), 18, fileName), L"enters the empty board");

        int place = 0;
        Assert::IsTrue(!submitScore("bad;name", game, place, fileName) && place == -1, L"a wrong name is refused");
        Assert::IsTrue(submitScore("  Winner ", game, place, fileName) && place == 1, L"saved as #1");

        ScoreTable table;
        loadLeaderboard(table, fileName);
        Assert::IsTrue(table.count == 1 && table.entries[0].playerName == "Winner", L"the name without spaces");
        Assert::IsTrue(table.entries[0].moves == 18 && table.entries[0].mazeSize == 10, L"moves and size");
        deleteLeaderboard(table);
        endGame(game);
        remove(fileName.c_str());
    }

    TEST_METHOD(fasterTimeWinsATie) {
        ScoreEntry slow = makeEntry("Slow", 500, 40);
        ScoreEntry fast = makeEntry("Fast", 500, 40);
        slow.seconds = 90;
        fast.seconds = 30;
        Assert::IsTrue(isBetterScore(fast, slow) && !isBetterScore(slow, fast), L"30 s beats 90 s");
        ScoreEntry unknown = makeEntry("Old", 500, 40);         
        Assert::IsTrue(isBetterScore(slow, unknown), L"a known time beats an unknown one");
    }

    TEST_METHOD(zeroScoreNeverEnters) {
        Assert::IsFalse(qualifiesForLeaderboard(0, 10, "noSuchFile.tmp"), L"0 points");
        Assert::IsTrue(qualifiesForLeaderboard(1, 10, "noSuchFile.tmp"), L"1 point on an empty board");
    }
};
