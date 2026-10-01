#include <iostream>

#include "pager.h"
#include "buffer_pool.h"
#include "record/schema.h"
#include "record/row.h"
#include "record/value.h"
#include "generic_table.h"

void printRow(const Row& row)
{
    std::cout
        << row.getValue(0).asInt()
        << " | "
        << row.getValue(1).asString()
        << " | "
        << row.getValue(2).asInt()
        << " | "
        << row.getValue(3).asDouble()
        << "\n";
}

int main()
{
    // ==========================================
    // Open EXISTING database
    // ==========================================

    Pager pager("data/database.db");

    BufferPool bufferPool(pager);


    // ==========================================
    // Recreate the same schema
    // ==========================================

    Schema schema;

    schema.addColumn(
        "id",
        DataType::INT
    );

    schema.addColumn(
        "name",
        DataType::VARCHAR
    );

    schema.addColumn(
        "age",
        DataType::INT
    );

    schema.addColumn(
        "cgpa",
        DataType::DOUBLE
    );


    // ==========================================
    // Open table
    // ==========================================

    GenericTable table(
        pager,
        bufferPool,
        schema
    );


    // ==========================================
    // Check persisted rows
    // ==========================================

    std::cout
        << "Reading database after restart...\n\n";

    int total = table.size();

    std::cout
        << "Total rows found: "
        << total
        << "\n";


    // ==========================================
    // Verify selected rows
    // ==========================================

    std::cout
        << "\nFirst row:\n";

    printRow(
        table.get(0)
    );


    std::cout
        << "\nRow 100:\n";

    printRow(
        table.get(99)
    );


    std::cout
        << "\nRow 200:\n";

    printRow(
        table.get(199)
    );


    std::cout
        << "\nLast row:\n";

    printRow(
        table.get(299)
    );


    std::cout
        << "\nPersistence test completed.\n";


    return 0;
}