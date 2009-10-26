// TODO: Capture special keys: AvPag (+5 cmds), RePag(-5cmds), Ins, Del, ALT+Arrows...

#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <map>

#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

using namespace std;

namespace ase {

class InteractiveShell
{
public:

    InteractiveShell() : linePos_(0), cmdIndex_(0), popIndex_(0), pageStep_(5)
    {
    }

    void init();
    void run();

private:
    typedef unsigned int uint;
    vector<char> line_; // Holds the current command line contents
    uint linePos_; // Position in the command line

    map<string,bool> validCmds_; // Solo necesito una hash list, ni siquiera un mapa...

    vector<string> cmdHistory_; // History of command line shell
    uint cmdIndex_; // Index of current command line
    uint popIndex_; // Index of popped command from history
    uint pageStep_;
    
    static const char BELL = '\07';
    static const char END_OF_LINE = '\n';
    static const char SPECIAL_KEY = 27;
    static const char BACKSPACE = 127;
    static const char TAB = 9;
    enum Key { UP = 0, DOWN, RIGHT, LEFT, HOME, END, PGUP, PGDOWN, INS, DEL, OTHER };

    int getch();
    bool acceptLine(); // Accept line after hitting RETURN/INTRO key
    void handleSpecialKey(); // Like Arrows, PgUp, PgDw, etc..
    void insertChar(char c);
    void eraseChar();
    void renderLine(uint adjust);
    void eraseLine();
    void popLine();
    Key getKeyCode();

    void checkCommad(string& line);

    static void signal_handler(int signal) {
        //cout << "\nCaught ^C\n";
    }
};
}

using namespace ase;

void InteractiveShell::init()
{
    signal(SIGINT, &signal_handler);
    validCmds_["ls"] = true;
    validCmds_["mkdir"] = true;
    validCmds_["cd"] = true;    
}

void InteractiveShell::run(void)
{
    char c;
    cout << "ase[" << cmdIndex_ << "]> ";
    uint adjust = 0;
    while ( (c = getch()) ) {

        // Uncomment the 3 cout ins this file to view var values
        //cout << endl << endl << "c:" << ( c > 0 ? (uint) c : (0x100 + (uint)c) ); // debug
        if (c == END_OF_LINE) {
            bool isOk = acceptLine();
            if (!isOk) {
                return;
            }
        }
        else if (c == SPECIAL_KEY) {
            // Warning: more characters follow!.
            handleSpecialKey();
        }
        else if (c == BACKSPACE) {

            if (line_.size() == 0 || linePos_ == 0) {
                cout << BELL;                
            }
            else {
                eraseChar();
                linePos_--;
                adjust = 1;
            }
        }
        else if (c == TAB) {
            string line;
            for (uint i = 0; i < line_.size(); i++) {
                line.push_back(line_[i]);
            }
            checkCommad(line);
        }
        else {
            if (linePos_ == line_.size()) {
                line_.push_back(c);
            }
            else {
                insertChar(c);
            }
            linePos_++;
        }

        //cout << " | pos:" << linePos_ << " | size:" << line_.size() << endl; // debug
        //cout << ":" << linePos_ << " | size:" << line_.size() << endl; // debug
        renderLine(adjust);
        adjust = 0;
    }
}

bool InteractiveShell::acceptLine()
{
    // cout << " | acceptCmd()"; // debug

    string line;
    for (uint i = 0; i < line_.size(); i++) {
        line.push_back(line_[i]);
    }
    cmdHistory_.push_back(line);

    cout << endl << "line:" << line << " | size:" << cmdHistory_[cmdIndex_].size();
    cout << " | cmdIdx:" << cmdIndex_ <<  " | rowIdx:" << popIndex_ << endl;
    
    for (uint i = 0; i < cmdHistory_.size(); i++) {
        cout << i << ":" << cmdHistory_[i] << endl;
    }

    linePos_ = 0;
    line_.clear();
    cmdIndex_++;
    popIndex_ = cmdIndex_;

    if (!line.compare("q")) {
        return false;
    }
    
    checkCommad(line);
    return true;
}

void InteractiveShell::checkCommad(string& line)
{
    map<string,bool>::iterator ii;
    ii = validCmds_.find(line);
    if (ii != validCmds_.end()) {
        cout << "Valid command" << endl;
    }
}

void InteractiveShell::insertChar(char c)
{
    // cout << " { insertChar('" << c << "')"; // debug
    int size = line_.size();
    line_.resize( size + 1 );
    for (uint i=size; i>linePos_; i--) {
        line_[i] = line_[i-1];
        // cout << " | l[" << i << "]:" << line_[i]; // debug
    }
    line_[linePos_] = c;
    // cout << " | l[0]:" << line_[0] << " }"; // debug
}

void InteractiveShell::eraseChar()
{
    int size = line_.size();
    for (int i=linePos_; i<size; i++) {
        line_[i-1] = line_[i];
    }
    line_.resize(size - 1);
}

void InteractiveShell::renderLine(uint adjust)
{
    // cout << "renderLine()" << endl; // debug

    //      "\rase[]> "
    cout << "\r        ";
    if (cmdIndex_ > 9) cout << " ";
    if (cmdIndex_ > 99) cout << " ";
    if (cmdIndex_ > 999) cout << " ";

    for (uint i=0; i<line_.size() + adjust; i++) {
        cout << " ";
    }

    cout << "\rase[" << dec << cmdIndex_ << "]> ";
    for (uint i=0; i<line_.size(); i++) {
        cout << line_[i];
    }

    for (uint i=line_.size(); i>linePos_; i--) {
        cout << "\b";
    }
}

void InteractiveShell::handleSpecialKey()
{
    Key key = getKeyCode();
    switch (key) {
    case UP:
        if (popIndex_ == 0) {
            cout << BELL;
            return;
        }
        eraseLine();
        popIndex_--;
        popLine();
        break;

    case DOWN:
        if ( (popIndex_ >= cmdIndex_ - 1) || cmdIndex_ == 0 ) {
            cout << BELL;
            return;
        }
        eraseLine();
        popIndex_++;
        popLine();
        break;

    case RIGHT: {
        if (linePos_ == line_.size()) {
            cout << BELL;
            return;
        }
        linePos_++;
        break;
    }
    case LEFT:
        if (linePos_ == 0) {
            cout << BELL;
            return;
        }
        linePos_--;
        break;

    case HOME:
        linePos_ = 0;
        break;

    case END:
        linePos_ = line_.size();
        break;

    case PGUP:
//        if (popIndex_ - pageStep_ <= 0) {
//            cout << BELL;
//            return;
//        }
//        eraseLine();
//        popIndex_ -= pageStep_;
//        popLine();
        break;

    case PGDOWN:
//        if (popIndex_ + pageStep_ >= cmdIndex_) {
//            cout << BELL;
//            return;
//        }
//        eraseLine();
//        popIndex_ += pageStep_;
//        popLine();
        break;

    case INS:
        cout << " (Ins) ";
        break;

    case DEL:
        cout << " (Del) ";
        break;

    default:
        break;
    }
}

InteractiveShell::Key InteractiveShell::getKeyCode()
{
    char sc[3]; // special character
    sc[0] = getchar(); // store next two characters
    sc[1] = getchar();
    if ( (sc[1] >= 49 && sc[1] <= 54) || sc[1] == 91) {
        // Check: http://bytes.com/topic/python/answers/502625-detecting-key-presses
        sc[2] = getchar();
    }

    //cout << "s0:" << (int)sc[0] <<"s1:" << (int)sc[1]; // debug
    if (sc[0] == 91 && sc[1] == 65) { return UP; }
    else if (sc[0] == 91 && sc[1] == 66) { return DOWN; }
    else if (sc[0] == 91 && sc[1] == 67) { return RIGHT; }
    else if (sc[0] == 91 && sc[1] == 68) { return LEFT; }
    else if (sc[0] == 91 && sc[1] == 49 && sc[2] == 126) { return HOME; }
    else if (sc[0] == 91 && sc[1] == 50 && sc[2] == 126) { return INS; }
    else if (sc[0] == 91 && sc[1] == 51 && sc[2] == 126) { return DEL; }
    else if (sc[0] == 91 && sc[1] == 52 && sc[2] == 126) { return END; }
    else if (sc[0] == 91 && sc[1] == 53 && sc[2] == 126) { return PGUP; }
    else if (sc[0] == 91 && sc[1] == 54 && sc[2] == 126) { return PGDOWN; }
    else return OTHER;
}

void InteractiveShell::eraseLine()
{
    for (uint i = 0; i < line_.size(); i++) {
        cout << "\b \b";
    }
    line_.clear();
}

void InteractiveShell::popLine()
{
    string line = cmdHistory_[popIndex_];
    for (uint i = 0; i < line.size(); i++) {
        line_.push_back(line[i]);
    }
    cout << line;
    linePos_ = line.size();
}

int InteractiveShell::getch(void)
{
/*
 * From:
    http://www.cplusplus.com/forum/beginner/14136/
     and
    http://www.anthonycargile.com/blog/?p=129
 */
    int c = 0;
    struct termios old_opts, new_opts;

    //----- store old settings -----------
    int res = tcgetattr(STDIN_FILENO, &old_opts);

    //---- set new terminal parms --------
    memcpy(&new_opts, &old_opts, sizeof (new_opts));
    new_opts.c_lflag &= ~(ICANON | ECHO); // enable canonical input, disable echo

    // Other options
    //new_opts.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL | ECHOPRT | ECHOKE | ICRNL);
    //new_opts.c_cc[VTIME] = 0;
    //new_opts.c_cc[VMIN] = 1;
    //new_opts.c_lflag |= ISIG; // handle sigs, in case you need this

    tcsetattr(STDIN_FILENO, TCSANOW, &new_opts); // set new terminal options
    c = getchar(); // standard getchar, no echo, not buffered now

    //------ restore old settings ---------
    res = tcsetattr(STDIN_FILENO, TCSANOW, &old_opts);
    return c;
}

int main()
{
    InteractiveShell shell;
    shell.init();
    shell.run();

    return 0;
}
