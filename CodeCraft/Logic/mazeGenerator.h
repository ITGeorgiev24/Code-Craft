#pragma once
#include "../Data/gameData.h"

using namespace std;

struct DifficultySettings {
    int loopChance;                   
    int baseDoors;                  
    int keyBonus;               
    int tilesPerCoin;            
    int compasses;            
    int scoreMultiplier;        
    bool fogOfWar;                  
};

struct GenerationReport {
    int loops = 0;
    int doors = 0;
    int keys = 0;
    int coins = 0;
    int compasses = 0;
};

DifficultySettings getDifficultySettings(Difficulty difficulty);

bool generateMaze(Maze& maze, int size, Difficulty difficulty, unsigned int seed, GenerationReport& report);

int randomNumber(int minValue, int maxValue);
