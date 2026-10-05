#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "../../core/pager/pager.h"
#include "../../core/buffer/buffer_pool.h"
#include "../../core/allocation/page_allocator.h"
#include "../../core/catalog/catalog.h"
#include "../../core/table/generic_table.h"

#include "../../record/row.h"

#include "../ast/query.h"

class Executor
{
private:
    Pager &pager;
    BufferPool &bufferPool;
    PageAllocator &pageAllocator;
    Catalog &catalog;

    GenericTable openTable(
        const std::string &tableName
    );

    bool evaluateCondition(
        const Row &row,
        const Schema &schema,
        const Condition &condition
    );

public:
    Executor(
        Pager &pager,
        BufferPool &bufferPool,
        PageAllocator &pageAllocator,
        Catalog &catalog);

    void execute(
        const SelectQuery &query);
};

#endif