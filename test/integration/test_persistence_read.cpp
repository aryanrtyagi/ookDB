#include <iostream>

#include "pager.h"
#include "buffer_pool.h"
#include "page_allocator.h"
#include "generic_table.h"

#include "record/row.h"
#include "record/value.h"

int main()
{
    std::cout << "========================================\n";
    std::cout << "       AryDB Persistence Read Test\n";
    std::cout << "========================================\n\n";

    // =========================
    // Open existing database
    // =========================

    Pager pager("data/database.db");

    BufferPool bufferPool(pager);

    PageAllocator pageAllocator(pager);

    // =========================
    // These pages were printed
    // by the WRITE test.
    //
    // IMPORTANT:
    // Replace these with the actual
    // page numbers printed by your
    // WRITE test.
    // =========================

    int studentsPage = 1;
    int employeesPage = 2;

    // =========================
    // Recreate schemas
    // =========================

    Schema studentsSchema;

    studentsSchema.addColumn("id", DataType::INT);
    studentsSchema.addColumn("name", DataType::VARCHAR);
    studentsSchema.addColumn("age", DataType::INT);

    Schema employeeSchema;

    employeeSchema.addColumn("id", DataType::INT);
    employeeSchema.addColumn("name", DataType::VARCHAR);
    employeeSchema.addColumn("salary", DataType::DOUBLE);

    // =========================
    // Reopen tables
    // =========================

    GenericTable students(
        pager,
        bufferPool,
        pageAllocator,
        studentsSchema,
        studentsPage);

    GenericTable employees(
        pager,
        bufferPool,
        pageAllocator,
        employeeSchema,
        employeesPage);

    // =========================
    // Read persisted rows
    // =========================

    std::cout << "Students rows after restart: "
              << students.size()
              << "\n";

    std::cout << "Employees rows after restart: "
              << employees.size()
              << "\n";

    // =========================
    // Read student
    // =========================

    Row student =
        students.get(0);

    std::cout << "\nStudent after restart:\n";

    std::cout
        << student.getValue(0).asInt()
        << " | "
        << student.getValue(1).asString()
        << " | "
        << student.getValue(2).asInt()
        << "\n";

    // =========================
    // Read employee
    // =========================

    Row employee =
        employees.get(0);

    std::cout << "\nEmployee after restart:\n";

    std::cout
        << employee.getValue(0).asInt()
        << " | "
        << employee.getValue(1).asString()
        << " | "
        << employee.getValue(2).asDouble()
        << "\n";

    // =========================
    // Final verification
    // =========================

    if (students.size() == 300 &&
        employees.size() == 1 &&
        student.getValue(1).asString() == "Student1" &&
        employee.getValue(1).asString() == "Employee1")
    {
        std::cout << "\n========================================\n";
        std::cout << "       PERSISTENCE TEST PASSED\n";
        std::cout << "========================================\n";
    }
    else
    {
        std::cout << "\n========================================\n";
        std::cout << "       PERSISTENCE TEST FAILED\n";
        std::cout << "========================================\n";
    }

    return 0;
}