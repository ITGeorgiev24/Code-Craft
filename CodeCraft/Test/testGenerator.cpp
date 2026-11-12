#include "CppUnitTest.h"
#include "../Logic/mazeGenerator.h"
#include "../Logic/pathfinder.h"
#include "testHelpers.h"

using namespace std;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

const Difficulty allDifficulties[4] = { EASY, NORMAL, HARD, EXPERT };

int countTiles(const Maze& maze, TileType type) {
    int count = 0;
    for (int row = 0; row < maze.size; row++) {
        for (int col = 0; col < maze.size; col++) {
            if (maze.tiles[row][col].type == type) {
                count++;
            }
        }
    }
    return count;
}

int countItems(const Maze& maze, ItemType item) {
    int count = 0;
    for (int row = 0; row < maze.size; row++) {
        for (int col = 0; col < maze.size; col++) {
            if (maze.tiles[row][col].item == item) {
                count++;
            }
        }
    }
    return count;
}

bool sameMaze(const Maze& first, const Maze& second) {
    for (int row = 0; row < first.size; row++) {
        for (int col = 0; col < first.size; col++) {
            if (first.tiles[row][col].type != second.tiles[row][col].type
                || first.tiles[row][col].item != second.tiles[row][col].item) {
                return false;
            }
        }
    }
    return true;
}

TEST_CLASS(GeneratorTests) {
public:
    TEST_METHOD(difficultiesGetHarder) {
        DifficultySettings easy = getDifficultySettings(EASY);
        DifficultySettings normal = getDifficultySettings(NORMAL);
        DifficultySettings hard = getDifficultySettings(HARD);
        DifficultySettings expert = getDifficultySettings(EXPERT);
        Assert::IsTrue(easy.loopChance > normal.loopChance && normal.loopChance > hard.loopChance, L"fewer loops");
        Assert::IsTrue(easy.keyBonus > normal.keyBonus && normal.keyBonus > hard.keyBonus, L"fewer keys");
        Assert::IsTrue(easy.scoreMultiplier < normal.scoreMultiplier && normal.scoreMultiplier < hard.scoreMultiplier && hard.scoreMultiplier < expert.scoreMultiplier, L"higher multiplier");
        Assert::IsTrue(!easy.fogOfWar && !normal.fogOfWar && !hard.fogOfWar && expert.fogOfWar, L"fog only on Expert");
    }

    TEST_METHOD(everySizeAndDifficultyIsSolvable) {
        for (int size = 10; size <= 50; size++) {
            for (int difficultyIndex = 0; difficultyIndex < 4; difficultyIndex++) {
                Maze maze;
                GenerationReport report;
                bool created = generateMaze(maze, size, allDifficulties[difficultyIndex], (unsigned int)(size * 7919 + difficultyIndex), report);
                Assert::IsTrue(created, L"maze created");
                Assert::IsTrue(maze.tiles[0][0].type == TILE_PATH, L"start is a path");
                Assert::IsTrue(maze.tiles[size - 1][size - 1].type == TILE_EXIT, L"exit in the corner");
                Assert::AreEqual(1, countTiles(maze, TILE_EXIT), L"exactly one exit");
                Assert::IsTrue(shortestPathLength(maze, makePosition(0, 0), makePosition(size - 1, size - 1), false) > 0, (L"size " + to_wstring(size) + L": the exit can be reached").c_str());
                deleteMaze(maze);
            }
        }
    }

    TEST_METHOD(wrongSizesAreRejected) {
        Maze maze;
        GenerationReport report;
        Assert::IsFalse(generateMaze(maze, 9, NORMAL, 1, report), L"9");
        Assert::IsFalse(generateMaze(maze, 51, NORMAL, 1, report), L"51");
        Assert::IsTrue(maze.tiles == nullptr, L"no memory used");
    }

    TEST_METHOD(sameSeedGivesSameMaze) {
        Maze first;
        Maze second;
        GenerationReport report;
        generateMaze(first, 31, NORMAL, 2026, report);
        generateMaze(second, 31, NORMAL, 2026, report);
        Assert::IsTrue(sameMaze(first, second), L"same seed - same maze (daily challenge, replay)");
        deleteMaze(first);
        deleteMaze(second);
    }

    TEST_METHOD(differentSeedsGiveDifferentMazes) {
        Maze first;
        Maze second;
        GenerationReport report;
        generateMaze(first, 25, NORMAL, 1, report);
        generateMaze(second, 25, NORMAL, 2, report);
        Assert::IsFalse(sameMaze(first, second), L"different seeds - different mazes");
        deleteMaze(first);
        deleteMaze(second);
    }

    TEST_METHOD(reportMatchesTheMaze) {
        for (int difficultyIndex = 0; difficultyIndex < 4; difficultyIndex++) {
            Maze maze;
            GenerationReport report;
            generateMaze(maze, 40, allDifficulties[difficultyIndex], 77, report);
            Assert::IsTrue(countTiles(maze, TILE_DOOR) == report.doors, L"doors");
            Assert::IsTrue(countItems(maze, ITEM_KEY) == report.keys, L"keys");
            Assert::IsTrue(countItems(maze, ITEM_COIN) == report.coins, L"coins");
            Assert::IsTrue(countItems(maze, ITEM_COMPASS) == report.compasses, L"compasses");
            Assert::IsTrue(report.coins > 0, L"there are coins");
            Assert::IsTrue((report.doors == 0) == (report.keys == 0), L"keys only together with doors");
            deleteMaze(maze);
        }
    }

    TEST_METHOD(bigMazesHaveDoors) {
        int withDoors = 0;
        for (unsigned int seed = 1; seed <= 20; seed++) {
            Maze maze;
            GenerationReport report;
            generateMaze(maze, 45, NORMAL, seed, report);
            if (report.doors > 0) {
                withDoors++;
            }
            deleteMaze(maze);
        }
        Assert::IsTrue(withDoors >= 18, L"almost every big maze has secret doors");
    }

    TEST_METHOD(doorsAreShortcuts) {
        Maze maze;
        GenerationReport report;
        generateMaze(maze, 49, NORMAL, 4242, report);
        for (int row = 1; row < maze.size - 1; row++) {
            for (int col = 1; col < maze.size - 1; col++) {
                if (maze.tiles[row][col].type != TILE_DOOR) {
                    continue;
                }
                bool leftRight = maze.tiles[row][col - 1].type != TILE_WALL && maze.tiles[row][col + 1].type != TILE_WALL;
                bool upDown = maze.tiles[row - 1][col].type != TILE_WALL && maze.tiles[row + 1][col].type != TILE_WALL;
                Assert::IsTrue(leftRight || upDown, L"a door connects two corridors");
            }
        }
        int withoutDoors = shortestPathLength(maze, makePosition(0, 0), makePosition(48, 48), false);
        int withDoors = shortestPathLength(maze, makePosition(0, 0), makePosition(48, 48), true);
        Assert::IsTrue(withDoors <= withoutDoors, L"doors never make the way longer");
        deleteMaze(maze);
    }

    TEST_METHOD(harderLevelsHaveFewerLoops) {
        int easyLoops = 0;
        int hardLoops = 0;
        for (unsigned int seed = 1; seed <= 5; seed++) {
            Maze maze;
            GenerationReport report;
            generateMaze(maze, 41, EASY, seed, report);
            easyLoops += report.loops;
            generateMaze(maze, 41, HARD, seed, report);
            hardLoops += report.loops;
            deleteMaze(maze);
        }
        Assert::IsTrue(easyLoops > hardLoops, L"Easy has more loops");
        Assert::AreEqual(0, hardLoops, L"Hard has no loops");
    }
};
