#include "repl.h"

#include <iostream>
#include <algorithm>
#include <cctype>
#include <stdexcept>

#include "../../sql/lexer/lexer.h"
#include "../../sql/parser/parser.h"

// ============================================================
// Constructor
// ============================================================

Repl::Repl()
    : running(true),
      pager("data/database.db"),
      bufferPool(pager),
      pageAllocator(pager),
      catalog(pageAllocator),
      catalogStorage(bufferPool),
      executor(pager, bufferPool, pageAllocator, catalog, catalogStorage)
{
    catalogStorage.load(catalog);
}

// ============================================================
// Print Prompt
// ============================================================

void Repl::printPrompt()
{
    std::cout << "arydb> ";
}

// ============================================================
// Process SQL Command
// ============================================================

void Repl::processCommand(const std::string& command)
{
    // Ignore leading and trailing whitespace when checking exit.
    std::string trimmed = command;

    auto first = std::find_if_not(
        trimmed.begin(), trimmed.end(),
        [](unsigned char c) { return std::isspace(c); });

    auto last = std::find_if_not(
        trimmed.rbegin(), trimmed.rend(),
        [](unsigned char c) { return std::isspace(c); }).base();

    if (first >= last)
        return;

    trimmed = std::string(first, last);

    if (trimmed == "exit")
    {
        running = false;
        std::cout << "Goodbye!\n";
        return;
    }

    try
    {
        // 1. Convert SQL text into tokens.
        Lexer lexer(trimmed);
        std::vector<Token> tokens = lexer.tokenize();

        // 2. Parse tokens into an AST Query.
        Parser parser(tokens);
        Query query = parser.parse();

        // 3. Execute the parsed query.
        executor.execute(query);
    }
    catch (const std::exception& e)
    {
        std::cout << "Error: " << e.what() << '\n';
    }
}

// ============================================================
// Run REPL
// ============================================================

void Repl::run()
{
    std::cout
        << "==============================\n"
        << "          AryDB v1.0\n"
        << "==============================\n"
        << "Enter SQL statements, or type 'exit' to quit.\n\n";

    std::string command;

    while (running)
    {
        printPrompt();

        if (!std::getline(std::cin, command))
            break;

        processCommand(command);
    }

    bufferPool.flushAll();
}
