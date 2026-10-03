#include "page_allocator.h"

#include <cstring>

PageAllocator::PageAllocator(Pager& pager)
    : pager(pager)
{
    long long fileSize = pager.getFileSize();

    if (fileSize <= 0)
    {
        // Page 0 is reserved for Catalog.
        nextPageNumber = 1;
    }
    else
    {
        int pageCount =
            static_cast<int>(
                (fileSize + PAGE_SIZE - 1) / PAGE_SIZE
            );

        nextPageNumber = pageCount;

        if (nextPageNumber < 1)
        {
            nextPageNumber = 1;
        }
    }
}


int PageAllocator::getNextPageNumber() const
{
    return nextPageNumber;
}


int PageAllocator::allocatePage()
{
    // Page number that will be allocated.
    int allocatedPage = nextPageNumber;

    // Create an empty page.
    char emptyPage[PAGE_SIZE] = {};

    // ------------------------------------------------------------
    // RecordPage header
    //
    // First 4 bytes  -> record count = 0
    // Next 4 bytes   -> next page = -1
    // ------------------------------------------------------------

    int recordCount = 0;
    int nextPage = -1;

    std::memcpy(
        emptyPage,
        &recordCount,
        sizeof(int)
    );

    std::memcpy(
        emptyPage + sizeof(int),
        &nextPage,
        sizeof(int)
    );

    // Physically create the page.
    // Pager::writePage() already flushes.
    pager.writePage(
        allocatedPage,
        emptyPage
    );

    // Move to next available page.
    nextPageNumber++;

    return allocatedPage;
}