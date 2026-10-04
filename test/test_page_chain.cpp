#include <iostream>
#include <string>

#include "pager.h"
#include "buffer_pool.h"
#include "page_allocator.h"
#include "catalog.h"
#include "generic_table.h"

#include "record/row.h"
#include "record/value.h"
#include "storage/record_page.h"


int main()
{
    std::cout << "========================================\n";
    std::cout << "        AryDB Page Chain Test\n";
    std::cout << "========================================\n\n";


    // ============================================================
    // Initialize AryDB
    // ============================================================

    Pager pager("data/database.db");

    BufferPool bufferPool(pager);

    PageAllocator pageAllocator(pager);

    Catalog catalog(pageAllocator);


    // ============================================================
    // Create students table
    // ============================================================

    Schema studentsSchema;

    studentsSchema.addColumn("id", DataType::INT);
    studentsSchema.addColumn("name", DataType::VARCHAR);
    studentsSchema.addColumn("age", DataType::INT);

    bool studentsCreated =
        catalog.createTable(
            "students",
            studentsSchema
        );

    if (!studentsCreated)
    {
        std::cout << "Failed to create students table.\n";
        return 1;
    }

    int studentsPage =
        catalog.getFirstPage("students");


    // ============================================================
    // Create employees table
    // ============================================================

    Schema employeeSchema;

    employeeSchema.addColumn("id", DataType::INT);
    employeeSchema.addColumn("name", DataType::VARCHAR);
    employeeSchema.addColumn("salary", DataType::DOUBLE);

    bool employeeCreated =
        catalog.createTable(
            "employees",
            employeeSchema
        );

    if (!employeeCreated)
    {
        std::cout << "Failed to create employees table.\n";
        return 1;
    }

    int employeesPage =
        catalog.getFirstPage("employees");


    // ============================================================
    // Create GenericTable objects
    // ============================================================

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


    std::cout << "Students start -> Page "
              << studentsPage
              << "\n";

    std::cout << "Employees start -> Page "
              << employeesPage
              << "\n\n";


    // ============================================================
    // Insert many students
    // ============================================================

    std::cout << "Inserting 300 students...\n";

    for (int i = 1; i <= 300; i++)
    {
        Row student;

        student.addValue(Value(i));

        student.addValue(
            Value(
                std::string("Student") +
                std::to_string(i)
            )
        );

        student.addValue(
            Value(18 + (i % 10))
        );

        if (!students.insert(student))
        {
            std::cout << "FAILED to insert student "
                      << i
                      << "\n";

            return 1;
        }
    }

    std::cout << "300 students inserted successfully.\n";


    // ============================================================
    // Insert employee
    // ============================================================

    Row employee;

    employee.addValue(Value(1001));
    employee.addValue(Value("Employee1"));
    employee.addValue(Value(75000.0));

    if (!employees.insert(employee))
    {
        std::cout << "FAILED to insert employee.\n";
        return 1;
    }

    std::cout << "Employee inserted successfully.\n\n";


    // ============================================================
    // Check row counts
    // ============================================================

    std::cout << "Students rows: "
              << students.size()
              << "\n";

    std::cout << "Employees rows: "
              << employees.size()
              << "\n\n";


    // ============================================================
    // Print students page chain
    // ============================================================

    std::cout << "========================================\n";
    std::cout << "        Students Page Chain\n";
    std::cout << "========================================\n";

    int pageNumber = studentsPage;

    int studentPageCount = 0;

    while (pageNumber != -1)
    {
        char* pageData =
            bufferPool.getPage(pageNumber);

        if (pageData == nullptr)
        {
            std::cout << "ERROR: Could not load Page "
                      << pageNumber
                      << "\n";

            return 1;
        }

        RecordPage page(pageData);

        std::cout << "Page "
                  << pageNumber
                  << " | Rows: "
                  << page.size()
                  << " | Next: "
                  << page.getNextPage()
                  << "\n";

        studentPageCount++;

        pageNumber =
            page.getNextPage();
    }


    // ============================================================
    // Print employees page chain
    // ============================================================

    std::cout << "\n========================================\n";
    std::cout << "        Employees Page Chain\n";
    std::cout << "========================================\n";

    pageNumber = employeesPage;

    while (pageNumber != -1)
    {
        char* pageData =
            bufferPool.getPage(pageNumber);

        if (pageData == nullptr)
        {
            std::cout << "ERROR: Could not load Page "
                      << pageNumber
                      << "\n";

            return 1;
        }

        RecordPage page(pageData);

        std::cout << "Page "
                  << pageNumber
                  << " | Rows: "
                  << page.size()
                  << " | Next: "
                  << page.getNextPage()
                  << "\n";

        pageNumber =
            page.getNextPage();
    }


    // ============================================================
    // Test reading across the page chain
    // ============================================================

    std::cout << "\n========================================\n";
    std::cout << "        Cross-Page Read Test\n";
    std::cout << "========================================\n";


    // First row
    Row firstStudent =
        students.get(0);

    std::cout << "First row:\n";

    std::cout
        << firstStudent.getValue(0).asInt()
        << " | "
        << firstStudent.getValue(1).asString()
        << " | "
        << firstStudent.getValue(2).asInt()
        << "\n";


    // Middle row
    Row middleStudent =
        students.get(150);

    std::cout << "\nMiddle row:\n";

    std::cout
        << middleStudent.getValue(0).asInt()
        << " | "
        << middleStudent.getValue(1).asString()
        << " | "
        << middleStudent.getValue(2).asInt()
        << "\n";


    // Last row
    Row lastStudent =
        students.get(299);

    std::cout << "\nLast row:\n";

    std::cout
        << lastStudent.getValue(0).asInt()
        << " | "
        << lastStudent.getValue(1).asString()
        << " | "
        << lastStudent.getValue(2).asInt()
        << "\n";


    // ============================================================
    // Final verification
    // ============================================================

    std::cout << "\n========================================\n";
    std::cout << "             FINAL RESULT\n";
    std::cout << "========================================\n";

    std::cout << "Students rows       : "
              << students.size()
              << "\n";

    std::cout << "Employees rows      : "
              << employees.size()
              << "\n";

    std::cout << "Students page count : "
              << studentPageCount
              << "\n";


    if (students.size() == 300 &&
        employees.size() == 1 &&
        studentPageCount > 1)
    {
        std::cout << "\nSUCCESS!\n";
        std::cout << "Students successfully use multiple pages.\n";
        std::cout << "Employees remain on their own page chain.\n";
    }
    else
    {
        std::cout << "\nTEST FAILED.\n";
    }


    std::cout << "\n========================================\n";
    std::cout << "          Page Chain Test Complete\n";
    std::cout << "========================================\n";


    return 0;
}