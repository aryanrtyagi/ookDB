#include <iostream>

#include "pager.h"
#include "buffer_pool.h"
#include "page_allocator.h"
#include "catalog.h"
#include "generic_table.h"

#include "record/row.h"
#include "record/value.h"

int main()
{
    std::cout << "========================================\n";
    std::cout << "       AryDB Persistence Write Test\n";
    std::cout << "========================================\n\n";

    Pager pager("data/database.db");

    BufferPool bufferPool(pager);

    PageAllocator pageAllocator(pager);

    Catalog catalog(pageAllocator);


    // =========================
    // Create students table
    // =========================

    Schema studentsSchema;

    studentsSchema.addColumn("id", DataType::INT);
    studentsSchema.addColumn("name", DataType::VARCHAR);
    studentsSchema.addColumn("age", DataType::INT);

    if (!catalog.createTable("students", studentsSchema))
    {
        std::cout << "Failed to create students table.\n";
        return 1;
    }

    int studentsPage =
        catalog.getFirstPage("students");


    // =========================
    // Create employees table
    // =========================

    Schema employeeSchema;

    employeeSchema.addColumn("id", DataType::INT);
    employeeSchema.addColumn("name", DataType::VARCHAR);
    employeeSchema.addColumn("salary", DataType::DOUBLE);

    if (!catalog.createTable("employees", employeeSchema))
    {
        std::cout << "Failed to create employees table.\n";
        return 1;
    }

    int employeesPage =
        catalog.getFirstPage("employees");


    // =========================
    // Create tables
    // =========================

    GenericTable students(
        pager,
        bufferPool,
        pageAllocator,
        studentsSchema,
        studentsPage
    );

    GenericTable employees(
        pager,
        bufferPool,
        pageAllocator,
        employeeSchema,
        employeesPage
    );


    // =========================
    // Insert students
    // =========================

    Row student1;

    student1.addValue(Value(1));
    student1.addValue(Value("Alice"));
    student1.addValue(Value(21));

    Row student2;

    student2.addValue(Value(2));
    student2.addValue(Value("Bob"));
    student2.addValue(Value(22));


    students.insert(student1);
    students.insert(student2);


    // =========================
    // Insert employee
    // =========================

    Row employee;

    employee.addValue(Value(101));
    employee.addValue(Value("Charlie"));
    employee.addValue(Value(75000.0));

    employees.insert(employee);


    // =========================
    // Flush Buffer Pool
    // =========================

    bufferPool.flushAll();


    // =========================
    // Print information needed
    // by the read test
    // =========================

    std::cout << "Students first page: "
              << studentsPage
              << "\n";

    std::cout << "Employees first page: "
              << employeesPage
              << "\n";

    std::cout << "Students rows: "
              << students.size()
              << "\n";

    std::cout << "Employees rows: "
              << employees.size()
              << "\n";


    std::cout << "\nData written successfully.\n";
    std::cout << "Close this program and run the READ test.\n";

    return 0;
}