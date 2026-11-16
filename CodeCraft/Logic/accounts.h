#pragma once
#include "../Data/gameData.h"

#include <string>

using namespace std;

const string accountsFile = "mazeboundAccounts.txt";
const int maxAccounts = 500;
const int minUsernameLength = 3;
const int maxUsernameLength = 16;
const int minPasswordLength = 4;
const int maxPasswordLength = 20;

enum AccountResult {
    ACCOUNT_OK,
    ACCOUNT_BAD_USERNAME,           
    ACCOUNT_BAD_PASSWORD,           
    ACCOUNT_NAME_TAKEN,              
    ACCOUNT_NOT_FOUND,                
    ACCOUNT_WRONG_PASSWORD,
    ACCOUNT_SAVE_FAILED              
};

bool isValidUsername(const string& username);

bool isValidPassword(const string& password);

unsigned int hashPassword(const string& password);

bool accountExists(const string& username, const string& fileName = accountsFile);

AccountResult createAccount(const string& username, const string& password, const string& fileName = accountsFile);

AccountResult logIn(const string& username, const string& password, const string& fileName = accountsFile);

string getSavedUsername(const string& username, const string& fileName = accountsFile);

CampaignProgress loadPlayerProgress(const string& username, const string& fileName = accountsFile);

bool savePlayerProgress(const string& username, const CampaignProgress& progress, const string& fileName = accountsFile);
