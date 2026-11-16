#pragma once
#include "../Data/gameData.h"

using namespace std;

const int paidHintCost = 25;               
const int hintVisibleSteps = 12;          
const int coinValue = 25;              
const int extraMovePenalty = 3;            
const int nearbyRadius = 3;             
const int fogVision = 2;                   

const int coinsPerReward = 3;
const int rewardPoints = 50;
const int allCoinsBonus = 100;
const int chaserFreezeMs = 5000;
const int chaserSafeDistance = 6;                
const int freeGameChaserWakeMs = 10000;                

enum Direction { DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT };

enum MoveStatus { MOVE_OK, MOVE_WALL, MOVE_LOCKED_DOOR, MOVE_EDGE, MOVE_NOT_PLAYING };

enum RewardType { REWARD_NONE, REWARD_COMPASS, REWARD_KEY, REWARD_GOLD };

struct MoveResult {
    MoveStatus status = MOVE_NOT_PLAYING;
    ItemType itemFound = ITEM_NONE;
    bool openedDoor = false;
    bool reachedExit = false;
    RewardType reward = REWARD_NONE;          
    bool allCoins = false;                     
    bool caught = false;                     
};

enum TimeEvent { TIME_NOTHING, TIME_CHASER_APPEARED, TIME_CHASER_MOVED, TIME_CAUGHT };

enum HintResult { HINT_FREE, HINT_PAID, HINT_NO_PATH, HINT_NOT_PLAYING };

enum NeighborKind { NEAR_OPEN, NEAR_WALL, NEAR_DOOR, NEAR_EDGE, NEAR_EXIT };

struct LocationInfo {
    Position position;
    NeighborKind neighbors[4] = { NEAR_EDGE, NEAR_EDGE, NEAR_EDGE, NEAR_EDGE };       
    int stepsToExit = -1;
    int itemsNearby = 0;
    int doorsNearby = 0;
    bool isStart = false;
};

void initializeRandom();

unsigned int generateRandomSeed();

bool startNewGame(GameState& game, int size, Difficulty difficulty);

bool startGameWithSeed(GameState& game, int size, Difficulty difficulty, unsigned int seed);

bool restartGame(GameState& game);

void enableChaser(GameState& game, int wakeAfterMs, int stepMs);

int chaserStepForDifficulty(Difficulty difficulty);

TimeEvent advanceTime(GameState& game, int deltaMs);

void endGame(GameState& game);

void quitGame(GameState& game);

MoveResult movePlayer(GameState& game, Direction direction);

HintResult requestHint(GameState& game);

int getMazeSize(const GameState& game);
TileType getTileType(const GameState& game, int row, int col);
ItemType getItemAt(const GameState& game, int row, int col);
bool isVisited(const GameState& game, int row, int col);
bool isPlayerAt(const GameState& game, int row, int col);
bool isHintTile(const GameState& game, int row, int col);
bool isHintVisible(const GameState& game);
bool isTileVisible(const GameState& game, int row, int col);         
bool hasFog(const GameState& game);

Player getPlayer(const GameState& game);
GameStatus getStatus(const GameState& game);
Difficulty getDifficulty(const GameState& game);
int getOptimalMoves(const GameState& game);
int getTotalCoins(const GameState& game);
int getHintsUsed(const GameState& game);
int getElapsedSeconds(const GameState& game);

bool hasChaser(const GameState& game);               
bool isChaserActive(const GameState& game);            
bool isChaserAt(const GameState& game, int row, int col);
bool isChaserFrozen(const GameState& game);
int getChaserWakeSeconds(const GameState& game);           
int getChaserDistance(const GameState& game);                

LocationInfo describeLocation(const GameState& game);

int getScoreMultiplier(Difficulty difficulty);

int computeScore(int mazeSize, Difficulty difficulty, int moves, int optimalMoves, int coins, int hintPenalty,
                 int bonusPoints = 0);

int calculateScore(const GameState& game);

int calculateStars(int moves, int optimalMoves);
