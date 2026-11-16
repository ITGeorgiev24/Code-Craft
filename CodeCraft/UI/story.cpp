#include "story.h"
#include "artwork.h"
#include "../Logic/campaign.h"
#include "console.h"
#include "layout.h"
#include "renderer.h"

using namespace std;

string levelName(int level) {
    const string levelNames[levelCount] = {
        "The Old Lighthouse", "The Storm Ship", "Whispering Jungle", "The Golden Temple",
        "The Fire Mountain", "The Frozen Peaks", "The Haunted Keep", "The Sunken City",
        "The Sky Bridge", "The Moon Labyrinth",
    };
    if (!isValidLevel(level)) {
        return "Unknown";
    }
    return levelNames[level - 1];
}

string chapterIntro(int level) {
    const string intros[levelCount] = {
        "A stormy night. Grandpa Elias, the keeper of the\n"
        "Old Lighthouse, has disappeared.\n"
        "On his desk you find a torn map and a short note:\n"
        "\n"
        "  'The Star Compass is broken. Its ten shards are\n"
        "   hidden in the mazes of the world. Find them\n"
        "   before the Shadow Warden does.        - E.'\n"
        "\n"
        "Under the lighthouse a stone door creaks open...",

        "You sail on the old ship 'Albatross' towards the\n"
        "Labyrinth Isles. At midnight a monster storm hits.\n"
        "Lightning splits the mast and the sea pours in.\n"
        "\n"
        "The corridors below deck have turned into a maze.\n"
        "Find the way up before the ship goes down!",

        "You wake up on a hot beach. Behind it grows a jungle\n"
        "so thick that its paths form a living maze.\n"
        "\n"
        "A red parrot lands on your shoulder and croaks:\n"
        "  'Elias! Elias was here! Rrraak!'\n"
        "\n"
        "Maybe it knows the way...",

        "In the heart of the jungle stands a temple of gold.\n"
        "Its halls are full of coins - and full of traps.\n"
        "\n"
        "Grandpa's map says: 'The fourth shard sleeps where\n"
        "the gold shines brightest.'\n"
        "\n"
        "But something else guards the gold: a tall black\n"
        "shadow with burning red eyes - the Shadow Warden!\n"
        "It wakes up soon after you enter. Every 3 coins\n"
        "open a treasure that freezes it for a moment.",

        "Inside grandpa's lost hat you find another note:\n"
        "  'The next shard is in the Fire Mountain.'\n"
        "\n"
        "The volcano rumbles. Rivers of lava glow between\n"
        "black walls of rock and the ground shakes.\n"
        "Hurry - before the mountain wakes up!",

        "From fire to ice. The sixth shard lies in a cave\n"
        "high up in the Frozen Peaks.\n"
        "\n"
        "The ice walls are clear like glass... and behind\n"
        "them you see it again: the tall black shadow with\n"
        "burning red eyes, watching you.\n"
        "The Shadow Warden has not forgotten you.",

        "An old keep on a black hill. The seventh shard is\n"
        "inside - and so is the Shadow Warden, faster now.\n"
        "\n"
        "It wakes up soon after you enter and hunts you\n"
        "through every corridor. It cannot pass closed doors.\n"
        "A treasure reward (every 3 coins) freezes it for\n"
        "5 seconds. Do not let it catch you. RUN!",

        "The eighth shard sank to the bottom of the sea with\n"
        "an ancient city. You dive into the dark in grandpa's\n"
        "old yellow submarine.\n"
        "\n"
        "Down here the lamp shows only a few steps around\n"
        "you. The rest is black, silent water...",

        "A balloon carries you above the clouds, to islands\n"
        "that float in the sky. Broken bridges join them\n"
        "like a maze in the air.\n"
        "\n"
        "Somewhere up here grandpa built his greatest\n"
        "machine: a rocket called the 'Comet'.",

        "The Comet roars into the night. On the Moon lies the\n"
        "last labyrinth - the home of the Shadow Warden.\n"
        "Grandpa Elias is its prisoner.\n"
        "\n"
        "Put the last shard into the Star Compass and reach\n"
        "the heart of the maze. The Warden will hunt you one\n"
        "last time. This is the end of the road.",
    };
    if (!isValidLevel(level)) {
        return "";
    }
    return intros[level - 1];
}

string chapterOutro(int level) {
    const string outros[levelCount] = {
        "In the cellar you find the first shard. It glows like a tiny star.",
        "You escape with the second shard - and the storm throws you ashore.",
        "The parrot leads you to an old tree. In its roots: the third shard.",
        "You escape the Warden with the fourth shard and grandpa's old hat.",
        "You grab the fifth shard as the mountain erupts behind you!",
        "The sixth shard is yours. Far below, a cold laugh echoes in the ice.",
        "You slam the gate on the Warden. The seventh shard shines in your hand.",
        "Among the ruins: the eighth shard and a note - 'Look above the clouds.'",
        "The Comet is ready. The Star Compass points to the Moon.",
        "The Star Compass blazes like a sun. The Warden fades away...",
    };
    if (!isValidLevel(level)) {
        return "";
    }
    return outros[level - 1];
}

const string endingText =
    "The light of the Star Compass fills the whole labyrinth.\n"
    "Where the Warden stood, only a cloud of dust remains -\n"
    "and Grandpa Elias, smiling.\n"
    "\n"
    "  'I knew you would come,' he says. 'Every maze has a\n"
    "   way out - for those who never give up.'\n"
    "\n"
    "The Comet brings you both home. That night the Old\n"
    "Lighthouse shines brighter than ever before.";

int letterCount(const string& text) {
    int count = 0;
    for (int i = 0; i < (int)text.length(); i++) {
        if (text[i] != '\n') {
            count++;
        }
    }
    return count;
}

string partOfText(const string& text, int shown) {
    string result;
    int letters = 0;
    for (int i = 0; i < (int)text.length(); i++) {
        if (text[i] == '\n') {
            result += '\n';
        } else {
            result += letters < shown ? text[i] : ' ';
            letters++;
        }
    }
    return result;
}

bool tellStory(const string& heading, const string& text, const string& leftArt, const string& rightArt,
               const string& footer, bool artAbove, int artLevel) {
    int total = letterCount(text);
    int shown = 0;
    int frame = 0;
    showCursor(false);
    clearKeyBuffer();

    while (true) {
        int width = getScreenWidth();
        string inside = heading + "\n\n" + paint(partOfText(text, shown), white) + "\n";
        if (footer != "") {
            inside += "\n" + footer + "\n";
        }
        string box = buildMazeBox(inside);

        string screen;
        if (artAbove) {
            string picture = artLevel == 0 ? endingArt(frame / 4) : leftArt;
            if (blockHeight(picture) + blockHeight(box) + 3 <= getScreenHeight()) {
                screen += centerBlock(picture, width) + "\n";
            }
            screen += centerBlock(box, width);
        } else {
            screen += centerBlock(withSideArt(box, leftArt, rightArt, 3, width), width);
        }

        string prompt;
        if (shown < total) {
            prompt = paint("Enter - show the whole text", muted);
        } else {
            prompt = paint(frame % 6 < 4 ? ">>  Press Enter to continue  <<" : "    Press Enter to continue    ",
                           bold + yellow);
        }
        screen += "\n" + centerLine(prompt, width) + "\n";
        showScreen(screen);

        int key = readKeyTimeout(shown < total ? 20 : 150);
        frame++;
        if (shown < total) {
            if (key == escapeKey) {
                return false;
            }
            shown = key != noKey ? total : shown + 2;
        } else if (key == enterKey || key == spaceKey) {
            return true;
        } else if (key == escapeKey) {
            return false;
        }
    }
}

bool showChapterIntro(int level) {
    LevelInfo info = getLevel(level);
    string heading = paint("CHAPTER " + to_string(level) + (level == levelCount ? "  -  THE FINAL CHAPTER" : ""),
                           bold + orange) + "\n"
                   + paint(levelName(level), bold + yellow) + "\n"
                   + paint("~ ~ ~ ~ ~ ~ ~ ~ ~ ~", dark);

    string footer = paint("Maze " + to_string(info.size) + "x" + to_string(info.size) + "  "
                          + difficultyName(info.difficulty), cyan);
    if (info.difficulty == EXPERT) {
        footer += paint("  |  darkness: you see only around you", muted);
    }
    if (levelHasChaser(level)) {
        footer += "\n" + paint("! The Shadow Warden " + chaserSymbol() + paint("hunts you in this maze !", bold + red),
                               bold + red);
    }
    return tellStory(heading, chapterIntro(level), levelArt(level, 0), levelArt(level, 1), footer, false, level);
}

void showEnding() {
    string heading = paint("EPILOGUE", bold + orange) + "\n" + paint("The Light Returns", bold + yellow) + "\n"
                   + paint("~ ~ ~ ~ ~ ~ ~ ~ ~ ~", dark);
    string theEnd = paint("* * *   T H E   E N D   * * *", bold + yellow);
    tellStory(heading, endingText, "", "", theEnd, true, 0);
}
