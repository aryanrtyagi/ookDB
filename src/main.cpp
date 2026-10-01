#include <iostream>
#include <string>

#include "record/value.h"
#include "record/column.h"
#include "record/schema.h"
#include "record/row.h"

int main()
{
    Schema schema;

    schema.addColumn("id", DataType::INT);
    schema.addColumn("name", DataType::VARCHAR);
    schema.addColumn("age", DataType::INT);
    schema.addColumn("salary", DataType::DOUBLE);

    std::cout << "Schema created!\n";
    std::cout << "Number of columns: "
              << schema.columnCount() << "\n\n";

    std::cout << "Table Structure:\n";

    for (int i = 0; i < schema.columnCount(); i++)
    {
        const Column& column = schema.getColumn(i);

        std::cout << "Column " << i
                  << ": "
                  << column.getName()
                  << "\n";
    }

    Row row;

    row.addValue(Value(1));
    row.addValue(Value(std::string("Aryan")));
    row.addValue(Value(21));
    row.addValue(Value(85000.0));

    std::cout << "\nRow Data:\n";

    std::cout << "ID     : "
              << row.getValue(0).asInt()
              << "\n";

    std::cout << "Name   : "
              << row.getValue(1).asString()
              << "\n";

    std::cout << "Age    : "
              << row.getValue(2).asInt()
              << "\n";

    std::cout << "Salary : "
              << row.getValue(3).asDouble()
              << "\n";

    return 0;
}