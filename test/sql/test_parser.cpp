#include <iostream>

#include "../../src/sql/lexer/lexer.h"
#include "../../src/sql/parser/parser.h"

int main()
{
    std::string query =
        "SELECT name, age "
        "FROM students "
        "WHERE age > 20 "
        "LIMIT 10;";

    Lexer lexer(query);

    std::vector<Token> tokens =
        lexer.tokenize();

    Parser parser(tokens);

    SelectQuery result =
        parser.parse();

    std::cout
        << "Table: "
        << result.tableName
        << "\n";

    std::cout
        << "Columns:\n";

    for (const std::string& column :
         result.columns)
    {
        std::cout
            << "  "
            << column
            << "\n";
    }

    if (result.hasCondition)
    {
        std::cout
            << "Condition: "
            << result.condition.column
            << " ";

        switch (result.condition.op)
        {
            case ComparisonOperator::EQUAL:
                std::cout << "=";
                break;

            case ComparisonOperator::NOT_EQUAL:
                std::cout << "!=";
                break;

            case ComparisonOperator::GREATER:
                std::cout << ">";
                break;

            case ComparisonOperator::LESS:
                std::cout << "<";
                break;

            case ComparisonOperator::GREATER_EQUAL:
                std::cout << ">=";
                break;

            case ComparisonOperator::LESS_EQUAL:
                std::cout << "<=";
                break;
        }

        std::cout
            << " "
            << result.condition.value.value
            << "\n";
    }

    std::cout
        << "Limit: "
        << result.limit
        << "\n";

    return 0;
}