#include "accounts.h"
#include "campaign.h"
#include "fileStorage.h"
#include "validation.h"

using namespace std;


string toLowerText(const string& text) {
    string result = text;
    for (int i = 0; i < (int)result.length(); i++) {
        if (result[i] >= 'A' && result[i] <= 'Z') {
            result[i] = (char)(result[i] - 'A' + 'a');
        }
    }
    return result;
}

bool sameUsername(const string& first, const string& second) {
    return toLowerText(first) == toLowerText(second);
}

void loadAccountList(AccountList& list, const string& fileName) {
    createAccountList(list, maxAccounts);
    loadAccounts(fileName, list);
}

int findAccount(const AccountList& list, const string& username) {
    for (int i = 0; i < list.count; i++) {
        if (sameUsername(list.accounts[i].username, username)) {
            return i;
        }
    }
    return -1;
}


bool isValidUsername(const string& username) {
    int length = (int)username.length();
    if (length < minUsernameLength || length > maxUsernameLength) {
        return false;
    }
    for (int i = 0; i < length; i++) {
        char symbol = username[i];
        bool letter = (symbol >= 'a' && symbol <= 'z') || (symbol >= 'A' && symbol <= 'Z');
        bool digit = symbol >= '0' && symbol <= '9';
        bool sign = symbol == '_' || symbol == '-' || symbol == '.';
        if (!letter && !digit && !sign) {
            return false;
        }
    }
    return true;
}

bool isValidPassword(const string& password) {
    int length = (int)password.length();
    if (length < minPasswordLength || length > maxPasswordLength) {
        return false;
    }
    for (int i = 0; i < length; i++) {
        if (password[i] < 33 || password[i] > 126 || password[i] == ';') {
            return false;
        }
    }
    return true;
}

unsigned int hashPassword(const string& password) {
    unsigned int hash = 5381;
    for (int i = 0; i < (int)password.length(); i++) {
        hash = hash * 33 + (unsigned int)password[i];
    }
    return hash;
}


bool accountExists(const string& username, const string& fileName) {
    AccountList list;
    loadAccountList(list, fileName);
    bool exists = findAccount(list, username) >= 0;
    deleteAccountList(list);
    return exists;
}

AccountResult createAccount(const string& username, const string& password, const string& fileName) {
    if (!isValidUsername(username)) {
        return ACCOUNT_BAD_USERNAME;
    }
    if (!isValidPassword(password)) {
        return ACCOUNT_BAD_PASSWORD;
    }

    AccountList list;
    loadAccountList(list, fileName);
    if (findAccount(list, username) >= 0) {
        deleteAccountList(list);
        return ACCOUNT_NAME_TAKEN;
    }

    Account account;
    account.username = username;
    account.passwordHash = hashPassword(password);
    bool saved = addAccount(list, account) && saveAccounts(fileName, list);
    deleteAccountList(list);
    return saved ? ACCOUNT_OK : ACCOUNT_SAVE_FAILED;
}

AccountResult logIn(const string& username, const string& password, const string& fileName) {
    AccountList list;
    loadAccountList(list, fileName);
    int place = findAccount(list, username);

    AccountResult result = ACCOUNT_OK;
    if (place < 0) {
        result = ACCOUNT_NOT_FOUND;
    }
    else if (list.accounts[place].passwordHash != hashPassword(password)) {
        result = ACCOUNT_WRONG_PASSWORD;
    }
    deleteAccountList(list);
    return result;
}

string getSavedUsername(const string& username, const string& fileName) {
    AccountList list;
    loadAccountList(list, fileName);
    int place = findAccount(list, username);
    string savedName = place >= 0 ? list.accounts[place].username : username;
    deleteAccountList(list);
    return savedName;
}

CampaignProgress loadPlayerProgress(const string& username, const string& fileName) {
    AccountList list;
    loadAccountList(list, fileName);
    int place = findAccount(list, username);

    CampaignProgress progress;  
    if (place >= 0) {
        progress = list.accounts[place].progress;
    }
    deleteAccountList(list);
    fixProgress(progress);
    return progress;
}

bool savePlayerProgress(const string& username, const CampaignProgress& progress, const string& fileName) {
    AccountList list;
    loadAccountList(list, fileName);
    int place = findAccount(list, username);

    bool saved = false;
    if (place >= 0) {
        list.accounts[place].progress = progress;
        saved = saveAccounts(fileName, list);
    }
    deleteAccountList(list);
    return saved;
}
