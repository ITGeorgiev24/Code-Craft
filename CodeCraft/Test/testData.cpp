#include "CppUnitTest.h"
#include "fileStorage.h"
#include "gameData.h"

#include <cstdio>
#include <fstream>

using namespace std;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

TEST_CLASS(DataTests) {
public:
    TEST_METHOD(createMazeMakesWalls) {
        Maze maze;
        createMaze(maze, 12);
        Assert::AreEqual(12, maze.size, L"size 12");
        Assert::IsTrue(maze.tiles != nullptr, L"memory is created");
        Assert::IsTrue(maze.tiles[0][0].type == TILE_WALL, L"top-left is a wall");
        Assert::IsTrue(maze.tiles[11][11].type == TILE_WALL, L"bottom-right is a wall");
        Assert::IsTrue(maze.tiles[5][7].item == ITEM_NONE, L"no items");
        deleteMaze(maze);
    }

    TEST_METHOD(deleteMazeIsSafeTwice) {
        Maze maze;
        createMaze(maze, 10);
        deleteMaze(maze);
        Assert::IsTrue(maze.tiles == nullptr && maze.size == 0, L"empty after delete");
        deleteMaze(maze);
        Assert::IsTrue(maze.tiles == nullptr, L"still empty");
    }

    TEST_METHOD(createMazeTwiceReplacesMemory) {
        Maze maze;
        createMaze(maze, 10);
        createMaze(maze, 50);
        Assert::AreEqual(50, maze.size, L"new size 50");
        maze.tiles[49][49].type = TILE_EXIT;
        Assert::IsTrue(maze.tiles[49][49].type == TILE_EXIT, L"the last square can be used");
        deleteMaze(maze);
    }

    TEST_METHOD(isInsideChecksAllBorders) {
        Maze maze;
        createMaze(maze, 10);
        Assert::IsTrue(isInside(maze, 0, 0) && isInside(maze, 9, 9), L"corners are inside");
        Assert::IsTrue(!isInside(maze, -1, 0) && !isInside(maze, 0, -1), L"negative = outside");
        Assert::IsTrue(!isInside(maze, 10, 0) && !isInside(maze, 0, 10), L"10 = outside");
        deleteMaze(maze);
        Assert::IsFalse(isInside(maze, 0, 0), L"deleted maze has no squares");
    }

    TEST_METHOD(positionListRespectsCapacity) {
        PositionList list;
        createList(list, 2);
        Position position;
        position.row = 1;
        position.col = 2;
        Assert::IsTrue(addToList(list, position), L"first");
        Assert::IsTrue(addToList(list, position), L"second");
        Assert::IsFalse(addToList(list, position), L"third does not fit");
        Assert::AreEqual(2, list.count, L"count is 2");
        deleteList(list);
        Assert::IsTrue(list.items == nullptr, L"deleted");
    }

    TEST_METHOD(scoreFileRoundTrip) {
        string fileName = "testScores.tmp";
        ScoreTable table;
        createScoreTable(table, 5);
        ScoreEntry entry;
        entry.playerName = "Tester";
        entry.mazeSize = 20;
        entry.difficulty = HARD;
        entry.moves = 99;
        entry.score = 2000;
        addScore(table, entry);
        Assert::IsTrue(saveScores(fileName, table), L"saved");

        ScoreTable loaded;
        createScoreTable(loaded, 5);
        Assert::IsTrue(loadScores(fileName, loaded), L"loaded");
        Assert::AreEqual(1, loaded.count, L"one result");
        Assert::AreEqual(string("Tester"), loaded.entries[0].playerName, L"name");
        Assert::IsTrue(loaded.entries[0].mazeSize == 20 && loaded.entries[0].difficulty == HARD, L"size and difficulty");
        Assert::IsTrue(loaded.entries[0].moves == 99 && loaded.entries[0].score == 2000, L"moves and score");
        deleteScoreTable(table);
        deleteScoreTable(loaded);
        remove(fileName.c_str());
    }

    TEST_METHOD(scoreFileSkipsWrongLines) {
        string fileName = "testBadScores.tmp";
        ofstream file(fileName);
        file << "Good;10;1;20;500\n" << "garbage line\n" << "Bad;ten;1;20;500\n" << "Short;10;1\n"
             << "WrongLevel;10;9;20;500\n" << "Second;12;0;40;700\n";
        file.close();

        ScoreTable table;
        createScoreTable(table, 10);
        loadScores(fileName, table);
        Assert::AreEqual(2, table.count, L"only the 2 good lines");
        Assert::IsTrue(table.entries[1].playerName == "Second" && table.entries[1].score == 700, L"second good line");
        deleteScoreTable(table);
        remove(fileName.c_str());
    }

    TEST_METHOD(missingFileReturnsFalse) {
        ScoreTable table;
        createScoreTable(table, 3);
        Assert::IsFalse(loadScores("thisFileDoesNotExist.tmp", table), L"no file");
        Assert::AreEqual(0, table.count, L"table is empty");
        deleteScoreTable(table);
    }

    TEST_METHOD(accountsFileRoundTrip) {
        string fileName = "testAccountsFile.tmp";
        AccountList list;
        createAccountList(list, 5);
        Account account;
        account.username = "Maria";
        account.passwordHash = 4000000000u;
        account.progress.unlockedLevels = 4;
        account.progress.bestStars[0] = 3;
        account.progress.bestScores[0] = 450;
        account.progress.bestStars[2] = 1;
        account.progress.bestScores[2] = 900;
        addAccount(list, account);
        Assert::IsTrue(saveAccounts(fileName, list), L"saved");

        AccountList loaded;
        createAccountList(loaded, 5);
        Assert::IsTrue(loadAccounts(fileName, loaded), L"loaded");
        Assert::IsTrue(loaded.count == 1 && loaded.accounts[0].username == "Maria", L"the account");
        Assert::AreEqual(4000000000u, loaded.accounts[0].passwordHash, L"the password hash");
        CampaignProgress progress = loaded.accounts[0].progress;
        Assert::AreEqual(4, progress.unlockedLevels, L"unlocked levels");
        Assert::IsTrue(progress.bestStars[0] == 3 && progress.bestScores[0] == 450, L"level 1");
        Assert::AreEqual(0, progress.bestStars[1], L"level 2 not completed");
        Assert::IsTrue(progress.bestStars[2] == 1 && progress.bestScores[2] == 900, L"level 3");
        deleteAccountList(list);
        deleteAccountList(loaded);
        remove(fileName.c_str());
    }

    TEST_METHOD(accountsFileSkipsWrongLines) {
        string fileName = "testBadAccounts.tmp";
        ofstream file(fileName);
        file << "Good;123;1;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0\n"
             << "Short;123;1\n"
             << "Letters;abc;1;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0\n"
             << ";123;1;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0;0\n";
        file.close();

        AccountList list;
        createAccountList(list, 10);
        loadAccounts(fileName, list);
        Assert::IsTrue(list.count == 1 && list.accounts[0].username == "Good", L"only the good line");
        deleteAccountList(list);
        remove(fileName.c_str());
    }
};
