// TODO: Capture special keys: AvPag (+5 cmds), RePag(-5cmds), Ins, Del, ALT+Arrows...

#include <iostream>
#include <cstring>

#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h> // atoi

#include "InteractiveShell.h"

using namespace std;
using namespace ase;

#define ISHELL_DEBUG(x) if (debug_) { x }

void InteractiveShell::init()
{
    signal(SIGINT, &signal_handler);
    validCmds_["ls"] = true;
    validCmds_["mkdir"] = true;
    validCmds_["cd"] = true;
    validCmds_["kk"] = true;
}

void InteractiveShell::run(void)
{
    cout << "ase[" << cmdIndex_ << "]> ";

    bool noQuit(true);
    while (noQuit) {

        char c = getch();

        if (c == END_OF_LINE) {
            noQuit = acceptLine();
        }
        else if (c == SPECIAL_KEY) {
            handleSpecialKey();
        }
        else if (c == 1) { // CTRL+A
            handleSpecialKey(HOME);
        }
        else if (c == 5) { // CTRL+E
            handleSpecialKey(END);
        }
        else if (c == BACKSPACE || c == BACKSPACE2) {
            handleBackSpace();
        }
        else if (c == TAB) {
            handleTabKey();
        }
        else if (c == KILL_LINE) { // CTRL+K
            line_.clear();
            linePos_ = 0;
        }
        else if (c < 27) { // We don't want any other CTRL+ keys.

        }
        else {
            insertChar(c);
        }

        ISHELL_DEBUG(cout << endl << "c:" << (c > 0 ? (uint) c : (0x100 + (uint) c));)
        ISHELL_DEBUG(cout << " | pos:" << linePos_ << " | size:" << line_.size() << endl;)

        renderLine();
    }
}

bool InteractiveShell::acceptLine()
{
    // TODO: Borrar espacios al final, pq "quit" != "quit ", y no deberia...

    string line;
    for (uint i = 0; i < line_.size(); i++) {
        line.push_back(line_[i]);
    }
    line_.clear();
    linePos_ = 0;
    cout << endl;
    
    ISHELL_DEBUG( cout << endl << "line:" << line << " | size:" << line.size(); )
    ISHELL_DEBUG( cout << " | cmdIdx:" << cmdIndex_ << " | rowIdx:" << popIndex_ << endl; )
            
    if (line.empty()) {
        //cout << "Not input" << endl; // debug
        return true;
    }
    
    cmdHistory_.push_back(line);
    bool ret = checkCommad(line);

    cmdIndex_++;
    popIndex_ = cmdIndex_;

    return ret;
}

bool InteractiveShell::checkCommad(string& line)
{
    if (!line.compare("q") || !line.compare("quit")) {
        return false;
    }

    if (!line.compare("debug_")) {
        debug_ = !debug_;
        return true;
    }

    if (!line.compare("history")) {
        for (uint i = 0; i < cmdHistory_.size(); i++) {
            cout << i << ":" << cmdHistory_[i] << endl;
        }
        return true;
    }

    uint prevIndex(0);
    if (line.find('!') == 0) {
        if (line[1] == '!') {
            prevIndex = cmdIndex_ - 1;
        }
        else {
            prevIndex = atoi(line.c_str() + 1);
        }

        if (prevIndex >= 0 && prevIndex <= cmdIndex_) {
            cout << "Repeat:" << cmdHistory_[prevIndex] << endl;
            cmdHistory_[cmdIndex_] = cmdHistory_[prevIndex];
        }
    }

    map<string, bool>::iterator ii;
    ii = validCmds_.find(line);
    if (ii != validCmds_.end()) {
        cout << "Valid command" << endl;
    }
    return true;
}

void InteractiveShell::handleTabKey()
{
    string line;
    for (uint i = 0; i < line_.size(); i++) {
        line.push_back(line_[i]);
    }
    checkCommad(line);
}

void InteractiveShell::handleBackSpace()
{
    if (line_.size() == 0 || linePos_ == 0) {
        cout << BELL;
    }
    else {
        eraseChar();
        linePos_--;
    }
}

void InteractiveShell::insertChar(char c)
{
    if (linePos_ == line_.size()) {
        line_.push_back(c);
    }
    else {
        // cout << " { insertChar('" << c << "')"; // debug
        int size = line_.size();
        line_.resize(size + 1);
        for (uint i = size; i > linePos_; i--) {
            line_[i] = line_[i - 1];
            // cout << " | l[" << i << "]:" << line_[i]; // debug
        }
        line_[linePos_] = c;
        // cout << " | l[0]:" << line_[0] << " }"; // debug
    }
    linePos_++;
}

void InteractiveShell::eraseChar()
{
    int size = line_.size();
    for (int i = linePos_; i < size; i++) {
        line_[i - 1] = line_[i];
    }
    line_.resize(size - 1);
}

void InteractiveShell::renderLine()
{
    cout << "\r";
    for (int i = 0; i < 80; i++) {
        cout << " ";
    }

    cout << "\rase[" << dec << cmdIndex_ << "]> ";
    for (uint i = 0; i < line_.size(); i++) {
        cout << line_[i];
    }

    for (uint i = line_.size(); i > linePos_; i--) {
        cout << "\b";
    }
}

void InteractiveShell::handleSpecialKey(Key key)
{
    if (key == OTHER) {
        key = getKeyCode();
    }
    switch (key) {
    case UP:
        if (popIndex_ == 0) {
            cout << BELL;
            return;
        }
        line_.clear();
        popIndex_--;
        popLine();
        break;

    case DOWN:
        if ((popIndex_ >= cmdIndex_ - 1) || cmdIndex_ == 0) {
            cout << BELL;
            return;
        }
        line_.clear();
        popIndex_++;
        popLine();
        break;

    case RIGHT:
    {
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
    char sc[3];
    memset(sc, 0, 3);

    // Check: http://bytes.com/topic/python/answers/502625-detecting-key-presses
    // for a list of decoded keypress
    sc[0] = getch();
    if (sc[0] != 91) {
        return OTHER;
    }

    sc[1] = getch();
    if ((sc[1] >= 49 && sc[1] <= 54) || sc[1] == 91) {
        sc[2] = getch();
    }

    //cout << "-s0:" << (int)sc[0] <<"-s1:" << (int)sc[1]; // debug
    if (sc[0] == 91 && sc[1] == 65) {
        return UP;
    }
    else if (sc[0] == 91 && sc[1] == 66) {
        return DOWN;
    }
    else if (sc[0] == 91 && sc[1] == 67) {
        return RIGHT;
    }
    else if (sc[0] == 91 && sc[1] == 68) {
        return LEFT;
    }
    else if (sc[0] == 91 && sc[1] == 49 && sc[2] == 126) {
        return HOME;
    }
    else if (sc[0] == 91 && sc[1] == 50 && sc[2] == 126) {
        return INS;
    }
    else if (sc[0] == 91 && sc[1] == 51 && sc[2] == 126) {
        return DEL;
    }
    else if (sc[0] == 91 && sc[1] == 52 && sc[2] == 126) {
        return END;
    }
    else if (sc[0] == 91 && sc[1] == 53 && sc[2] == 126) {
        return PGUP;
    }
    else if (sc[0] == 91 && sc[1] == 54 && sc[2] == 126) {
        return PGDOWN;
    }
    else return OTHER;
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
    /* From:
        http://www.cplusplus.com/forum/beginner/14136/
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

