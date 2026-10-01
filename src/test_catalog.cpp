#include <iostream>

#include "catalog.h"
#include "catalog_serializer.h"


int main()
{
    // ==========================================
    // Create original Catalog
    // ==========================================

    Catalog catalog;


    // Students
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


    // Employees
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


    // ==========================================
    // Serialize
    // ==========================================

    std::vector<uint8_t> data =
        CatalogSerializer::serialize(
            catalog
        );


    std::cout
        << "Serialized catalog size: "
        << data.size()
        << " bytes\n";


    // ==========================================
    // Create empty Catalog
    // ==========================================

    Catalog loaded;


    // ==========================================
    // Deserialize
    // ==========================================

    CatalogSerializer::deserialize(
        data,
        loaded
    );


    // ==========================================
    // Verify
    // ==========================================

    std::cout
        << "\nLoaded tables:\n";

    for (
        const std::string& name :
        loaded.listTables()
    )
    {
        std::cout
            << "- "
            << name
            << "\n";

        std::cout
            << "  First page: "
            << loaded.getFirstPage(name)
            << "\n";
    }


    std::cout
        << "\nCatalog serialization test completed.\n";


    return 0;
}