#include "loginMenu.h"
#include "../Logic/accounts.h"
#include "console.h"
#include "input.h"
#include "layout.h"

using namespace std;

const int questionLines = 3;

string fieldText(const string& label, const string& value) {
    return padRight(label, 18) + (value == "" ? paint("...", dark) : value);
}

string hiddenText(const string& password) {
    return string(password.length(), '*');
}

string logInForm() {
    string username;
    string password;
    string message;
    int selected = 0;

    while (true) {
        const string items[4] = {
            fieldText("Username:", username),
            fieldText("Password:", hiddenText(password)),
            "Log in",
            "Back",
        };
        int choice = runMenu("LOG IN", items, 4, selected, message, questionLines);

        if (choice == 0) {
            username = readText("Username");
            selected = 1;
        } else if (choice == 1) {
            password = readPassword("Password");
            selected = 2;
        } else if (choice == 2) {
            AccountResult result = logIn(username, password);
            if (result == ACCOUNT_OK) {
                return getSavedUsername(username);
            }
            if (username == "") {
                message = paint("Please type your username first.", red);
                selected = 0;
            } else if (result == ACCOUNT_NOT_FOUND) {
                message = paint("There is no account \"" + username + "\". Go back and choose \"Create account\".", red);
                selected = 0;
            } else {
                message = paint("Wrong password. Please try again.", red);
                password = "";
                selected = 1;
            }
        } else {
            return "";      
        }
    }
}

string createAccountForm() {
    string username;
    string password;
    string repeatedPassword;
    string message;
    int selected = 0;

    string rules = paint("Username: " + to_string(minUsernameLength) + "-" + to_string(maxUsernameLength)
                         + " latin letters, digits, '_', '-', '.'\n"
                         + "Password: " + to_string(minPasswordLength) + "-" + to_string(maxPasswordLength)
                         + " characters, at least 1 digit and 1 capital letter, no spaces", muted);

    while (true) {
        const string items[5] = {
            fieldText("Username:", username),
            fieldText("Password:", hiddenText(password)),
            fieldText("Repeat password:", hiddenText(repeatedPassword)),
            "Create account",
            "Back",
        };
        string footer = message == "" ? rules : rules + "\n" + message;
        int choice = runMenu("CREATE ACCOUNT", items, 5, selected, footer, questionLines);

        if (choice == 0) {
            username = readText("Username");
            message = "";
            selected = 1;
        } else if (choice == 1) {
            password = readPassword("Password");
            message = "";
            selected = 2;
        } else if (choice == 2) {
            repeatedPassword = readPassword("Repeat the password");
            message = "";
            selected = 3;
        } else if (choice == 3) {
            if (!isValidUsername(username)) {
                message = paint("The username does not follow the rules.", red);
                selected = 0;
            } else if (accountExists(username)) {
                message = paint("The username \"" + username + "\" is already taken.", red);
                selected = 0;
            } else if (!isValidPassword(password)) {
                message = paint("The password needs " + to_string(minPasswordLength) + "-" + to_string(maxPasswordLength)
                                + " characters, at least one digit (0-9) and one capital letter (A-Z).", red);
                selected = 1;
            } else if (repeatedPassword != password) {
                message = paint("The two passwords are different.", red);
                repeatedPassword = "";
                selected = 2;
            } else if (createAccount(username, password) != ACCOUNT_OK) {
                message = paint("The account could not be saved.", red);
            } else {
                return username;
            }
        } else {
            return "";      
        }
    }
}

void showWelcome(const string& text) {
    showPage("WELCOME", paint(text, bold + green) + "\n", "Press any key to continue...");
}

string showLoginMenu() {
    const string menuItems[3] = { "Log in", "Create account", "Exit" };

    int selected = 0;
    while (true) {
        int choice = runMenu("WELCOME, TRAVELLER", menuItems, 3, selected, "");
        if (choice >= 0) {
            selected = choice;
        }

        if (choice == 0) {
            string username = logInForm();
            if (username != "") {
                showWelcome("Welcome back, " + username + "!");
                return username;
            }
        } else if (choice == 1) {
            string username = createAccountForm();
            if (username != "") {
                showWelcome("Account created! Welcome, " + username + "!");
                return username;
            }
        } else {
            return "";      
        }
    }
}
