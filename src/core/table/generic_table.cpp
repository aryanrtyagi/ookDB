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
// Check whether page belongs to this table
// ============================================================

bool GenericTable::containsPage(int targetPage)
{
    int currentPage = firstPage;

    while (currentPage != -1)
    {
        if (currentPage == targetPage)
            return true;

        char* pageData = bufferPool.getPage(currentPage);

        if (pageData == nullptr)
            return false;

        RecordPage page(pageData);

        currentPage = page.getNextPage();
    }

    return false;
}


// ============================================================
// Insert using stable RecordId
// ============================================================

RecordId GenericTable::insertWithId(const Row& row)
{
    int pageNumber = firstPage;

    while (pageNumber != -1)
    {
        char* pageData = bufferPool.getPage(pageNumber);

        if (pageData == nullptr)
            return RecordId();

        RecordPage page(pageData);

        uint32_t slotNumber = 0;
        uint32_t generation = 0;

        // Try inserting into the current page.
        if (page.insert(row, slotNumber, generation))
        {
            bufferPool.markDirty(pageNumber);

            return RecordId(
                pageNumber,
                slotNumber,
                generation
            );
        }

        int nextPage = page.getNextPage();

        // Continue if another page already exists.
        if (nextPage != -1)
        {
            pageNumber = nextPage;
            continue;
        }

        // Current page is full and has no next page.
        int newPageNumber = pageAllocator.allocatePage();

        if (newPageNumber < 0)
            return RecordId();

        // Link the new page.
        page.setNextPage(newPageNumber);

        bufferPool.markDirty(pageNumber);

        char* newPageData =
            bufferPool.getPage(newPageNumber);

        if (newPageData == nullptr)
            return RecordId();

        RecordPage newPage(newPageData);

        // Insert into the newly allocated page.
        if (!newPage.insert(
                row,
                slotNumber,
                generation
            ))
        {
            return RecordId();
        }

        bufferPool.markDirty(newPageNumber);

        return RecordId(
            newPageNumber,
            slotNumber,
            generation
        );
    }

    return RecordId();
}


// ============================================================
// Existing insert API
// ============================================================

bool GenericTable::insert(const Row& row)
{
    RecordId rid = insertWithId(row);

    return rid.isValid();
}


// ============================================================
// Retrieve using RecordId
// ============================================================

Row GenericTable::get(const RecordId& rid)
{
    if (!rid.isValid())
    {
        throw std::out_of_range(
            "Invalid RecordId"
        );
    }

    if (!containsPage(rid.pageNumber))
    {
        throw std::out_of_range(
            "RecordId does not belong to this table"
        );
    }

    char* pageData = bufferPool.getPage(rid.pageNumber);

    if (pageData == nullptr)
    {
        throw std::runtime_error(
            "Could not load record page"
        );
    }

    RecordPage page(pageData);

    return page.getBySlot(
        rid.slotNumber,
        rid.generation
    );
}


// ============================================================
// Update using RecordId
// ============================================================

bool GenericTable::updateRow(
    const RecordId& rid,
    const Row& row
)
{
    if (!rid.isValid())
        return false;

    if (!containsPage(rid.pageNumber))
        return false;

    char* pageData = bufferPool.getPage(rid.pageNumber);

    if (pageData == nullptr)
        return false;

    RecordPage page(pageData);

    if (!page.updateBySlot(
            rid.slotNumber,
            rid.generation,
            row
        ))
    {
        return false;
    }

    bufferPool.markDirty(rid.pageNumber);

    return true;
}


// ============================================================
// Delete using RecordId
// ============================================================

bool GenericTable::deleteRow(const RecordId& rid)
{
    if (!rid.isValid())
        return false;

    if (!containsPage(rid.pageNumber))
        return false;

    char* pageData = bufferPool.getPage(rid.pageNumber);

    if (pageData == nullptr)
        return false;

    RecordPage page(pageData);

    if (!page.eraseBySlot(
            rid.slotNumber,
            rid.generation
        ))
    {
        return false;
    }

    bufferPool.markDirty(rid.pageNumber);

    return true;
}


// ============================================================
// Existing logical-index get
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
        char* pageData = bufferPool.getPage(pageNumber);

        if (pageData == nullptr)
        {
            throw std::runtime_error(
                "Could not load page"
            );
        }

        RecordPage page(pageData);

        int count = page.size();

        if (remaining < count)
            return page.get(remaining);

        remaining -= count;

        pageNumber = page.getNextPage();
    }

    throw std::out_of_range(
        "Row does not exist"
    );
}


// ============================================================
// Existing logical-index update
// ============================================================

bool GenericTable::updateRow(
    int index,
    const Row& row
)
{
    if (index < 0)
        return false;

    int remaining = index;
    int pageNumber = firstPage;

    while (pageNumber != -1)
    {
        char* pageData = bufferPool.getPage(pageNumber);

        if (pageData == nullptr)
            return false;

        RecordPage page(pageData);

        int count = page.size();

        if (remaining < count)
        {
            if (!page.update(remaining, row))
                return false;

            bufferPool.markDirty(pageNumber);

            return true;
        }

        remaining -= count;

        pageNumber = page.getNextPage();
    }

    return false;
}


// ============================================================
// Existing logical-index delete
// ============================================================

bool GenericTable::deleteRow(int index)
{
    if (index < 0)
        return false;

    int remaining = index;
    int pageNumber = firstPage;

    while (pageNumber != -1)
    {
        char* pageData = bufferPool.getPage(pageNumber);

        if (pageData == nullptr)
            return false;

        RecordPage page(pageData);

        int count = page.size();

        if (remaining < count)
        {
            if (!page.erase(remaining))
                return false;

            bufferPool.markDirty(pageNumber);

            return true;
        }

        remaining -= count;

        pageNumber = page.getNextPage();
    }

    return false;
}


// ============================================================
// Number of active rows
// ============================================================

int GenericTable::size()
{
    int total = 0;

    int pageNumber = firstPage;

    while (pageNumber != -1)
    {
        char* pageData = bufferPool.getPage(pageNumber);

        if (pageData == nullptr)
            break;

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
