#include <iostream>

#include "pager.h"
#include "page_allocator.h"

int main()
{
    Pager pager("data/database.db");

    PageAllocator allocator(pager);

    std::cout << "========================================\n";
    std::cout << "        AryDB Page Allocation Test\n";
    std::cout << "========================================\n\n";


    // -----------------------------------------
    // Catalog
    // -----------------------------------------

    std::cout << "[CATALOG]\n";
    std::cout << "Page 0 -> RESERVED FOR CATALOG\n\n";


    // -----------------------------------------
    // Existing pages
    // -----------------------------------------

    long long fileSize = pager.getFileSize();

    int existingPages =
        static_cast<int>(
            (fileSize + PAGE_SIZE - 1) / PAGE_SIZE
        );

    std::cout << "[DATABASE]\n";

    std::cout << "Database file size: "
              << fileSize
              << " bytes\n";

    std::cout << "Existing pages: "
              << existingPages
              << "\n\n";


    std::cout << "Page status:\n";

    for (int i = 0; i < existingPages; i++)
    {
        if (i == 0)
        {
            std::cout << "Page "
                      << i
                      << " -> CATALOG\n";
        }
        else
        {
            std::cout << "Page "
                      << i
                      << " -> ALREADY ALLOCATED\n";
        }
    }


    // -----------------------------------------
    // Next page
    // -----------------------------------------

    std::cout << "\n[ALLOCATOR]\n";

    std::cout << "Next available page: "
              << allocator.getNextPageNumber()
              << "\n";


    // -----------------------------------------
    // Allocate pages
    // -----------------------------------------

    std::cout << "\nAllocating 3 new pages...\n\n";

    int page1 = allocator.allocatePage();

    std::cout << "Allocated Page "
              << page1
              << "\n";

    int page2 = allocator.allocatePage();

    std::cout << "Allocated Page "
              << page2
              << "\n";

    int page3 = allocator.allocatePage();

    std::cout << "Allocated Page "
              << page3
              << "\n";


    // -----------------------------------------
    // Final state
    // -----------------------------------------

    std::cout << "\n[FINAL STATE]\n";

    std::cout << "Page 0 -> CATALOG\n";

    for (int i = 1; i <= page3; i++)
    {
        std::cout << "Page "
                  << i
                  << " -> ALLOCATED\n";
    }

    std::cout << "Page "
              << allocator.getNextPageNumber()
              << " -> NEXT AVAILABLE\n";


    std::cout << "\n========================================\n";
    std::cout << "        Page Allocation Complete\n";
    std::cout << "========================================\n";

    return 0;
}