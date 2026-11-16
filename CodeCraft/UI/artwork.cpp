#include "artwork.h"
#include "console.h"
#include "layout.h"
#include "renderer.h"

#include <cstring>
#include <sstream>

using namespace std;

const int maxArtLines = 14;

struct ArtPiece {
    int count;
    const char* lines[maxArtLines];
    const char* lineColors;
    const char* palette;
};

string letterColor(char letter) {
    switch (letter) {
        case 'b': return "\x1b[38;5;39m";     
        case 'B': return "\x1b[38;5;25m";      
        case 'c': return "\x1b[38;5;51m";     
        case 'w': return "\x1b[38;5;255m";    
        case 's': return "\x1b[38;5;248m";     
        case 'd': return "\x1b[38;5;61m";      
        case 'y': return "\x1b[38;5;226m";    
        case 'Y': return "\x1b[38;5;229m";     
        case 'o': return "\x1b[38;5;208m";    
        case 'r': return "\x1b[38;5;196m";    
        case 'R': return "\x1b[38;5;124m";     
        case 'g': return "\x1b[38;5;46m";     
        case 'G': return "\x1b[38;5;28m";      
        case 'k': return "\x1b[38;5;130m";     
        case 'n': return "\x1b[38;5;178m";      
        case 'm': return "\x1b[38;5;171m";    
        case 'p': return "\x1b[38;5;99m";     
        default:  return muted;
    }
}

string renderArt(const ArtPiece& art) {
    int width = 0;
    for (int i = 0; i < art.count; i++) {
        int length = (int)strlen(art.lines[i]);
        if (length > width) {
            width = length;
        }
    }
    int paletteLength = (int)strlen(art.palette);
    int colorCount = (int)strlen(art.lineColors);

    string block;
    for (int i = 0; i < art.count; i++) {
        char lineColor = i < colorCount ? art.lineColors[i] : ' ';
        string line;
        string currentColor;
        int length = (int)strlen(art.lines[i]);
        for (int x = 0; x < width; x++) {
            char character = x < length ? art.lines[i][x] : ' ';
            char colorLetter = lineColor;
            for (int p = 0; p + 1 < paletteLength; p += 2) {
                if (art.palette[p] == character) {
                    colorLetter = art.palette[p + 1];
                }
            }
            string color = letterColor(colorLetter);
            if (character != ' ' && color != currentColor) {         
                line += color;
                currentColor = color;
            }
            line += character;
        }
        block += line + reset + "\n";
    }
    return block;
}

string buildTitle(int linesBelow, int frame) {
    int width = getScreenWidth();

    if (7 + linesBelow > getScreenHeight()) {
        return centerLine(paint(" M A Z E B O U N D ", titleTag), width) + "\n\n";
    }

    const string titleLines[5] = {
        " __  __    _     __________ ____   ___  _   _ _   _ ____  ",
        "|  \\/  |  / \\   |__  / ____| __ ) / _ \\| | | | \\ | |  _ \\ ",
        "| |\\/| | / _ \\    / /|  _| |  _ \\| | | | | | |  \\| | | | |",
        "| |  | |/ ___ \\  / /_| |___| |_) | |_| | |_| | |\\  | |_| |",
        "|_|  |_/_/   \\_\\/____|_____|____/ \\___/ \\___/|_| \\_|____/ ",
    };
    const int waveColors[16] = { 51, 45, 39, 33, 63, 99, 135, 171, 207, 171, 135, 99, 63, 33, 39, 45 };

    string title;
    for (int i = 0; i < 5; i++) {
        string line;
        int lastColor = -1;
        for (int x = 0; x < (int)titleLines[i].length(); x++) {
            int index = ((x / 4 + i - frame) % 16 + 16) % 16;
            int color = waveColors[index];
            if (color != lastColor) {
                line += "\x1b[1;38;5;" + to_string(color) + "m";
                lastColor = color;
            }
            line += titleLines[i][x];
        }
        title += line + reset + "\n";
    }
    string result = centerBlock(title, width);

    const int track = 24;
    int position = frame % (track + 6);
    string runner = arrowSymbol() + " ";
    for (int i = 0; i < track; i++) {
        if (i == position) {
            runner += playerSymbol();
        } else if (i < position) {
            runner += trailSymbol();
        } else {
            runner += "  ";
        }
    }
    if (position >= track) {
        runner += (frame % 2 == 0) ? paint("@!", bold + yellow) : paint("EX", bold + yellow);      
    } else {
        runner += exitSymbol();
    }
    result += centerLine(runner, width) + "\n\n";
    return result;
}

string torchArt(int frame) {
    const char* sparks[4] = { "   .   ", "  ' .  ", "    '  ", " .   ' " };
    const char* flames[4][3] = {
        { "   (   ", "  ) )  ", " ( ( ) " },
        { "    )  ", "  ( (  ", " ) ) ( " },
        { "   )   ", "  ( )  ", " ) ( ( " },
        { "  (    ", "   ) ) ", " ( ) ) " },
    };
    const char* flameColors[3] = { "yor", "oyr", "yyo" };

    int step = frame % 4;
    const char* colors = flameColors[frame % 3];
    ArtPiece torch = { 11, {
        sparks[(frame / 2) % 4],
        flames[step][0],
        flames[step][1],
        flames[step][2],
        R"( \###/ )",
        R"(  |=|  )",
        R"(  |=|  )",
        R"(  |=|  )",
        R"(  |=|  )",
        R"( _|=|_ )",
        R"([_____])",
    }, "", "" };

    string lineColors = "Y";
    lineColors += colors;
    lineColors += "skkkkks";
    torch.lineColors = lineColors.c_str();
    return renderArt(torch);
}

const ArtPiece lighthouseArt = { 12, {
    R"(      .  |  .     )",
    R"(    `.  \|/  .`   )",
    R"(  ---- ( O ) ---- )",
    R"(        /_\       )",
    R"(       |===|      )",
    R"(       | o |      )",
    R"(       |===|      )",
    R"(       |   |      )",
    R"(       |===|      )",
    R"(      /  _  \     )",
    R"(  ~~~/  | |  \~~~ )",
    R"( ~~~~~~~~~~~~~~~~~)",
}, "yyysswwwwwwb", "=r~bOYoy" };

const ArtPiece sailboatArt = { 10, {
    R"( +      .      C  )",
    R"(    .      +      )",
    R"(        |\        )",
    R"(        | \       )",
    R"(        |  \      )",
    R"(        |___\     )",
    R"(    ____|_____    )",
    R"(    \ o  o  o/    )",
    R"( ~~~~\______/~~~~ )",
    R"(  ~~~   ~~~~  ~~  )",
}, "ywwwwwkkkb", "|koy~bCY+y" };

const ArtPiece stormArt = { 10, {
    R"(        .-~~~~-.      )",
    R"(    .-~~        ~-.   )",
    R"(   (    ~~~  ~~~   )  )",
    R"(    `-.~~~--~~~~.-'   )",
    R"(     ' ,  /  ' , ' ,  )",
    R"(    , ' ,/__ , ' ,    )",
    R"(     ' ,   / ' , ' ,  )",
    R"(    , ' , / , ' , '   )",
    R"(     ' , /' , ' ,     )",
    R"(    , ' /, ' , ' ,    )",
}, "ssssbbbbbb", "/y_y" };

const ArtPiece stormShipArt = { 9, {
    R"(         |\           )",
    R"(         | \   |\     )",
    R"(         |  \  | \    )",
    R"(         |___\ |__\   )",
    R"(    _____|_____|____  )",
    R"(    \  o   o   o   /  )",
    R"( /\  \____________/ /\)",
    R"(/  \/\~~~~~~~~~~~~/\/ )",
    R"(~~~~~~~~~~~~~~~~~~~~~~)",
}, "wwwwkkbbb", "|koy~b_k" };

const ArtPiece palmArt = { 11, {
    R"(     __ _ __       )",
    R"(   .'  ' '  '.     )",
    R"(  / .-'| |'-. \    )",
    R"(   /  /| |\  \     )",
    R"(     / |)| \       )",
    R"(       |(|         )",
    R"(       |)|         )",
    R"(       |(|         )",
    R"(       |)|         )",
    R"(  ,/\,_|(|_,/\,    )",
    R"(  ^^^^^^^^^^^^^^   )",
}, "gggggkkkkGG", "|k(k)k" };

const ArtPiece parrotArt = { 11, {
    R"(       .---.       )",
    R"(      / o  >>      )",
    R"(     |   __/       )",
    R"(     |  /  \       )",
    R"(     | |   |       )",
    R"(      \ \  |       )",
    R"(  =====\_\_/=====  )",
    R"(        | |        )",
    R"(        | |        )",
    R"(        |||        )",
    R"(        ||         )",
}, "rrrrrrkbbbb", "ow>y=k" };

const ArtPiece templeArt = { 10, {
    R"(          []          )",
    R"(        _|==|_        )",
    R"(       |______|       )",
    R"(      _|  ||  |_      )",
    R"(     |__________|     )",
    R"(    _|   |  |   |_    )",
    R"(   |______________|   )",
    R"(  _|    |    |    |_  )",
    R"( |__________________| )",
    R"( ,;,,;,,;,,,;,,;,,;,, )",
}, "ynnnnnnnnG", "[y]y=y" };

const ArtPiece chestArt = { 9, {
    R"(    $    *    $    )",
    R"(  *   .-~~~~-.   * )",
    R"(    .'  $$$$  '.   )",
    R"(   /____________\  )",
    R"(   |  .------.  |  )",
    R"(   |==|  ()  |==|  )",
    R"(   |  '------'  |  )",
    R"(   |____________|  )",
    R"(  $  $$  $$$  $$ $ )",
}, "yykkkkkky", "$y*w=y(y)y~y" };

const ArtPiece volcanoArt = { 10, {
    R"(     (  ) (  )  (     )",
    R"(    (  (    )  ) )    )",
    R"(      '.\ | /.'       )",
    R"(     .  \\|//  .      )",
    R"(        _\|/_         )",
    R"(       /  ~  \        )",
    R"(      /  ~ ~  \       )",
    R"(     /  /   ~  \      )",
    R"(    /  /  ~  \  \     )",
    R"(   /__/________\__\   )",
}, "ssoorkkkkk", "~o|y" };

const ArtPiece lavaArt = { 9, {
    R"(      )    (   )     )",
    R"(   ( (  ) ) ( (      )",
    R"(    ) ) ( (  ) )     )",
    R"(   ( ( ) ) ) ( (     )",
    R"(    \/\/\/\/\/\/     )",
    R"(  _/^^\__/^^\__/^\_  )",
    R"( ~~~~~~~~~~~~~~~~~~~ )",
    R"(  ~  ~~~ ~ ~~~~  ~~  )",
    R"( ~~~ ~ ~~~~~~ ~~ ~~~ )",
}, "yoorrsroR", "" };

const ArtPiece mountainArt = { 8, {
    R"(   +      .    +     )",
    R"(        /\     .     )",
    R"(   .   /**\   /\   + )",
    R"(      /****\ /**\    )",
    R"(     /  /\  \/   \   )",
    R"(    /  /  \  \    \  )",
    R"(   /  /    \  \    \ )",
    R"(  /__/______\__\____\)",
}, "wccccccc", "*w+y.w" };

const ArtPiece snowflakeArt = { 10, {
    R"( ____________________ )",
    R"( \/\/\/V\/\/V\/\/\/\/ )",
    R"(  V  V  |  V  V  V    )",
    R"(     '  V     '       )",
    R"(  .        +      .   )",
    R"(       \  |  /        )",
    R"(     -- \ | / --      )",
    R"(    ---- >*< ----     )",
    R"(     -- / | \ --      )",
    R"(       /  |  \        )",
}, "ccccwwwwww", "*c+c" };

const ArtPiece castleArt = { 9, {
    R"(  ^v^          ^v^   )",
    R"(   T~~       T~~     )",
    R"(   |         |       )",
    R"(  /^\  ___  /^\      )",
    R"( |[ ]|[_|_]|[ ]|     )",
    R"( |   |  _  |   |     )",
    R"( |   | | | |   |     )",
    R"( |___|_|_|_|___|     )",
    R"(/^^^^^^^^^^^^^^^\    )",
}, "mrssssssd", "[y]y" };

const ArtPiece ghostArt = { 11, {
    R"(      _..._          )",
    R"(    .:::::::.   ^v^  )",
    R"(   :::::::::::       )",
    R"(   :::::::::::  ^v^  )",
    R"(    ':::::::'        )",
    R"(      `'''`          )",
    R"(       .----.        )",
    R"(      / O  O \       )",
    R"(     |        |      )",
    R"(     |  \__/  |      )",
    R"(     /\/\/\/\/\      )",
}, "YYYYYYsssss", "Or^mvm" };

const ArtPiece submarineArt = { 10, {
    R"(      o    O        )",
    R"(    .    __|_   o   )",
    R"(       _|____|_     )",
    R"(  ____/        \__  )",
    R"( / []   []   []   \ )",
    R"(<==                ))",
    R"( \_______________/  )",
    R"(      .   o   .     )",
    R"(  ,,  )  (  ,,  )(  )",
    R"( ((  ( )  ) )) (  ) )",
}, "cyyyyyycGG", "ocOc.c[c]c<s=s" };

const ArtPiece fishArt = { 10, {
    R"(   o                 )",
    R"(      ><>       O    )",
    R"(  o        ><(((('>  )",
    R"(    <><              )",
    R"(            o   ><>  )",
    R"(   ><(((('>          )",
    R"(         o     <><   )",
    R"(  )  ((   )  ((  )   )",
    R"( ((  ))  ((  ))  ((  )",
    R"( _/\_/\__/\__/\_/\__ )",
}, "coymgoyGGn", "ocOc" };

const ArtPiece balloonArt = { 11, {
    R"(      _.--._        )",
    R"(    .'  ||  '.      )",
    R"(   /  | || |  \     )",
    R"(  |   | || |   |    )",
    R"(   \  | || |  /     )",
    R"(    '.|_||_|.'      )",
    R"(      \ || /        )",
    R"(       \||/         )",
    R"(       [__]         )",
    R"(  .-~~-.    .-~-.   )",
    R"( (_______)  (____)  )",
}, "rrrrrrkkkww", "|y" };

const ArtPiece skyIslandArt = { 11, {
    R"(  \/        \/      )",
    R"(        _  _        )",
    R"(      _| || |_  \/  )",
    R"(     | |_||_| |     )",
    R"(  ___|________|___  )",
    R"(  \,,,,,,,,,,,,,,/  )",
    R"(   \  .  ..  .  /   )",
    R"(    \__  ..  __/    )",
    R"(       \____/       )",
    R"(  .-~-.     .-~~-.  )",
    R"( (_____)   (______) )",
}, "swwwwgkkkww", ",g" };

const ArtPiece rocketArt = { 13, {
    R"(        /\         )",
    R"(       /  \        )",
    R"(      |    |       )",
    R"(      | [] |       )",
    R"(      |    |       )",
    R"(      | [] |       )",
    R"(     /|    |\      )",
    R"(    / |    | \     )",
    R"(   /__|____|__\    )",
    R"(       |##|        )",
    R"(       (  )        )",
    R"(      ( )( )       )",
    R"(     (  )(  )      )",
}, "rrwwwwrrrsyor", "[c]c#s" };

const ArtPiece planetArt = { 12, {
    R"(  *     .     +    . )",
    R"(         _____       )",
    R"(  .   .-'     '-.    )",
    R"(   ==/===========\== )",
    R"(  ==(=============)==)",
    R"(     \           /   )",
    R"(  +   '-._____.-'    )",
    R"(                   * )",
    R"(   .       *         )",
    R"(       .-.     .     )",
    R"( *    (   )       +  )",
    R"(       '-'    .      )",
}, "wppppppwwsss", "=y*y+c" };

string levelArt(int level, int side) {
    const ArtPiece* pictures[10][2] = {
        { &lighthouseArt, &sailboatArt },
        { &stormArt, &stormShipArt },
        { &palmArt, &parrotArt },
        { &templeArt, &chestArt },
        { &volcanoArt, &lavaArt },
        { &mountainArt, &snowflakeArt },
        { &castleArt, &ghostArt },
        { &submarineArt, &fishArt },
        { &balloonArt, &skyIslandArt },
        { &rocketArt, &planetArt },
    };
    if (level < 1 || level > 10 || side < 0 || side > 1) {
        return "";
    }
    return renderArt(*pictures[level - 1][side]);
}

string lockArt() {
    const ArtPiece lock = { 8, {
        R"(    .-----.    )",
        R"(   / .---. \   )",
        R"(   | |   | |   )",
        R"(  _|_|___|_|_  )",
        R"( |           | )",
        R"( |    (_)    | )",
        R"( |     |     | )",
        R"( |___________| )",
    }, "ddddssss", "(y_y)y" };
    return renderArt(lock);
}

string trophyArt() {
    const ArtPiece trophy = { 10, {
        R"(    ___________    )",
        R"(   '._==_==_=_.'   )",
        R"(   .-\:      /-.   )",
        R"(  | (|:.     |) |  )",
        R"(   '-|:.     |-'   )",
        R"(     \::.    /     )",
        R"(      '::. .'      )",
        R"(        ) (        )",
        R"(      _.' '._      )",
        R"(     '-------'     )",
    }, "nnnnnnnnkk", ":Y=y" };
    return renderArt(trophy);
}

string endingArt(int frame) {
    bool left = frame % 2 == 0;
    const ArtPiece ending = { 12, {
        left ? R"(  `.          \  |              )" : R"(               |  /          .`  )",
        left ? R"(     `-.  .    \ |   .          )" : R"(           .   | /    .  .-`     )",
        left ? R"(  -------====( (O) )            )" : R"(            ( (O) )====-------  )",
        R"(               /___\              )",
        R"(               |===|              )",
        R"(               | o |              )",
        R"(               |===|     o    o   )",
        R"(               |   |    /|\  /|\  )",
        R"(               |===|    / \  / \  )",
        R"(              /  _  \  __________ )",
        R"(   ~~~~~~~~~~/  | |  \~~~~~~~~~~~ )",
        R"(  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~)",
    }, "yyysswwwwwwb", "=r~bOYoy" };
    return renderArt(ending);
}

int blockHeight(const string& block) {
    if (block == "") {
        return 0;
    }
    int lines = countLines(block);
    if (block[block.length() - 1] != '\n') {
        lines++;         
    }
    return lines;
}

int blockWidth(const string& block) {
    stringstream stream(block);
    string line;
    int widest = 0;
    while (getline(stream, line)) {
        if (visibleLength(line) > widest) {
            widest = visibleLength(line);
        }
    }
    return widest;
}

string* splitLines(const string& block, int& count) {
    count = blockHeight(block);
    string* lines = new string[count > 0 ? count : 1];
    stringstream stream(block);
    for (int i = 0; i < count; i++) {
        getline(stream, lines[i]);
    }
    return lines;
}

string besideEachOther(const string& left, const string& middle, const string& right, int gap) {
    const string* blocks[3] = { &left, &middle, &right };
    string* lines[3];
    int counts[3];
    int widths[3];
    int tallest = 0;
    for (int i = 0; i < 3; i++) {
        lines[i] = splitLines(*blocks[i], counts[i]);
        widths[i] = blockWidth(*blocks[i]);
        if (counts[i] > tallest) {
            tallest = counts[i];
        }
    }

    string result;
    for (int row = 0; row < tallest; row++) {
        string line;
        bool first = true;
        for (int i = 0; i < 3; i++) {
            if (counts[i] == 0) {
                continue;         
            }
            if (!first) {
                line += string(gap, ' ');
            }
            first = false;
            int top = (tallest - counts[i]) / 2;       
            int index = row - top;
            string part = (index >= 0 && index < counts[i]) ? lines[i][index] : "";
            line += padRight(part + reset, widths[i]);
        }
        result += line + "\n";
    }

    for (int i = 0; i < 3; i++) {
        delete[] lines[i];
    }
    return result;
}

string withSideArt(const string& middle, const string& left, const string& right, int gap, int maxWidth) {
    int middleWidth = blockWidth(middle);
    int leftWidth = left == "" ? 0 : blockWidth(left) + gap;
    int rightWidth = right == "" ? 0 : blockWidth(right) + gap;
    if (middleWidth + leftWidth + rightWidth <= maxWidth) {
        return besideEachOther(left, middle, right, gap);
    }
    if (left != "" && middleWidth + leftWidth <= maxWidth) {
        return besideEachOther(left, middle, "", gap);
    }
    return middle;
}
