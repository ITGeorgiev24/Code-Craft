#include "CppUnitTest.h"
#include "accounts.h"
#include "campaign.h"

#include <cstdio>
#include <fstream>

using namespace std;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

const string testAccountsFile = "testAccounts.tmp";

TEST_CLASS(AccountsTests) {
public:
    TEST_METHOD(usernameRules) {
        Assert::IsTrue(isValidUsername("Ivan"), L"Ivan");
        Assert::IsTrue(isValidUsername("team_4.qa-1"), L"letters, digits, '_', '.', '-'");
        Assert::IsTrue(isValidUsername("abc"), L"3 characters");
        Assert::IsTrue(isValidUsername("ABCDEFGHIJKLMNOP"), L"16 characters");
        Assert::IsFalse(isValidUsername("ab"), L"2 characters");
        Assert::IsFalse(isValidUsername("ABCDEFGHIJKLMNOPQ"), L"17 characters");
        Assert::IsFalse(isValidUsername("Ivan Petrov"), L"a space");
        Assert::IsFalse(isValidUsername("bad;name"), L"';'");
        Assert::IsFalse(isValidUsername(""), L"empty");
    }

    TEST_METHOD(passwordRules) {
        Assert::IsTrue(isValidPassword("1234"), L"4 characters");
        Assert::IsTrue(isValidPassword("Secret!2026"), L"letters, digits and signs");
        Assert::IsFalse(isValidPassword("123"), L"3 characters");
        Assert::IsFalse(isValidPassword("123456789012345678901"), L"21 characters");
        Assert::IsFalse(isValidPassword("my pass"), L"a space");
        Assert::IsFalse(isValidPassword("pass;word"), L"';'");
    }

    TEST_METHOD(sameHashForSamePassword) {
        Assert::IsTrue(hashPassword("secret") == hashPassword("secret"), L"the same password - the same number");
        Assert::IsTrue(hashPassword("secret") != hashPassword("Secret"), L"big letters make a difference");
        Assert::IsTrue(hashPassword("secret") != hashPassword("secret1"), L"different passwords - different numbers");
    }

    TEST_METHOD(createAndLogIn) {
        remove(testAccountsFile.c_str());
        Assert::IsFalse(accountExists("Ivan", testAccountsFile), L"no account at the start");
        Assert::IsTrue(createAccount("Ivan", "secret1", testAccountsFile) == ACCOUNT_OK, L"account created");
        Assert::IsTrue(accountExists("Ivan", testAccountsFile), L"the account exists");
        Assert::IsTrue(logIn("Ivan", "secret1", testAccountsFile) == ACCOUNT_OK, L"correct password");
        Assert::IsTrue(logIn("ivan", "secret1", testAccountsFile) == ACCOUNT_OK, L"small letters in the username are OK");
        Assert::AreEqual(string("Ivan"), getSavedUsername("ivan", testAccountsFile), L"the name as it was created");
        Assert::IsTrue(logIn("Ivan", "wrong", testAccountsFile) == ACCOUNT_WRONG_PASSWORD, L"wrong password");
        Assert::IsTrue(logIn("Maria", "secret1", testAccountsFile) == ACCOUNT_NOT_FOUND, L"no such account");
        remove(testAccountsFile.c_str());
    }

    TEST_METHOD(wrongAccountsAreRefused) {
        remove(testAccountsFile.c_str());
        Assert::IsTrue(createAccount("ab", "secret1", testAccountsFile) == ACCOUNT_BAD_USERNAME, L"short username");
        Assert::IsTrue(createAccount("Ivan", "12", testAccountsFile) == ACCOUNT_BAD_PASSWORD, L"short password");
        createAccount("Ivan", "secret1", testAccountsFile);
        Assert::IsTrue(createAccount("IVAN", "other1", testAccountsFile) == ACCOUNT_NAME_TAKEN, L"the name is taken");
        remove(testAccountsFile.c_str());
    }

    TEST_METHOD(passwordIsNotSavedAsText) {
        remove(testAccountsFile.c_str());
        createAccount("Ivan", "mySecret", testAccountsFile);
        ifstream file(testAccountsFile);
        string line;
        getline(file, line);
        file.close();
        Assert::IsTrue(line.find("Ivan") == 0, L"the account is in the file");
        Assert::IsTrue(line.find("mySecret") == string::npos, L"the password text is not in the file");
        remove(testAccountsFile.c_str());
    }

    TEST_METHOD(everyPlayerHasHisOwnProgress) {
        remove(testAccountsFile.c_str());
        createAccount("Ivan", "secret1", testAccountsFile);
        createAccount("Maria", "secret2", testAccountsFile);

        CampaignProgress progress = loadPlayerProgress("Ivan", testAccountsFile);
        Assert::AreEqual(1, progress.unlockedLevels, L"a new player has only level 1");
        saveLevelResult(progress, 1, 3, 500);
        Assert::IsTrue(savePlayerProgress("Ivan", progress, testAccountsFile), L"saved");

        CampaignProgress ivan = loadPlayerProgress("Ivan", testAccountsFile);
        CampaignProgress maria = loadPlayerProgress("Maria", testAccountsFile);
        Assert::IsTrue(ivan.unlockedLevels == 2 && ivan.bestStars[0] == 3 && ivan.bestScores[0] == 500, L"Ivan's progress");
        Assert::IsTrue(maria.unlockedLevels == 1 && maria.bestStars[0] == 0, L"Maria's progress did not change");
        Assert::IsTrue(logIn("Ivan", "secret1", testAccountsFile) == ACCOUNT_OK, L"the password still works");
        Assert::IsFalse(savePlayerProgress("Nobody", progress, testAccountsFile), L"no progress for a missing account");
        remove(testAccountsFile.c_str());
    }
};
