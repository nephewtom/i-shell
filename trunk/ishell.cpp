// TODO: Capture special keys: AvPag (+5 cmds), RePag(-5cmds), Ins, Del, ALT+Arrows...

#include <iostream>
#include <string>
#include <vector>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <cstring>

using namespace std;

namespace ase {

class InteractiveShell
{
public:

    InteractiveShell() : linePos_(0), cmdIndex_(0), rowIndex_(0), pageStep_(5)
    {
    }

    void run();

private:
    typedef unsigned int uint;
    vector<char> line_;
    uint linePos_;

    vector<string> cmdHistory_;
    uint cmdIndex_;
    uint rowIndex_;
    uint pageStep_;
    
    static const char BELL = '\07';
    static const char END_OF_LINE = '\n';
    static const char SPECIAL_KEY = 27;
    static const char BACKSPACE = 127;
    enum Key { UP = 0, DOWN, RIGHT, LEFT, HOME, END, PGUP, PGDOWN, INS, DEL, OTHER };

    int getch();
    void acceptCmd();
    void handleSpecialKey();
    void insertChar(char c);
    void eraseChar();
    void renderLine(uint adjust);
    void eraseCommand();
    void showCommand();
    Key getKeyCode();
};
}

using namespace ase;

void InteractiveShell::run(void)
{
    char c;
    cout << "ase[" << cmdIndex_ << "]> ";
    uint adjust = 0;
    while ( (c = getch()) ) {

        //cout << endl << "c:" << ( c > 0 ? (uint) c : (0x100 + (uint)c) ); // debug
        if (c == END_OF_LINE) {
            acceptCmd();
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

void InteractiveShell::acceptCmd()
{
    // cout << " | acceptCmd()"; // debug

    string line;
    for (uint i = 0; i < line_.size(); i++) {
        line.push_back(line_[i]);
    }
    cmdHistory_.push_back(line);

    cout << endl << "command:" << line << " | cmdIdx:" << cmdIndex_ << " | size:" ;
    cout << cmdHistory_[cmdIndex_].size() << " | rowIdx:" << rowIndex_ << endl;
    
    for (uint i = 0; i < cmdHistory_.size(); i++) {
        cout << i << ":" << cmdHistory_[i] << endl;
    }

    linePos_ = 0;
    line_.clear();
    cmdIndex_++;
    rowIndex_ = cmdIndex_;
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
        if (rowIndex_ == 0) {
            cout << BELL;
            return;
        }
        eraseCommand();
        rowIndex_--;
        showCommand();
        break;

    case DOWN:
        if ( (rowIndex_ >= cmdIndex_ - 1) || cmdIndex_ == 0 ) {
            cout << BELL;
            return;
        }
        eraseCommand();
        rowIndex_++;
        showCommand();
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
//        if (rowIndex_ - pageStep_ <= 0) {
//            cout << BELL;
//            return;
//        }
//        eraseCommand();
//        rowIndex_ -= pageStep_;
//        showCommand();
        break;

    case PGDOWN:
//        if (rowIndex_ + pageStep_ >= cmdIndex_) {
//            cout << BELL;
//            return;
//        }
//        eraseCommand();
//        rowIndex_ += pageStep_;
//        showCommand();
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

void InteractiveShell::eraseCommand()
{
    for (uint i = 0; i < line_.size(); i++) {
        cout << "\b \b";
    }
    line_.clear();
}

void InteractiveShell::showCommand()
{
    string line = cmdHistory_[rowIndex_];
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
    shell.run();

    return 0;
}
