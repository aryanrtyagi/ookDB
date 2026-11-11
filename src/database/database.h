#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <vector>

#include "../core/pager/pager.h"
#include "../core/buffer/buffer_pool.h"
#include "../core/allocation/page_allocator.h"
#include "../core/catalog/catalog.h"
#include "../core/catalog/catalog_storage.h"
#include "../sql/executor/executor.h"

class Database
{
private:
    // Engine components: dependency initialization order matters.
    Pager pager;
    BufferPool bufferPool;
    PageAllocator pageAllocator;
    Catalog catalog;
    CatalogStorage catalogStorage;
    Executor executor;

    bool closed;

public:
    explicit Database(const std::string& databasePath);

    // Execute one SQL statement and return its structured result.
    QueryResult execute(const std::string& sql);

    // Flush pending data and close the database.
    void close();

    ~Database();

    // Prevent accidental copying of engine ownership.
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;
};

#endif
