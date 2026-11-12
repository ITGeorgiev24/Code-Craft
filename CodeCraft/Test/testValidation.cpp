#include "CppUnitTest.h"
#include "validation.h"

using namespace std;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

TEST_CLASS(ValidationTests) {
public:
    TEST_METHOD(trimRemovesSpaces) {
        Assert::AreEqual(string("15"), trim("  15  "), L"spaces on both sides");
        Assert::AreEqual(string("name"), trim("\t name \r"), L"tabs and \\r");
        Assert::AreEqual(string(""), trim("   "), L"only spaces");
        Assert::AreEqual(string(""), trim(""), L"empty text");
    }

    TEST_METHOD(textToIntAcceptsNumbers) {
        int number = 0;
        Assert::IsTrue(textToInt("42", number) && number == 42, L"42");
        Assert::IsTrue(textToInt("  -7 ", number) && number == -7, L"-7 with spaces");
        Assert::IsTrue(textToInt("999999999", number) && number == 999999999, L"9 digits");
    }

    TEST_METHOD(textToIntRejectsWrongText) {
        int number = 5;
        Assert::IsFalse(textToInt("", number), L"empty");
        Assert::IsFalse(textToInt("abc", number), L"letters");
        Assert::IsFalse(textToInt("12abc", number), L"number + letters");
        Assert::IsFalse(textToInt("1.5", number), L"decimal");
        Assert::IsFalse(textToInt("-", number), L"only a minus");
        Assert::IsFalse(textToInt("1 2", number), L"two numbers");
        Assert::IsFalse(textToInt("12345678901", number), L"too long");
        Assert::AreEqual(5, number, L"the number is not changed after an error");
    }

    TEST_METHOD(mazeSizeMustBeBetween10And50) {
        Assert::IsFalse(isValidMazeSize(9), L"9");
        Assert::IsTrue(isValidMazeSize(10), L"10");
        Assert::IsTrue(isValidMazeSize(27), L"27");
        Assert::IsTrue(isValidMazeSize(50), L"50");
        Assert::IsFalse(isValidMazeSize(51), L"51");
        Assert::IsFalse(isValidMazeSize(-10), L"-10");
    }

    TEST_METHOD(rangeIncludesBothLimits) {
        Assert::IsTrue(isInRange(1, 1, 3), L"lower limit");
        Assert::IsTrue(isInRange(3, 1, 3), L"upper limit");
        Assert::IsFalse(isInRange(0, 1, 3), L"below");
        Assert::IsFalse(isInRange(4, 1, 3), L"above");
    }

    TEST_METHOD(playerNameRules) {
        Assert::IsTrue(isValidPlayerName("Ivan"), L"Ivan");
        Assert::IsTrue(isValidPlayerName("Team_4 - QA.1"), L"letters, digits and signs");
        Assert::IsTrue(isValidPlayerName("  Maria  "), L"spaces around are removed");
        Assert::IsTrue(isValidPlayerName("ABCDEFGHIJKLMNOP"), L"exactly 16 characters");
        Assert::IsFalse(isValidPlayerName(""), L"empty");
        Assert::IsFalse(isValidPlayerName("    "), L"only spaces");
        Assert::IsFalse(isValidPlayerName("ABCDEFGHIJKLMNOPQ"), L"17 characters");
        Assert::IsFalse(isValidPlayerName("bad;name"), L"';' would break the file");
        Assert::IsFalse(isValidPlayerName("hello!"), L"'!'");
    }
};
