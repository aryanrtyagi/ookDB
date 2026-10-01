#include "generic_table.h"

#include <stdexcept>


GenericTable::GenericTable(
    Pager& pager,
    BufferPool& bufferPool,
    const Schema& schema
)
    : pager(pager),
      bufferPool(bufferPool),
      schema(schema),
      firstPage(0)
{
}


bool GenericTable::insert(const Row& row)
{
    /*
        For now we start from the first page.

        Later we will maintain a page directory
        so that we don't have to scan from page 0.
    */

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

        /*
            Try inserting into this page.
        */

        if (page.insert(row))
        {
            /*
                Page has changed.
                Tell BufferPool that it needs
                to be written to disk.
            */

            bufferPool.markDirty(pageNumber);

            return true;
        }

        /*
            Page is full.

            Move to next page.
        */

        pageNumber++;
    }
}


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

    while (true)
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

        /*
            If there are no records on this page
            and we haven't found the requested row,
            the row doesn't exist.
        */

        if (count == 0)
        {
            throw std::out_of_range(
                "Row does not exist"
            );
        }

        pageNumber++;
    }
}


int GenericTable::size()
{
    int total = 0;

    int pageNumber = firstPage;

    while (true)
    {
        char* pageData =
            bufferPool.getPage(pageNumber);

        if (pageData == nullptr)
        {
            break;
        }

        RecordPage page(pageData);

        int count = page.size();

        if (count == 0)
        {
            break;
        }

        total += count;

        pageNumber++;
    }

    return total;
}


const Schema& GenericTable::getSchema() const
{
    return schema;
}