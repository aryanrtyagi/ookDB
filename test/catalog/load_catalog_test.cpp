#include <iostream>

#include "pager.h"
#include "buffer_pool.h"
#include "catalog.h"
#include "catalog_storage.h"

int main()
{
    Pager pager("data/database.db");
    BufferPool bufferPool(pager);

    Catalog catalog;

    CatalogStorage storage(bufferPool);

    storage.load(catalog);

    std::cout << "Tables loaded from database.db:\n\n";

    for (const std::string& table : catalog.listTables())
    {
        std::cout << "Table: "
                  << table
                  << "\n";

        std::cout << "First page: "
                  << catalog.getFirstPage(table)
                  << "\n\n";
    }

    return 0;
}