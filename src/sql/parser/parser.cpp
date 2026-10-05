#include "parser.h"

#include <stdexcept>

Parser::Parser(
    const std::vector<Token>& tokens)
    : tokens(tokens),
      position(0)
{
}

const Token& Parser::currentToken() const
{
    if (position >= tokens.size())
    {
        throw std::runtime_error(
            "Unexpected end of input"
        );
    }

    return tokens[position];
}

bool Parser::check(TokenType type) const
{
    if (position >= tokens.size())
    {
        return false;
    }

    return currentToken().type == type;
}

Token Parser::consume()
{
    if (position >= tokens.size())
    {
        throw std::runtime_error(
            "Unexpected end of input"
        );
    }

    return tokens[position++];
}

bool Parser::match(TokenType type)
{
    if (!check(type))
    {
        return false;
    }

    consume();

    return true;
}

Token Parser::expect(TokenType type)
{
    if (!check(type))
    {
        throw std::runtime_error(
            "Unexpected token: " +
            currentToken().value
        );
    }

    return consume();
}

std::string Parser::parseIdentifier()
{
    Token token = expect(
        TokenType::IDENTIFIER
    );

    return token.value;
}

Literal Parser::parseLiteral()
{
    if (check(TokenType::NUMBER))
    {
        Token token = consume();

        Literal literal;

        literal.type =
            LiteralType::NUMBER;

        literal.value =
            token.value;

        return literal;
    }

    if (check(TokenType::STRING))
    {
        Token token = consume();

        Literal literal;

        literal.type =
            LiteralType::STRING;

        literal.value =
            token.value;

        return literal;
    }

    throw std::runtime_error(
        "Expected literal"
    );
}

Condition Parser::parseCondition()
{
    Condition condition;

    condition.column =
        parseIdentifier();

    if (match(TokenType::EQUAL))
    {
        condition.op =
            ComparisonOperator::EQUAL;
    }
    else if (match(TokenType::NOT_EQUAL))
    {
        condition.op =
            ComparisonOperator::NOT_EQUAL;
    }
    else if (match(TokenType::GREATER))
    {
        condition.op =
            ComparisonOperator::GREATER;
    }
    else if (match(TokenType::LESS))
    {
        condition.op =
            ComparisonOperator::LESS;
    }
    else if (match(TokenType::GREATER_EQUAL))
    {
        condition.op =
            ComparisonOperator::GREATER_EQUAL;
    }
    else if (match(TokenType::LESS_EQUAL))
    {
        condition.op =
            ComparisonOperator::LESS_EQUAL;
    }
    else
    {
        throw std::runtime_error(
            "Expected comparison operator"
        );
    }

    condition.value =
        parseLiteral();

    return condition;
}

std::vector<std::string>
Parser::parseColumnList()
{
    std::vector<std::string> columns;

    columns.push_back(
        parseIdentifier()
    );

    while (match(TokenType::COMMA))
    {
        columns.push_back(
            parseIdentifier()
        );
    }

    return columns;
}

SelectQuery Parser::parseSelect()
{
    SelectQuery query;

    expect(TokenType::SELECT);

    if (match(TokenType::STAR))
    {
        query.columns.push_back("*");
    }
    else
    {
        query.columns =
            parseColumnList();
    }

    expect(TokenType::FROM);

    query.tableName =
        parseIdentifier();

    query.hasCondition = false;

    if (match(TokenType::WHERE))
    {
        query.hasCondition = true;

        query.condition =
            parseCondition();
    }

    query.limit = -1;

    if (match(TokenType::LIMIT))
    {
        Literal limit =
            parseLiteral();

        if (limit.type !=
            LiteralType::NUMBER)
        {
            throw std::runtime_error(
                "LIMIT requires a number"
            );
        }

        query.limit =
            std::stoi(limit.value);
    }

    match(TokenType::SEMICOLON);

    if (!check(TokenType::END))
    {
        throw std::runtime_error(
            "Unexpected token: " +
            currentToken().value
        );
    }

    return query;
}

SelectQuery Parser::parse()
{
    if (check(TokenType::SELECT))
    {
        return parseSelect();
    }

    throw std::runtime_error(
        "Only SELECT is supported currently"
    );
}