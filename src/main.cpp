#include <iostream>
#include <string>

#include "pager.h"
#include "page_allocator.h"
#include "catalog.h"

int main()
{
    // ==========================================
    // Initialize AryDB storage engine
    // ==========================================

    Pager pager("data/database.db");

    PageAllocator pageAllocator(pager);

    Catalog catalog(pageAllocator);


    // ==========================================
    // AryDB Shell
    // ==========================================

    std::cout
        << "==============================\n"
        << "          AryDB v1.0\n"
        << "==============================\n";

    std::cout
        << "Type 'exit' to quit.\n\n";


    // ==========================================
    // REPL
    // ==========================================

    std::string command;

    while (true)
    {
        std::cout << "arydb> ";

        // Read complete line from user
        if (!std::getline(std::cin, command))
        {
            break;
        }


        // ======================================
        // Exit command
        // ======================================

        if (command == "exit")
        {
            std::cout << "Goodbye!\n";
            break;
        }


        // ======================================
        // Empty command
        // ======================================

        if (command.empty())
        {
            continue;
        }

        // ======================================
        // Temporary response
        // ======================================

        std::cout
            << "Unknown command: "
            << command
            << "\n";
    }


    return 0;
}