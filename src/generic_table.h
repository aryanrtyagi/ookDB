#ifndef GENERIC_TABLE_H
#define GENERIC_TABLE_H

#include "pager.h"
#include "buffer_pool.h"

#include "record/row.h"
#include "record/schema.h"

#include "storage/record_page.h"

class GenericTable
{
private:
    Pager& pager;
    BufferPool& bufferPool;

    Schema schema;

    // First page used by this table
    int firstPage;

public:

    GenericTable(
        Pager& pager,
        BufferPool& bufferPool,
        const Schema& schema
    );

    bool insert(const Row& row);

    Row get(int index);

    int size();

    const Schema& getSchema() const;
};

#endif