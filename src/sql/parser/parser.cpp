#include "parser.h"

#include <stdexcept>
#include <string>

Parser::Parser(const std::vector<Token>& tokens)
    : tokens(tokens), position(0)
{
}

const Token& Parser::currentToken() const
{
    if (position >= tokens.size())
        throw std::runtime_error("Unexpected end of input");

    return tokens[position];
}

bool Parser::check(TokenType type) const
{
    return position < tokens.size() &&
           currentToken().type == type;
}

Token Parser::consume()
{
    if (position >= tokens.size())
        throw std::runtime_error("Unexpected end of input");

    return tokens[position++];
}

bool Parser::match(TokenType type)
{
    if (!check(type))
        return false;

    consume();
    return true;
}

Token Parser::expect(TokenType type)
{
    if (!check(type))
    {
        std::string found =
            position < tokens.size()
                ? currentToken().value
                : "END";

        throw std::runtime_error(
            "Unexpected token: " + found
        );
    }

    return consume();
}

std::string Parser::parseIdentifier()
{
    return expect(TokenType::IDENTIFIER).value;
}

Literal Parser::parseLiteral()
{
    if (check(TokenType::NUMBER))
    {
        Literal literal;
        literal.type = LiteralType::NUMBER;
        literal.value = consume().value;
        return literal;
    }

    if (check(TokenType::STRING))
    {
        Literal literal;
        literal.type = LiteralType::STRING;
        literal.value = consume().value;
        return literal;
    }

    throw std::runtime_error("Expected literal");
}

Condition Parser::parseCondition()
{
    Condition condition;

    condition.column = parseIdentifier();

    if (match(TokenType::EQUAL))
        condition.op = ComparisonOperator::EQUAL;
    else if (match(TokenType::NOT_EQUAL))
        condition.op = ComparisonOperator::NOT_EQUAL;
    else if (match(TokenType::GREATER))
        condition.op = ComparisonOperator::GREATER;
    else if (match(TokenType::LESS))
        condition.op = ComparisonOperator::LESS;
    else if (match(TokenType::GREATER_EQUAL))
        condition.op = ComparisonOperator::GREATER_EQUAL;
    else if (match(TokenType::LESS_EQUAL))
        condition.op = ComparisonOperator::LESS_EQUAL;
    else
        throw std::runtime_error("Expected comparison operator");

    condition.value = parseLiteral();

    return condition;
}

std::vector<std::string> Parser::parseColumnList()
{
    std::vector<std::string> columns;

    columns.push_back(parseIdentifier());

    while (match(TokenType::COMMA))
        columns.push_back(parseIdentifier());

    return columns;
}

// --------------------------------------------------
// SELECT
// --------------------------------------------------

SelectQuery Parser::parseSelect()
{
    SelectQuery query;

    expect(TokenType::SELECT);

    if (match(TokenType::STAR))
        query.columns.push_back("*");
    else
        query.columns = parseColumnList();

    expect(TokenType::FROM);

    query.tableName = parseIdentifier();

    query.hasCondition = false;
    query.limit = -1;

    if (match(TokenType::WHERE))
    {
        query.hasCondition = true;
        query.condition = parseCondition();
    }

    if (match(TokenType::LIMIT))
    {
        Literal limit = parseLiteral();

        if (limit.type != LiteralType::NUMBER)
            throw std::runtime_error("LIMIT requires a number");

        try
        {
            std::size_t position = 0;
            int value = std::stoi(limit.value, &position);

            if (position != limit.value.size() || value < 0)
                throw std::runtime_error("Invalid LIMIT value");

            query.limit = value;
        }
        catch (const std::exception&)
        {
            throw std::runtime_error("Invalid LIMIT value");
        }
    }

    match(TokenType::SEMICOLON);

    if (!check(TokenType::END))
        throw std::runtime_error(
            "Unexpected token: " + currentToken().value
        );

    return query;
}

// --------------------------------------------------
// CREATE TABLE
// --------------------------------------------------

CreateTableQuery Parser::parseCreateTable()
{
    expect(TokenType::CREATE);
    expect(TokenType::TABLE);

    CreateTableQuery query;

    query.tableName = parseIdentifier();

    expect(TokenType::LEFT_PAREN);

    while (true)
    {
        std::string columnName = parseIdentifier();
        std::string columnType = parseIdentifier();

        query.columns.push_back({
            columnName,
            columnType
        });

        if (!match(TokenType::COMMA))
            break;
    }

    expect(TokenType::RIGHT_PAREN);

    match(TokenType::SEMICOLON);

    if (!check(TokenType::END))
        throw std::runtime_error(
            "Unexpected token: " + currentToken().value
        );

    return query;
}

// --------------------------------------------------
// CREATE INDEX
// --------------------------------------------------

CreateIndexQuery Parser::parseCreateIndex()
{
    expect(TokenType::CREATE);
    expect(TokenType::INDEX);

    CreateIndexQuery query;

    query.indexName = parseIdentifier();

    expect(TokenType::ON);

    query.tableName = parseIdentifier();

    expect(TokenType::LEFT_PAREN);

    query.columnName = parseIdentifier();

    expect(TokenType::RIGHT_PAREN);

    match(TokenType::SEMICOLON);

    if (!check(TokenType::END))
        throw std::runtime_error(
            "Unexpected token: " + currentToken().value
        );

    return query;
}

// --------------------------------------------------
// INSERT
// --------------------------------------------------

InsertQuery Parser::parseInsert()
{
    expect(TokenType::INSERT);
    expect(TokenType::INTO);

    InsertQuery query;

    query.tableName = parseIdentifier();

    expect(TokenType::VALUES);
    expect(TokenType::LEFT_PAREN);

    query.values.push_back(parseLiteral());

    while (match(TokenType::COMMA))
        query.values.push_back(parseLiteral());

    expect(TokenType::RIGHT_PAREN);

    match(TokenType::SEMICOLON);

    if (!check(TokenType::END))
        throw std::runtime_error(
            "Unexpected token: " + currentToken().value
        );

    return query;
}

// --------------------------------------------------
// UPDATE
// --------------------------------------------------

UpdateQuery Parser::parseUpdate()
{
    expect(TokenType::UPDATE);

    UpdateQuery query;

    query.tableName = parseIdentifier();

    expect(TokenType::SET);

    query.column = parseIdentifier();

    expect(TokenType::EQUAL);

    query.value = parseLiteral();
    query.hasCondition = false;

    if (match(TokenType::WHERE))
    {
        query.hasCondition = true;
        query.condition = parseCondition();
    }

    match(TokenType::SEMICOLON);

    if (!check(TokenType::END))
        throw std::runtime_error(
            "Unexpected token: " + currentToken().value
        );

    return query;
}

// --------------------------------------------------
// DELETE
// --------------------------------------------------

DeleteQuery Parser::parseDelete()
{
    expect(TokenType::DELETE);
    expect(TokenType::FROM);

    DeleteQuery query;

    query.tableName = parseIdentifier();
    query.hasCondition = false;

    if (match(TokenType::WHERE))
    {
        query.hasCondition = true;
        query.condition = parseCondition();
    }

    match(TokenType::SEMICOLON);

    if (!check(TokenType::END))
        throw std::runtime_error(
            "Unexpected token: " + currentToken().value
        );

    return query;
}

// --------------------------------------------------
// Main parser entry point
// --------------------------------------------------

Query Parser::parse()
{
    Query query;

    if (check(TokenType::SELECT))
    {
        query.type = QueryType::SELECT;
        query.select = parseSelect();
        return query;
    }

    if (check(TokenType::CREATE))
    {
        if (position + 1 >= tokens.size())
            throw std::runtime_error("Incomplete CREATE statement");

        if (tokens[position + 1].type == TokenType::TABLE)
        {
            query.type = QueryType::CREATE_TABLE;
            query.createTable = parseCreateTable();
            return query;
        }

        if (tokens[position + 1].type == TokenType::INDEX)
        {
            query.type = QueryType::CREATE_INDEX;
            query.createIndex = parseCreateIndex();
            return query;
        }

        throw std::runtime_error(
            "Expected TABLE or INDEX after CREATE"
        );
    }

    if (check(TokenType::INSERT))
    {
        query.type = QueryType::INSERT;
        query.insert = parseInsert();
        return query;
    }

    if (check(TokenType::UPDATE))
    {
        query.type = QueryType::UPDATE;
        query.update = parseUpdate();
        return query;
    }

    if (check(TokenType::DELETE))
    {
        query.type = QueryType::DELETE;
        query.remove = parseDelete();
        return query;
    }

    throw std::runtime_error("Unsupported query");
}
