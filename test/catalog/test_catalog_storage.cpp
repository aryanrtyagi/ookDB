#include <iostream>

#include "pager.h"
#include "buffer_pool.h"
#include "catalog.h"
#include "catalog_storage.h"


int main()
{
    std::cout
        << "=== Catalog Storage Test ===\n\n";


    // ========================================================
    // Open database
    // ========================================================

    Pager pager(
        "data/database.db"
    );

    BufferPool bufferPool(
        pager
    );


    // ========================================================
    // Create Catalog
    // ========================================================

    Catalog catalog;


    Schema students;

    students.addColumn(
        "id",
        DataType::INT
    );

    students.addColumn(
        "name",
        DataType::VARCHAR
    );

    students.addColumn(
        "age",
        DataType::INT
    );


    catalog.createTable(
        "students",
        students,
        1
    );


    Schema employees;

    employees.addColumn(
        "id",
        DataType::INT
    );

    employees.addColumn(
        "name",
        DataType::VARCHAR
    );

    employees.addColumn(
        "salary",
        DataType::DOUBLE
    );


    catalog.createTable(
        "employees",
        employees,
        10
    );


    // ========================================================
    // Save Catalog
    // ========================================================

    CatalogStorage storage(
        bufferPool
    );


    storage.save(
        catalog
    );


    std::cout
        << "Catalog saved to Page 0.\n";


    // ========================================================
    // Create a completely new Catalog
    // ========================================================

    Catalog loadedCatalog;


    // ========================================================
    // Load from database
    // ========================================================

    storage.load(
        loadedCatalog
    );


    // ========================================================
    // Display
    // ========================================================

    std::cout
        << "\nTables loaded from disk:\n";


    for (
        const std::string& table :
        loadedCatalog.listTables()
    )
    {
        std::cout
            << "- "
            << table
            << "\n";

        std::cout
            << "  First page: "
            << loadedCatalog.getFirstPage(
                table
            )
            << "\n";
    }


    std::cout
        << "\nCatalog storage test completed.\n";


    return 0;
}