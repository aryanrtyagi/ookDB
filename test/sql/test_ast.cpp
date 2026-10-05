#include <iostream>

#include "../../src/sql/ast/query.h"

int main()
{
    SelectQuery query;

    query.tableName = "students";

    query.columns = {
        "name",
        "age"
    };

    query.hasCondition = true;

    query.condition.column = "age";

    query.condition.op =
        ComparisonOperator::GREATER;

    query.condition.value = {
        LiteralType::NUMBER,
        "20"
    };

    query.limit = 10;


    std::cout
        << "Table: "
        << query.tableName
        << "\n";

    std::cout
        << "Columns:\n";

    for (const std::string& column :
         query.columns)
    {
        std::cout
            << "  "
            << column
            << "\n";
    }

    std::cout
        << "Condition: "
        << query.condition.column
        << " > "
        << query.condition.value.value
        << "\n";

    std::cout
        << "Limit: "
        << query.limit
        << "\n";

    return 0;
}