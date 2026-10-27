#ifndef EXECUTOR_H
#define EXECUTOR_H

#include "../../core/pager/pager.h"
#include "../../core/buffer/buffer_pool.h"
#include "../../core/allocation/page_allocator.h"
#include "../../core/catalog/catalog.h"
#include "../../core/table/generic_table.h"
#include "../../core/catalog/catalog_storage.h"
#include "../../record/row.h"
#include "../ast/query.h"

class Executor
{
private:
    Pager &pager;
    BufferPool &bufferPool;
    PageAllocator &pageAllocator;
    Catalog &catalog;
    CatalogStorage &catalogStorage;

    GenericTable openTable(
        const std::string &tableName
    );

    bool evaluateCondition(
        const Row &row,
        const Schema &schema,
        const Condition &condition
    );

    Value convertLiteral(
        const Literal& literal,
        DataType expectedType
    );

    void executeSelect(const SelectQuery& query);
    void executeInsert(const InsertQuery& query);
    void executeUpdate(const UpdateQuery& query);
    void executeDelete(const DeleteQuery& query);
    
public:
    Executor(
        Pager &pager,
        BufferPool &bufferPool,
        PageAllocator &pageAllocator,
        Catalog &catalog,
        CatalogStorage &CatalogStorage);

    void execute(
        const Query& query);
};

#endif