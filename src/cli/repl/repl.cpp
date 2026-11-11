#include "repl.h"

#include <iostream>
#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <vector>

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
// Display Query Result
// ============================================================

static void displayResult(const QueryResult& result)
{
    if (!result.columns.empty())
    {
        for (std::size_t i = 0; i < result.columns.size(); ++i)
        {
            if (i)
                std::cout << " | ";

            std::cout << result.columns[i];
        }

        std::cout << '\n';

        for (std::size_t i = 0; i < result.columns.size(); ++i)
        {
            if (i)
                std::cout << "-+-";

            std::cout << "--------";
        }

        std::cout << '\n';

        for (const auto& row : result.rows)
        {
            for (std::size_t i = 0; i < row.size(); ++i)
            {
                if (i)
                    std::cout << " | ";

                const Value& value = row[i];

                switch (value.getType())
                {
                    case DataType::INT:
                        std::cout << value.asInt();
                        break;

                    case DataType::DOUBLE:
                        std::cout << value.asDouble();
                        break;

                    case DataType::BOOLEAN:
                        std::cout << (value.asBool() ? "TRUE" : "FALSE");
                        break;

                    case DataType::VARCHAR:
                        std::cout << value.asString();
                        break;
                }
            }

            std::cout << '\n';
        }

        std::cout << result.affectedRows
                  << " row(s) selected. ["
                  << result.message << "]\n";

        return;
    }

    std::cout << result.message << '\n';
}

// ============================================================
// Process SQL Command
// ============================================================

void Repl::processCommand(const std::string& command)
{
    std::string trimmed = command;

    auto first = std::find_if_not(
        trimmed.begin(),
        trimmed.end(),
        [](unsigned char c) { return std::isspace(c); }
    );

    auto last = std::find_if_not(
        trimmed.rbegin(),
        trimmed.rend(),
        [](unsigned char c) { return std::isspace(c); }
    ).base();

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
        Lexer lexer(trimmed);
        std::vector<Token> tokens = lexer.tokenize();

        Parser parser(tokens);
        Query query = parser.parse();

        QueryResult result = executor.execute(query);

        displayResult(result);
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
