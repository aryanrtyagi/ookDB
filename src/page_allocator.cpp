#include "page_allocator.h"

PageAllocator::PageAllocator(
    Pager& pager
)
    : pager(pager)
{
}


// ============================================================
// Get next available page
// ============================================================

int PageAllocator::getNextPageNumber() const
{
    long long fileSize =
        pager.getFileSize();

    // Page 0 is reserved for Catalog.

    if (fileSize <= 0)
    {
        return 1;
    }

    int pageCount =
        static_cast<int>(
            (fileSize + PAGE_SIZE - 1)
            / PAGE_SIZE
        );

    if (pageCount == 0)
    {
        return 1;
    }

    return pageCount;
}


// ============================================================
// Allocate a new page
// ============================================================

int PageAllocator::allocatePage()
{
    return getNextPageNumber();
}