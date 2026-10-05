#include <iostream>

#include "../../src/sql/lexer/lexer.h"

int main()
{
    std::string query =
        "SELECT * FROM students WHERE age >= 20;";

    Lexer lexer(query);

    std::vector<Token> tokens =
        lexer.tokenize();

    for (const Token& token : tokens)
    {
        std::cout
            << tokenTypeToString(token.type)
            << " -> ["
            << token.value
            << "]\n";
    }

    return 0;
}