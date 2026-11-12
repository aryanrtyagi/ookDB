
#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>
#include <cstddef>

enum class TokenType
{
    SELECT, FROM, WHERE,
    INSERT, INTO, VALUES,
    UPDATE, SET, DELETE,
    CREATE, TABLE, INDEX, ON,
    LIMIT,

    DISTINCT, ORDER, BY, ASC, DESC,
    GROUP, HAVING,
    INNER, LEFT, JOIN,
    AS,
    COUNT, SUM, AVG, MIN, MAX,

    SHOW, TABLES, DESCRIBE, DROP, ALTER,
    ADD, COLUMN, RENAME, TO,

    AND, OR, NOT, IN, BETWEEN, LIKE,

    IDENTIFIER, NUMBER, STRING,

    EQUAL, GREATER, LESS,
    GREATER_EQUAL, LESS_EQUAL, NOT_EQUAL,

    STAR, LEFT_PAREN, RIGHT_PAREN,
    COMMA, SEMICOLON, DOT,

    END, INVALID
};

struct Token
{
    TokenType type;
    std::string value;
};

class Lexer
{
private:
    std::string input;
    std::size_t position;

    char currentChar() const;
    void skipWhitespace();
    Token readIdentifierOrKeyword();
    Token readNumber();
    Token readString();
    Token readOperatorOrSymbol();

public:
    explicit Lexer(const std::string& input);

    Token nextToken();
    std::vector<Token> tokenize();
};

std::string tokenTypeToString(TokenType type);

#endif
