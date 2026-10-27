#include <iostream>

#include "../../src/sql/lexer/lexer.h"
#include "../../src/sql/parser/parser.h"

int main()
{
    std::string input =
        "CREATE TABLE employees (id INT, name VARCHAR, salary DOUBLE);";

    Lexer lexer(input);

    std::vector<Token> tokens =
        lexer.tokenize();

    Parser parser(tokens);

    Query query =
        parser.parse();

    if (query.type != QueryType::CREATE_TABLE)
    {
        std::cout << "WRONG QUERY TYPE\n";
        return 1;
    }

    CreateTableQuery createQuery =
        query.createTable;

    std::cout << "Table: "
              << createQuery.tableName
              << "\n";

    for (const auto& column : createQuery.columns)
    {
        std::cout << column.name
                  << " : "
                  << column.type
                  << "\n";
    }

    return 0;
}