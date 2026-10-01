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

    Schema students;

    students.addColumn("id", DataType::INT);
    students.addColumn("name", DataType::VARCHAR);
    students.addColumn("age", DataType::INT);

    catalog.createTable(
        "students",
        students,
        1
    );

    CatalogStorage storage(bufferPool);

    storage.save(catalog);

    std::cout << "Catalog saved successfully.\n";
    std::cout << "Program will now exit.\n";

    return 0;
}