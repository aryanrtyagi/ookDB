#include <iostream>

#include "../../src/sql/lexer/lexer.h"
#include "../../src/sql/parser/parser.h"

int main()
{
    std::string input =
        "INSERT INTO employees "
        "VALUES (1, 'Aryan', 75000);";

    Lexer lexer(input);

    std::vector<Token> tokens =
        lexer.tokenize();

    Parser parser(tokens);

    Query query =
        parser.parse();

    if (query.type != QueryType::INSERT)
    {
        std::cout
            << "WRONG QUERY TYPE\n";

        return 1;
    }

    InsertQuery insertQuery =
        query.insert;

    std::cout
        << "Table: "
        << insertQuery.tableName
        << "\n";

    for (const Literal& value :
         insertQuery.values)
    {
        std::cout
            << value.value
            << "\n";
    }

    std::cout
        << "INSERT PARSER TEST PASSED\n";

    return 0;
}