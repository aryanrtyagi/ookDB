#include "generic_table.h"

#include <stdexcept>


GenericTable::GenericTable(
    Pager& pager,
    BufferPool& bufferPool,
    PageAllocator& pageAllocator,
    const Schema& schema,
    int firstPage
)
    : pager(pager),
      bufferPool(bufferPool),
      pageAllocator(pageAllocator),
      schema(schema),
      firstPage(firstPage)
{
}


// ============================================================
// Insert
// ============================================================

bool GenericTable::insert(const Row& row)
{
    int pageNumber = firstPage;

    while (true)
    {
        char* pageData =
            bufferPool.getPage(pageNumber);

        if (pageData == nullptr)
        {
            return false;
        }

        RecordPage page(pageData);

        // Try inserting into current page
        if (page.insert(row))
        {
            bufferPool.markDirty(pageNumber);
            return true;
        }

        // Current page is full.
        int nextPage = page.getNextPage();

        // Another page already belongs to this table.
        if (nextPage != -1)
        {
            pageNumber = nextPage;
            continue;
        }

        // No next page.
        // Allocate a completely new page.
        int newPage =
            pageAllocator.allocatePage();

        // Link current page to new page.
        page.setNextPage(newPage);

        bufferPool.markDirty(pageNumber);

        // Load the new page.
        char* newPageData =
            bufferPool.getPage(newPage);

        if (newPageData == nullptr)
        {
            return false;
        }

        RecordPage newRecordPage(newPageData);

        // New page must be empty.
        if (!newRecordPage.insert(row))
        {
            return false;
        }

        bufferPool.markDirty(newPage);

        return true;
    }
}


// ============================================================
// Get Row
// ============================================================

Row GenericTable::get(int index)
{
    if (index < 0)
    {
        throw std::out_of_range(
            "Invalid row index"
        );
    }

    int remaining = index;

    int pageNumber = firstPage;

    while (pageNumber != -1)
    {
        char* pageData =
            bufferPool.getPage(pageNumber);

        if (pageData == nullptr)
        {
            throw std::runtime_error(
                "Could not load page"
            );
        }

        RecordPage page(pageData);

        int count = page.size();

        if (remaining < count)
        {
            return page.get(remaining);
        }

        remaining -= count;

        pageNumber = page.getNextPage();
    }

    throw std::out_of_range(
        "Row does not exist"
    );
}


// ============================================================
// Number of rows
// ============================================================

int GenericTable::size()
{
    int total = 0;

    int pageNumber = firstPage;

    while (pageNumber != -1)
    {
        char* pageData =
            bufferPool.getPage(pageNumber);

        if (pageData == nullptr)
        {
            break;
        }

        RecordPage page(pageData);

        total += page.size();

        pageNumber = page.getNextPage();
    }

    return total;
}


// ============================================================
// Schema
// ============================================================

const Schema& GenericTable::getSchema() const
{
    return schema;
}