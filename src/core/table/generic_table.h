#ifndef GENERIC_TABLE_H
#define GENERIC_TABLE_H

#include "../pager/pager.h"
#include "../buffer/buffer_pool.h"
#include "../allocation/page_allocator.h"

#include "../../record/row.h"
#include "../../record/schema.h"
#include "../../storage/record_page.h"

class GenericTable
{
private:
    Pager& pager;
    BufferPool& bufferPool;
    PageAllocator& pageAllocator;

    Schema schema;

    int firstPage;

public:

    GenericTable(
        Pager& pager,
        BufferPool& bufferPool,
        PageAllocator& pageAllocator,
        const Schema& schema,
        int firstPage
    );

    bool insert(const Row& row);

    Row get(int index);

    int size();

    const Schema& getSchema() const;
};

#endif