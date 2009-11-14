/* 
 * File:   InteractiveShell.h
 * Author: user
 *
 * Created on 14 November 2009, 20:52
 */

#ifndef _INTERACTIVESHELL_H
#define	_INTERACTIVESHELL_H

#include <string>
#include <vector>
#include <map>

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

    map<string, bool> validCmds_; // Solo necesito una hash list, ni siquiera un mapa...

    vector<string> cmdHistory_; // History of command line shell
    uint cmdIndex_; // Index of current command line
    uint popIndex_; // Index of popped command from history
    uint pageStep_;

    static const char BELL = '\07';
    static const char END_OF_LINE = '\n';
    static const char KILL_LINE = 11;
    static const char SPECIAL_KEY = 27;
    static const char BACKSPACE = 8;
    static const char TAB = 9;

    enum Key
    {
        UP = 0, DOWN, RIGHT, LEFT, HOME, END, PGUP, PGDOWN, INS, DEL, OTHER
    };

    int getch();
    bool acceptLine(); // Accept line after hitting RETURN/INTRO key
    void handleSpecialKey(Key key = OTHER); // Like Arrows, Home, PgUp, PgDw, etc..
    void insertChar(char c);
    void eraseChar();
    void renderLine(uint adjust, bool repos = true);
    void eraseLine();
    void popLine();
    Key getKeyCode();

    bool checkCommad(string& line);

    static void signal_handler(int signal)
    {
        //cout << "\nCaught ^C\n";
    }
};
}

#endif	/* _INTERACTIVESHELL_H */

