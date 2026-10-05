#ifndef REPL_H
#define REPL_H

#include <string>

class Repl
{
private:
    bool running;

    void printPrompt();

    void processCommand(
        const std::string& command
    );

public:
    Repl();

    void run();
};

#endif