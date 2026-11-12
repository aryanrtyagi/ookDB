
#ifndef EXECUTOR_H
#define EXECUTOR_H

#include <string>
#include <map>
#include <memory>
#include <cstdint>

#include "../../index/index_manager.h"
#include "../../core/pager/pager.h"
#include "../../core/buffer/buffer_pool.h"
#include "../../core/allocation/page_allocator.h"
#include "../../core/catalog/catalog.h"
#include "../../core/table/generic_table.h"
#include "../../core/catalog/catalog_storage.h"
#include "../../record/row.h"
#include "../ast/query.h"
#include "query_result.h"

class Executor
{
private:
    Pager& pager;
    BufferPool& bufferPool;
    PageAllocator& pageAllocator;
    Catalog& catalog;
    CatalogStorage& catalogStorage;

    std::map<std::string, std::unique_ptr<IndexManager>> indexManagers;

    GenericTable openTable(const std::string& tableName);

    Value convertLiteral(
        const Literal& literal,
        DataType expectedType
    );

    bool evaluateCondition(
        const Row& row,
        const Schema& schema,
        const Condition& condition
    );

    QueryResult executeCreateIndex(const CreateIndexQuery& query);
    QueryResult executeSelect(const SelectQuery& query);
    QueryResult executeInsert(const InsertQuery& query);
    QueryResult executeUpdate(const UpdateQuery& query);
    QueryResult executeDelete(const DeleteQuery& query);

    void loadIndexManagers();

public:
    Executor(
        Pager& pager,
        BufferPool& bufferPool,
        PageAllocator& pageAllocator,
        Catalog& catalog,
        CatalogStorage& catalogStorage
    );

    QueryResult execute(const Query& query);
};

#endif
