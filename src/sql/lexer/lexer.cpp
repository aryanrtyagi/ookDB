
#include "lexer.h"

#include <cctype>
#include <unordered_map>

Lexer::Lexer(const std::string& input)
    : input(input), position(0) {}

char Lexer::currentChar() const
{
    return position < input.size() ? input[position] : '\0';
}

void Lexer::skipWhitespace()
{
    while (position < input.size() &&
           std::isspace(static_cast<unsigned char>(input[position])))
        ++position;
}

Token Lexer::readIdentifierOrKeyword()
{
    std::string value;

    while (position < input.size())
    {
        char c = input[position];

        if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_')
            break;

        value += c;
        ++position;
    }

    std::string upper = value;

    for (char& c : upper)
        c = static_cast<char>(
            std::toupper(static_cast<unsigned char>(c)));

    static const std::unordered_map<std::string, TokenType> keywords = {
        {"SELECT", TokenType::SELECT},
        {"FROM", TokenType::FROM},
        {"WHERE", TokenType::WHERE},
        {"INSERT", TokenType::INSERT},
        {"INTO", TokenType::INTO},
        {"VALUES", TokenType::VALUES},
        {"UPDATE", TokenType::UPDATE},
        {"SET", TokenType::SET},
        {"DELETE", TokenType::DELETE},
        {"CREATE", TokenType::CREATE},
        {"TABLE", TokenType::TABLE},
        {"INDEX", TokenType::INDEX},
        {"ON", TokenType::ON},
        {"LIMIT", TokenType::LIMIT},

        {"DISTINCT", TokenType::DISTINCT},
        {"ORDER", TokenType::ORDER},
        {"BY", TokenType::BY},
        {"ASC", TokenType::ASC},
        {"DESC", TokenType::DESC},
        {"GROUP", TokenType::GROUP},
        {"HAVING", TokenType::HAVING},
        {"INNER", TokenType::INNER},
        {"LEFT", TokenType::LEFT},
        {"JOIN", TokenType::JOIN},
        {"AS", TokenType::AS},

        {"COUNT", TokenType::COUNT},
        {"SUM", TokenType::SUM},
        {"AVG", TokenType::AVG},
        {"MIN", TokenType::MIN},
        {"MAX", TokenType::MAX},

        {"SHOW", TokenType::SHOW},
        {"TABLES", TokenType::TABLES},
        {"DESCRIBE", TokenType::DESCRIBE},
        {"DROP", TokenType::DROP},
        {"ALTER", TokenType::ALTER},
        {"ADD", TokenType::ADD},
        {"COLUMN", TokenType::COLUMN},
        {"RENAME", TokenType::RENAME},
        {"TO", TokenType::TO},

        {"AND", TokenType::AND},
        {"OR", TokenType::OR},
        {"NOT", TokenType::NOT},
        {"IN", TokenType::IN},
        {"BETWEEN", TokenType::BETWEEN},
        {"LIKE", TokenType::LIKE}
    };

    auto it = keywords.find(upper);

    if (it != keywords.end())
        return {it->second, value};

    return {TokenType::IDENTIFIER, value};
}

Token Lexer::readNumber()
{
    std::string value;
    bool decimal = false;

    while (position < input.size())
    {
        char c = input[position];

        if (std::isdigit(static_cast<unsigned char>(c)))
        {
            value += c;
            ++position;
        }
        else if (c == '.' && !decimal &&
                 position + 1 < input.size() &&
                 std::isdigit(static_cast<unsigned char>(input[position + 1])))
        {
            decimal = true;
            value += c;
            ++position;
        }
        else
            break;
    }

    return {TokenType::NUMBER, value};
}

Token Lexer::readString()
{
    char quote = input[position++];
    std::string value;

    while (position < input.size() && input[position] != quote)
    {
        if (input[position] == '\\' && position + 1 < input.size())
        {
            ++position;
            value += input[position++];
        }
        else
            value += input[position++];
    }

    if (position >= input.size())
        return {TokenType::INVALID, "Unterminated string"};

    ++position;
    return {TokenType::STRING, value};
}

Token Lexer::readOperatorOrSymbol()
{
    char c = input[position++];

    switch (c)
    {
        case '=': return {TokenType::EQUAL, "="};
        case '*': return {TokenType::STAR, "*"};
        case '(': return {TokenType::LEFT_PAREN, "("};
        case ')': return {TokenType::RIGHT_PAREN, ")"};
        case ',': return {TokenType::COMMA, ","};
        case ';': return {TokenType::SEMICOLON, ";"};
        case '.': return {TokenType::DOT, "."};

        case '>':
            if (currentChar() == '=')
            {
                ++position;
                return {TokenType::GREATER_EQUAL, ">="};
            }
            return {TokenType::GREATER, ">"};

        case '<':
            if (currentChar() == '=')
            {
                ++position;
                return {TokenType::LESS_EQUAL, "<="};
            }
            if (currentChar() == '>')
            {
                ++position;
                return {TokenType::NOT_EQUAL, "<>"};
            }
            return {TokenType::LESS, "<"};

        case '!':
            if (currentChar() == '=')
            {
                ++position;
                return {TokenType::NOT_EQUAL, "!="};
            }
            return {TokenType::INVALID, "!"};

        default:
            return {TokenType::INVALID, std::string(1, c)};
    }
}

Token Lexer::nextToken()
{
    skipWhitespace();

    if (position >= input.size())
        return {TokenType::END, ""};

    char c = currentChar();

    if (std::isalpha(static_cast<unsigned char>(c)) || c == '_')
        return readIdentifierOrKeyword();

    if (std::isdigit(static_cast<unsigned char>(c)))
        return readNumber();

    if (c == '\'' || c == '"')
        return readString();

    return readOperatorOrSymbol();
}

std::vector<Token> Lexer::tokenize()
{
    std::vector<Token> tokens;

    while (true)
    {
        Token token = nextToken();
        tokens.push_back(token);

        if (token.type == TokenType::END ||
            token.type == TokenType::INVALID)
            break;
    }

    return tokens;
}

std::string tokenTypeToString(TokenType type)
{
    switch (type)
    {
        case TokenType::SELECT: return "SELECT";
        case TokenType::FROM: return "FROM";
        case TokenType::WHERE: return "WHERE";
        case TokenType::INSERT: return "INSERT";
        case TokenType::INTO: return "INTO";
        case TokenType::VALUES: return "VALUES";
        case TokenType::UPDATE: return "UPDATE";
        case TokenType::SET: return "SET";
        case TokenType::DELETE: return "DELETE";
        case TokenType::CREATE: return "CREATE";
        case TokenType::TABLE: return "TABLE";
        case TokenType::INDEX: return "INDEX";
        case TokenType::ON: return "ON";
        case TokenType::LIMIT: return "LIMIT";
        case TokenType::AND: return "AND";
        case TokenType::OR: return "OR";
        case TokenType::NOT: return "NOT";
        case TokenType::IN: return "IN";
        case TokenType::BETWEEN: return "BETWEEN";
        case TokenType::LIKE: return "LIKE";
        case TokenType::END: return "END";
        case TokenType::INVALID: return "INVALID";
        default: return "TOKEN";
    }
}
