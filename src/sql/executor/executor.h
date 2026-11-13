#ifndef EXECUTOR_H
#define EXECUTOR_H

#include <memory>
#include <string>
#include <unordered_map>

#include "../ast/query.h"
#include "../../record/value.h"
#include "../../record/row.h"
#include "../../record/schema.h"
#include "../../core/pager/pager.h"
#include "../../core/buffer/buffer_pool.h"
#include "../../core/allocation/page_allocator.h"
#include "../../core/catalog/catalog.h"
#include "../../core/catalog/catalog_storage.h"
#include "../../core/table/generic_table.h"
#include "../../index/index_manager.h"

#include "query_result.h"

class Executor
{
private:
    Pager& pager;
    BufferPool& bufferPool;
    PageAllocator& pageAllocator;
    Catalog& catalog;
    CatalogStorage& catalogStorage;
    std::unordered_map<std::string, std::unique_ptr<IndexManager>> indexManagers;

    void loadIndexManagers();
    GenericTable openTable(const std::string& tableName);
    Value convertLiteral(const Literal& literal, DataType expectedType);
    bool evaluateCondition(const Row& row, const Schema& schema, const Condition& condition);

    QueryResult executeCreateIndex(const CreateIndexQuery& query);
    QueryResult executeInsert(const InsertQuery& query);
    QueryResult executeSelect(const SelectQuery& query);
    QueryResult executeUpdate(const UpdateQuery& query);
    QueryResult executeDelete(const DeleteQuery& query);

public:
    Executor(Pager& pager,
             BufferPool& bufferPool,
             PageAllocator& pageAllocator,
             Catalog& catalog,
             CatalogStorage& catalogStorage);

    QueryResult execute(const Query& query);
};

#endif
