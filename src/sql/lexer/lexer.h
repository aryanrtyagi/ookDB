#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>

// ============================================================
// Token Types
// ============================================================

enum class TokenType
{
    // Keywords
    SELECT,
    FROM,
    WHERE,
    INSERT,
    INTO,
    VALUES,
    UPDATE,
    SET,
    DELETE,
    CREATE,
    TABLE,
    LIMIT,

    // Data
    IDENTIFIER,
    NUMBER,
    STRING,

    // Operators
    EQUAL,
    GREATER,
    LESS,
    GREATER_EQUAL,
    LESS_EQUAL,
    NOT_EQUAL,

    // Symbols
    STAR,
    LEFT_PAREN,
    RIGHT_PAREN,
    COMMA,
    SEMICOLON,

    // Special
    END,
    INVALID
};


// ============================================================
// Token
// ============================================================

struct Token
{
    TokenType type;
    std::string value;
};


// ============================================================
// Lexer
// ============================================================

class Lexer
{
private:
    std::string input;
    size_t position;

    char currentChar() const;

    void skipWhitespace();

    Token readIdentifierOrKeyword();

    Token readNumber();

    Token readString();

    Token readOperatorOrSymbol();

public:

    explicit Lexer(
        const std::string& input
    );

    Token nextToken();

    std::vector<Token> tokenize();
};


// ============================================================
// Helper
// ============================================================

std::string tokenTypeToString(
    TokenType type
);

#endif