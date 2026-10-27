#ifndef REPL_H
#define REPL_H

#include <string>

#include "../../core/pager/pager.h"
#include "../../core/buffer/buffer_pool.h"
#include "../../core/allocation/page_allocator.h"
#include "../../core/catalog/catalog.h"
#include "../../sql/executor/executor.h"
#include "../../core/catalog/catalog_storage.h"

class Repl
{
private:
    bool running;

    // Storage components: initialized in dependency order
    Pager pager;
    BufferPool bufferPool;
    PageAllocator pageAllocator;
    Catalog catalog;
    CatalogStorage catalogStorage;
    Executor executor;

    void printPrompt();
    void processCommand(const std::string& command);

public:
    Repl();
    void run();
};

#endif
