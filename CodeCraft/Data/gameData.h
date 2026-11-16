#pragma once
#include <string>

using namespace std;

enum TileType { TILE_WALL, TILE_PATH, TILE_DOOR, TILE_EXIT };

enum ItemType { ITEM_NONE, ITEM_KEY, ITEM_COIN, ITEM_COMPASS };

enum Difficulty { EASY, NORMAL, HARD, EXPERT };

enum GameStatus { NOT_STARTED, PLAYING, WON, QUIT, CAUGHT };

struct Position {
    int row = 0;
    int col = 0;
};

struct Tile {
    TileType type = TILE_WALL;
    ItemType item = ITEM_NONE;
    bool visited = false;        
    bool seen = false;                 
};

struct Maze {
    int size = 0;
    Tile** tiles = nullptr;
};

struct PositionList {
    Position* items = nullptr;
    int count = 0;
    int capacity = 0;
};

struct Player {
    Position position;
    int keys = 0;
    int coins = 0;
    int compasses = 0;
    int moves = 0;
    int doorsOpened = 0;
    int rewards = 0;              
    int bonusPoints = 0;          
};

struct Chaser {
    bool enabled = false;        
    int wakeAfterMs = 0;            
    int stepMs = 0;                
    bool active = false;          
    Position position;
    int stepTimerMs = 0;          
    int frozenMs = 0;               
};

struct GameState {
    Maze maze;
    Player player;
    Difficulty difficulty = NORMAL;
    unsigned int seed = 0;                  
    GameStatus status = NOT_STARTED;
    bool fogOfWar = false;
    int totalCoins = 0;
    int optimalMoves = 0;                 

    PositionList hintPath;               
    int hintStep = 0;                    
    bool hintVisible = false;
    int hintsUsed = 0;
    int hintPenalty = 0;

    int elapsedMs = 0;                     
    Chaser chaser;
};

struct ScoreEntry {
    string playerName;
    int mazeSize = 0;
    Difficulty difficulty = NORMAL;
    int moves = 0;
    int score = 0;
    int seconds = 0;              
};

struct ScoreTable {
    ScoreEntry* entries = nullptr;
    int count = 0;
    int capacity = 0;
};

const int maxLevels = 10;

struct CampaignProgress {
    int unlockedLevels = 1;                   
    int bestStars[maxLevels] = {};          
    int bestScores[maxLevels] = {};
    int bestTimes[maxLevels] = {};            
};

struct Account {
    string username;
    unsigned int passwordHash = 0;               
    CampaignProgress progress;
};

struct AccountList {
    Account* accounts = nullptr;
    int count = 0;
    int capacity = 0;
};

void createMaze(Maze& maze, int size);

void deleteMaze(Maze& maze);

bool isInside(const Maze& maze, int row, int col);

void createList(PositionList& list, int capacity);
void deleteList(PositionList& list);

bool addToList(PositionList& list, Position position);

bool samePosition(Position first, Position second);

void createScoreTable(ScoreTable& table, int capacity);
void deleteScoreTable(ScoreTable& table);

bool addScore(ScoreTable& table, const ScoreEntry& entry);

void createAccountList(AccountList& list, int capacity);
void deleteAccountList(AccountList& list);

bool addAccount(AccountList& list, const Account& account);
