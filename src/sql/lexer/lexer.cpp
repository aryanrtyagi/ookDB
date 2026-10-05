#include "lexer.h"
#include <cctype>

// ============================================================
// Constructor
// ============================================================

Lexer::Lexer(
    const std::string& input)
    : input(input),
      position(0)
{
}

// ============================================================
// Current Character
// ============================================================

char Lexer::currentChar() const
{
    if (position >= input.size())
    {
        return '\0';
    }

    return input[position];
}


// ============================================================
// Skip Whitespace
// ============================================================

void Lexer::skipWhitespace()
{
    while (
        position < input.size() &&
        std::isspace(
            static_cast<unsigned char>(
                input[position])))
    {
        position++;
    }
}

// ============================================================
// Read Identifier / Keyword
// ============================================================

Token Lexer::readIdentifierOrKeyword()
{
    std::string value;

    while (
        position < input.size() &&
        (
            std::isalnum(
                static_cast<unsigned char>(
                    input[position]))
            ||
            input[position] == '_'
        ))
    {
        value += input[position];
        position++;
    }


    // Convert to uppercase for keyword comparison.
    std::string upper = value;

    for (char& c : upper)
    {
        c = static_cast<char>(
            std::toupper(
                static_cast<unsigned char>(c)));
    }


    if (upper == "SELECT")
        return {TokenType::SELECT, value};

    if (upper == "FROM")
        return {TokenType::FROM, value};

    if (upper == "WHERE")
        return {TokenType::WHERE, value};

    if (upper == "INSERT")
        return {TokenType::INSERT, value};

    if (upper == "INTO")
        return {TokenType::INTO, value};

    if (upper == "VALUES")
        return {TokenType::VALUES, value};

    if (upper == "UPDATE")
        return {TokenType::UPDATE, value};

    if (upper == "SET")
        return {TokenType::SET, value};

    if (upper == "DELETE")
        return {TokenType::DELETE, value};

    if (upper == "CREATE")
        return {TokenType::CREATE, value};

    if (upper == "TABLE")
        return {TokenType::TABLE, value};

    if (upper == "LIMIT")
        return {TokenType::LIMIT, value};


    return {
        TokenType::IDENTIFIER,
        value
    };
}


// ============================================================
// Read Number
// ============================================================

Token Lexer::readNumber()
{
    std::string value;

    bool hasDecimal = false;

    while (position < input.size())
    {
        char c = input[position];

        if (std::isdigit(
                static_cast<unsigned char>(c)))
        {
            value += c;
            position++;
        }
        else if (c == '.' && !hasDecimal)
        {
            hasDecimal = true;
            value += c;
            position++;
        }
        else
        {
            break;
        }
    }

    return {
        TokenType::NUMBER,
        value
    };
}

// ============================================================
// Read String
// ============================================================

Token Lexer::readString()
{
    char quote = input[position];

    // Skip opening quote.
    position++;

    std::string value;

    while (
        position < input.size() &&
        input[position] != quote)
    {
        value += input[position];
        position++;
    }

    // Skip closing quote.
    if (
        position < input.size() &&
        input[position] == quote)
    {
        position++;
    }

    return {
        TokenType::STRING,
        value
    };
}

// ============================================================
// Read Operators / Symbols
// ============================================================

Token Lexer::readOperatorOrSymbol()
{
    char c = currentChar();


    // ----------------------------------------
    // =
    // ----------------------------------------

    if (c == '=')
    {
        position++;

        return {
            TokenType::EQUAL,
            "="
        };
    }


    // ----------------------------------------
    // >
    // ----------------------------------------

    if (c == '>')
    {
        position++;

        if (currentChar() == '=')
        {
            position++;

            return {
                TokenType::GREATER_EQUAL,
                ">="
            };
        }

        return {
            TokenType::GREATER,
            ">"
        };
    }


    // ----------------------------------------
    // <
    // ----------------------------------------

    if (c == '<')
    {
        position++;

        if (currentChar() == '=')
        {
            position++;

            return {
                TokenType::LESS_EQUAL,
                "<="
            };
        }

        return {
            TokenType::LESS,
            "<"
        };
    }


    // ----------------------------------------
    // !
    // ----------------------------------------

    if (c == '!')
    {
        position++;

        if (currentChar() == '=')
        {
            position++;

            return {
                TokenType::NOT_EQUAL,
                "!="
            };
        }

        return {
            TokenType::INVALID,
            "!"
        };
    }


    // ----------------------------------------
    // *
    // ----------------------------------------

    if (c == '*')
    {
        position++;

        return {
            TokenType::STAR,
            "*"
        };
    }


    // ----------------------------------------
    // (
    // ----------------------------------------

    if (c == '(')
    {
        position++;

        return {
            TokenType::LEFT_PAREN,
            "("
        };
    }


    // ----------------------------------------
    // )
    // ----------------------------------------

    if (c == ')')
    {
        position++;

        return {
            TokenType::RIGHT_PAREN,
            ")"
        };
    }


    // ----------------------------------------
    // ,
    // ----------------------------------------

    if (c == ',')
    {
        position++;

        return {
            TokenType::COMMA,
            ","
        };
    }


    // ----------------------------------------
    // ;
    // ----------------------------------------

    if (c == ';')
    {
        position++;

        return {
            TokenType::SEMICOLON,
            ";"
        };
    }


    // ----------------------------------------
    // Invalid
    // ----------------------------------------

    position++;

    return {
        TokenType::INVALID,
        std::string(1, c)
    };
}


// ============================================================
// Get Next Token
// ============================================================

Token Lexer::nextToken()
{
    skipWhitespace();


    // End of input.
    if (position >= input.size())
    {
        return {
            TokenType::END,
            ""
        };
    }


    char c = currentChar();


    // Identifier / keyword.
    if (
        std::isalpha(
            static_cast<unsigned char>(c))
        ||
        c == '_')
    {
        return readIdentifierOrKeyword();
    }


    // Number.
    if (
        std::isdigit(
            static_cast<unsigned char>(c)))
    {
        return readNumber();
    }


    // String.
    if (c == '\'' || c == '"')
    {
        return readString();
    }


    // Operator / symbol.
    return readOperatorOrSymbol();
}


// ============================================================
// Tokenize Entire Input
// ============================================================

std::vector<Token> Lexer::tokenize()
{
    std::vector<Token> tokens;

    while (true)
    {
        Token token = nextToken();

        tokens.push_back(token);

        if (token.type == TokenType::END)
        {
            break;
        }
    }

    return tokens;
}


// ============================================================
// Token Type → String
// ============================================================

std::string tokenTypeToString(
    TokenType type)
{
    switch (type)
    {
        case TokenType::SELECT:
            return "SELECT";

        case TokenType::FROM:
            return "FROM";

        case TokenType::WHERE:
            return "WHERE";

        case TokenType::INSERT:
            return "INSERT";

        case TokenType::INTO:
            return "INTO";

        case TokenType::VALUES:
            return "VALUES";

        case TokenType::UPDATE:
            return "UPDATE";

        case TokenType::SET:
            return "SET";

        case TokenType::DELETE:
            return "DELETE";

        case TokenType::CREATE:
            return "CREATE";

        case TokenType::TABLE:
            return "TABLE";

        case TokenType::LIMIT:
            return "LIMIT";

        case TokenType::IDENTIFIER:
            return "IDENTIFIER";

        case TokenType::NUMBER:
            return "NUMBER";

        case TokenType::STRING:
            return "STRING";

        case TokenType::EQUAL:
            return "EQUAL";

        case TokenType::GREATER:
            return "GREATER";

        case TokenType::LESS:
            return "LESS";

        case TokenType::GREATER_EQUAL:
            return "GREATER_EQUAL";

        case TokenType::LESS_EQUAL:
            return "LESS_EQUAL";

        case TokenType::NOT_EQUAL:
            return "NOT_EQUAL";

        case TokenType::STAR:
            return "STAR";

        case TokenType::LEFT_PAREN:
            return "LEFT_PAREN";

        case TokenType::RIGHT_PAREN:
            return "RIGHT_PAREN";

        case TokenType::COMMA:
            return "COMMA";

        case TokenType::SEMICOLON:
            return "SEMICOLON";

        case TokenType::END:
            return "END";

        case TokenType::INVALID:
            return "INVALID";
    }

    return "UNKNOWN";
}