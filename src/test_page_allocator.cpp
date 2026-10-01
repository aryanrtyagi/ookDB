#include <iostream>

#include "pager.h"
#include "page_allocator.h"

int main()
{
    Pager pager(
        "data/database.db"
    );

    PageAllocator allocator(
        pager
    );


    std::cout
        << "Current database size: "
        << pager.getFileSize()
        << " bytes\n";


    int page1 =
        allocator.allocatePage();

    std::cout
        << "Next available page: "
        << page1
        << "\n";


    int page2 =
        allocator.allocatePage();

    std::cout
        << "Next available page again: "
        << page2
        << "\n";


    std::cout
        << "\nPage allocator test completed.\n";


    return 0;
}