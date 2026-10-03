#include <iostream>

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
    std::cout << "       AryDB Multiple Table Test\n";
    std::cout << "========================================\n\n";

    Pager pager("data/database.db");
    BufferPool bufferPool(pager);
    PageAllocator pageAllocator(pager);
    Catalog catalog(pageAllocator);

    std::cout << "Page 0 -> CATALOG\n";

    std::cout << "Next available page -> Page "
              << pageAllocator.getNextPageNumber()
              << "\n\n";

    // =========================
    // Students table
    // =========================

    Schema studentsSchema;

    studentsSchema.addColumn("id", DataType::INT);
    studentsSchema.addColumn("name", DataType::VARCHAR);
    studentsSchema.addColumn("age", DataType::INT);

    bool studentsCreated =
        catalog.createTable("students", studentsSchema);

    if (!studentsCreated)
    {
        std::cout << "Failed to create students table.\n";
        return 1;
    }

    int studentsPage =
        catalog.getFirstPage("students");

    std::cout << "students  -> Page "
              << studentsPage
              << "\n";


    // =========================
    // Employees table
    // =========================

    Schema employeeSchema;

    employeeSchema.addColumn("id", DataType::INT);
    employeeSchema.addColumn("name", DataType::VARCHAR);
    employeeSchema.addColumn("salary", DataType::DOUBLE);

    bool employeeCreated =
        catalog.createTable("employees", employeeSchema);

    if (!employeeCreated)
    {
        std::cout << "Failed to create employees table.\n";
        return 1;
    }

    int employeesPage =
        catalog.getFirstPage("employees");

    std::cout << "employees -> Page "
              << employeesPage
              << "\n";

    std::cout << "next page -> Page "
              << pageAllocator.getNextPageNumber()
              << "\n\n";


    // =========================
    // Generic tables
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

    bool studentInsert1 =
        students.insert(student1);

    bool studentInsert2 =
        students.insert(student2);

    std::cout << "Student insert: ";

    if (studentInsert1 && studentInsert2)
        std::cout << "SUCCESS\n";
    else
        std::cout << "FAILED\n";


    // =========================
    // Insert employee
    // =========================

    Row employee1;

    employee1.addValue(Value(101));
    employee1.addValue(Value("Charlie"));
    employee1.addValue(Value(75000.0));

    bool employeeInsert =
        employees.insert(employee1);

    std::cout << "Employee insert: ";

    if (employeeInsert)
        std::cout << "SUCCESS\n";
    else
        std::cout << "FAILED\n";


    // =========================
    // Row counts
    // =========================

    std::cout << "\nStudents rows: "
              << students.size()
              << "\n";

    std::cout << "Employees rows: "
              << employees.size()
              << "\n";


    // =========================
    // Read student
    // =========================

    Row firstStudent =
        students.get(0);

    std::cout << "\nStudent first row:\n";

    std::cout
        << firstStudent.getValue(0).asInt()
        << " | "
        << firstStudent.getValue(1).asString()
        << " | "
        << firstStudent.getValue(2).asInt()
        << "\n";


    // =========================
    // Read employee
    // =========================

    Row firstEmployee =
        employees.get(0);

    std::cout << "\nEmployee first row:\n";

    std::cout
        << firstEmployee.getValue(0).asInt()
        << " | "
        << firstEmployee.getValue(1).asString()
        << " | "
        << firstEmployee.getValue(2).asDouble()
        << "\n";


    // =========================
    // Final state
    // =========================

    std::cout << "\n========================================\n";
    std::cout << "             FINAL STATE\n";
    std::cout << "========================================\n";

    std::cout << "Page 0 -> CATALOG\n";

    std::cout << "Page "
              << studentsPage
              << " -> students\n";

    std::cout << "Page "
              << employeesPage
              << " -> employees\n";

    std::cout << "Page "
              << pageAllocator.getNextPageNumber()
              << " -> NEXT AVAILABLE\n";


    // =========================
    // Students page chain
    // =========================

    std::cout << "\nStudents page chain:\n";

    int pageNumber = studentsPage;

    while (pageNumber != -1)
    {
        char* pageData =
            bufferPool.getPage(pageNumber);

        if (pageData == nullptr)
            break;

        RecordPage page(pageData);

        std::cout << "Page "
                  << pageNumber
                  << " -> ";

        int nextPage =
            page.getNextPage();

        if (nextPage == -1)
            std::cout << "END\n";
        else
            std::cout << "Page "
                      << nextPage
                      << "\n";

        pageNumber = nextPage;
    }


    // =========================
    // Employees page chain
    // =========================

    std::cout << "\nEmployees page chain:\n";

    pageNumber = employeesPage;

    while (pageNumber != -1)
    {
        char* pageData =
            bufferPool.getPage(pageNumber);

        if (pageData == nullptr)
            break;

        RecordPage page(pageData);

        std::cout << "Page "
                  << pageNumber
                  << " -> ";

        int nextPage =
            page.getNextPage();

        if (nextPage == -1)
            std::cout << "END\n";
        else
            std::cout << "Page "
                      << nextPage
                      << "\n";

        pageNumber = nextPage;
    }


    std::cout << "\n========================================\n";
    std::cout << "       Multiple Table Test Complete\n";
    std::cout << "========================================\n";

    return 0;
}