#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <string>

#include "../lexer/lexer.h"
#include "../ast/query.h"

class Parser
{
private:
    std::vector<Token> tokens;
    size_t position;

    const Token& currentToken() const;

    bool check(TokenType type) const;

    Token consume();

    bool match(TokenType type);

    Token expect(TokenType type);

    std::string parseIdentifier();

    Literal parseLiteral();

    Condition parseCondition();

    std::vector<std::string> parseColumnList();

    SelectQuery parseSelect();

public:
    explicit Parser(
        const std::vector<Token>& tokens
    );

    SelectQuery parse();
};

#endif