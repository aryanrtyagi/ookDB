#include <iostream>

#include "catalog.h"


int main()
{
    Catalog catalog;


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
    // Create table
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
    // Create another table
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


    catalog.createTable(
        "employees",
        employeeSchema
    );


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
    // Get schema
    // ==========================================

    const Schema& schema =
        catalog.getSchema("students");

    std::cout
        << "\nstudents schema loaded successfully.\n";


    return 0;
}