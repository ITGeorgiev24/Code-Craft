#include "CppUnitTest.h"
#include "testHelpers.h"
#include "pathfinder.h"

using namespace std;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

bool areNeighbours(Position first, Position second) {
    int rowDifference = first.row > second.row ? first.row - second.row : second.row - first.row;
    int colDifference = first.col > second.col ? first.col - second.col : second.col - first.col;
    return rowDifference + colDifference == 1;
}

TEST_CLASS(PathfinderTests) {
public:
    TEST_METHOD(walkableSquares) {
        Maze maze;
        buildMaze(maze, sampleMaze, 10);
        Assert::IsTrue(isWalkable(maze, 0, 0, false), L"path");
        Assert::IsTrue(isWalkable(maze, 9, 9, false), L"exit");
        Assert::IsFalse(isWalkable(maze, 1, 1, false), L"wall");
        Assert::IsFalse(isWalkable(maze, 1, 0, false), L"door without doorsAllowed");
        Assert::IsTrue(isWalkable(maze, 1, 0, true), L"door with doorsAllowed");
        Assert::IsFalse(isWalkable(maze, -1, 0, true), L"outside");
        deleteMaze(maze);
    }

    TEST_METHOD(shortestPathOnKnownMaze) {
        Maze maze;
        buildMaze(maze, sampleMaze, 10);
        Assert::AreEqual(18, shortestPathLength(maze, makePosition(0, 0), makePosition(9, 9), false), L"18 steps to the exit");
        Assert::AreEqual(0, shortestPathLength(maze, makePosition(0, 0), makePosition(0, 0), false), L"0 steps to itself");
        Assert::AreEqual(5, shortestPathLength(maze, makePosition(0, 0), makePosition(5, 0), true), L"5 steps through the door");
        Assert::AreEqual(34, shortestPathLength(maze, makePosition(0, 0), makePosition(2, 0), false), L"34 steps without the door");
        Assert::AreEqual(-1, shortestPathLength(maze, makePosition(0, 0), makePosition(4, 4), true), L"a wall cannot be reached");
        deleteMaze(maze);
    }

    TEST_METHOD(pathIsContinuous) {
        Maze maze;
        buildMaze(maze, sampleMaze, 10);
        PositionList path;
        Assert::IsTrue(findShortestPath(maze, makePosition(0, 0), makePosition(9, 9), false, path), L"a way is found");
        Assert::AreEqual(19, path.count, L"18 moves + the start square");
        Assert::IsTrue(samePosition(path.items[0], makePosition(0, 0)), L"starts at the start");
        Assert::IsTrue(samePosition(path.items[path.count - 1], makePosition(9, 9)), L"ends at the exit");
        for (int i = 1; i < path.count; i++) {
            Assert::IsTrue(areNeighbours(path.items[i - 1], path.items[i]), L"every step goes to a neighbour");
        }
        deleteList(path);
        deleteMaze(maze);
    }

    TEST_METHOD(noPathReturnsFalse) {
        const string closedMaze[10] = {
            "....#.....", "....#.....", "....#.....", "....#.....", "....#.....",
            "....#.....", "....#.....", "....#.....", "....#.....", "....#....E",
        };
        Maze maze;
        buildMaze(maze, closedMaze, 10);
        PositionList path;
        Assert::IsFalse(findShortestPath(maze, makePosition(0, 0), makePosition(9, 9), true, path), L"no way through the wall");
        Assert::IsTrue(path.items == nullptr, L"the list stays empty");
        Assert::AreEqual(-1, shortestPathLength(maze, makePosition(0, 0), makePosition(9, 9), true), L"length -1");
        deleteMaze(maze);
    }
};
