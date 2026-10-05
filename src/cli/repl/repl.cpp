#include "repl.h"

#include <iostream>

// ============================================================
// Constructor
// ============================================================

Repl::Repl()
    : running(true)
{
}

// ============================================================
// Print Prompt
// ============================================================

void Repl::printPrompt()
{
    std::cout << "arydb> ";
}

// ============================================================
// Process Command
// ============================================================

void Repl::processCommand(
    const std::string& command)
{
    if (command == "exit")
    {
        running = false;

        std::cout
            << "Goodbye!\n";

        return;
    }

    if (command.empty())
    {
        return;
    }

    std::cout
        << "Unknown command: "
        << command
        << "\n";
}

// ============================================================
// Run REPL
// ============================================================

void Repl::run()
{
    std::cout
        << "==============================\n"
        << "          AryDB v1.0\n"
        << "==============================\n";

    std::cout
        << "Type 'exit' to quit.\n\n";


    std::string command;

    while (running)
    {
        printPrompt();

        if (!std::getline(
                std::cin,
                command))
        {
            break;
        }

        processCommand(command);
    }
}