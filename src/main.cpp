#include <iostream>

#include "pager.h"
#include "page_allocator.h"
#include "catalog.h"


int main()
{
    // ==========================================
    // Storage components
    // ==========================================

    Pager pager("data/database.db");

    PageAllocator pageAllocator(pager);

    Catalog catalog(pageAllocator);


    // ==========================================
    // Show page state
    // ==========================================

    std::cout
        << "Page 0 -> RESERVED FOR CATALOG\n";

    std::cout
        << "Next available page -> "
        << pageAllocator.getNextPageNumber()
        << "\n\n";


    // ==========================================
    // Create students schema
    // ==========================================

    Schema studentsSchema;

    studentsSchema.addColumn(
        "id",
        DataType::INT
    );

    studentsSchema.addColumn(
        "name",
        DataType::VARCHAR
    );

    studentsSchema.addColumn(
        "age",
        DataType::INT
    );


    // ==========================================
    // Create students table
    // ==========================================

    bool created =
        catalog.createTable(
            "students",
            studentsSchema
        );

    std::cout
        << "Create students: "
        << (created ? "SUCCESS" : "FAILED")
        << "\n";


    if (created)
    {
        std::cout
            << "students -> Page "
            << catalog.getFirstPage("students")
            << "\n";
    }


    // ==========================================
    // Check table
    // ==========================================

    std::cout
        << "students exists: "
        << (
            catalog.tableExists("students")
            ? "YES"
            : "NO"
        )
        << "\n";


    // ==========================================
    // Try duplicate
    // ==========================================

    bool duplicate =
        catalog.createTable(
            "students",
            studentsSchema
        );

    std::cout
        << "Create students again: "
        << (
            duplicate
            ? "SUCCESS"
            : "REJECTED"
        )
        << "\n";


    // ==========================================
    // Create employees schema
    // ==========================================

    Schema employeeSchema;

    employeeSchema.addColumn(
        "id",
        DataType::INT
    );

    employeeSchema.addColumn(
        "name",
        DataType::VARCHAR
    );

    employeeSchema.addColumn(
        "salary",
        DataType::DOUBLE
    );


    // ==========================================
    // Create employees
    // ==========================================

    bool employeeCreated =
        catalog.createTable(
            "employees",
            employeeSchema
        );

    std::cout
        << "Create employees: "
        << (
            employeeCreated
            ? "SUCCESS"
            : "FAILED"
        )
        << "\n";

    if (employeeCreated)
    {
        std::cout
            << "employees -> Page "
            << catalog.getFirstPage("employees")
            << "\n";
    }


    // ==========================================
    // List tables
    // ==========================================

    std::cout
        << "\nTables:\n";

    for (const std::string& name :
         catalog.listTables())
    {
        std::cout
            << "- "
            << name
            << "\n";
    }


    // ==========================================
    // Show final page state
    // ==========================================

    std::cout
        << "\nPage 0 -> CATALOG\n";

    for (const auto& table :
         catalog.getTables())
    {
        std::cout
            << "Page "
            << table.firstPage
            << " -> "
            << table.name
            << "\n";
    }

    std::cout
        << "Page "
        << pageAllocator.getNextPageNumber()
        << " -> NEXT AVAILABLE\n";


    // ==========================================
    // Get schema
    // ==========================================

    const Schema& schema =
        catalog.getSchema("students");

    std::cout
        << "\nstudents schema loaded successfully.\n";


    return 0;
}