#include <iostream>

#include "pager.h"
#include "page_allocator.h"
#include "catalog.h"
#include "record/schema.h"
#include "record/column.h"

int main()
{
    Pager pager("data/database.db");

    PageAllocator allocator(pager);

    Catalog catalog;


    std::cout << "========================================\n";
    std::cout << "     AryDB Catalog + PageAllocator\n";
    std::cout << "========================================\n\n";


    // -----------------------------------------
    // Page 0
    // -----------------------------------------

    std::cout << "Page 0 -> RESERVED FOR CATALOG\n\n";


    // -----------------------------------------
    // Create students schema
    // -----------------------------------------

    Schema studentSchema;

    studentSchema.addColumn("id", DataType::INT);
    studentSchema.addColumn("name", DataType::VARCHAR);
    studentSchema.addColumn("age", DataType::INT);


    // -----------------------------------------
    // Create students table
    // -----------------------------------------

    std::cout << "Creating table: students\n";

    catalog.createTable(
        "students",
        studentSchema,
        allocator
    );

    int studentsPage =
        catalog.getFirstPage("students");

    std::cout << "students -> Page "
              << studentsPage
              << "\n\n";


    // -----------------------------------------
    // Create employees schema
    // -----------------------------------------

    Schema employeeSchema;

    employeeSchema.addColumn("id", DataType::INT);
    employeeSchema.addColumn("name", DataType::VARCHAR);
    employeeSchema.addColumn("salary", DataType::DOUBLE);


    // -----------------------------------------
    // Create employees table
    // -----------------------------------------

    std::cout << "Creating table: employees\n";

    catalog.createTable(
        "employees",
        employeeSchema,
        allocator
    );

    int employeesPage =
        catalog.getFirstPage("employees");

    std::cout << "employees -> Page "
              << employeesPage
              << "\n\n";


    // -----------------------------------------
    // Final state
    // -----------------------------------------

    std::cout << "========================================\n";
    std::cout << "             FINAL STATE\n";
    std::cout << "========================================\n\n";

    std::cout << "Page 0 -> CATALOG\n";

    for (const auto& table : catalog.getTables())
    {
        std::cout
            << "Page "
            << table.firstPage
            << " -> "
            << table.name
            << "\n";
    }

    std::cout << "\nPage "
              << allocator.getNextPageNumber()
              << " -> NEXT AVAILABLE\n";


    std::cout << "\n========================================\n";
    std::cout << "          Test Completed\n";
    std::cout << "========================================\n";

    return 0;
}