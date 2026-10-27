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

Query Parser::parse()
{
    Query query;

    if (check(TokenType::SELECT))
    {
        query.type = QueryType::SELECT;
        query.select = parseSelect();
<<<<<<< HEAD
=======

        return query;
    }

    if (check(TokenType::CREATE))
    {
        query.type = QueryType::CREATE_TABLE;
        query.createTable = parseCreateTable();

        return query;
    }

    if (check(TokenType::INSERT))
    {
        query.type = QueryType::INSERT;
        query.insert = parseInsert();

>>>>>>> 4b0779494586eeddfd737f5f686076f8c696dc22
        return query;
    }

    if (check(TokenType::CREATE))
    {
        query.type = QueryType::CREATE_TABLE;
        query.createTable = parseCreateTable();
        return query;
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
    
    throw std::runtime_error(
        "Unsupported query"
    );
}

CreateTableQuery Parser::parseCreateTable()
{
    expect(TokenType::CREATE);

    expect(TokenType::TABLE);

    std::string tableName =
        parseIdentifier();

    expect(TokenType::LEFT_PAREN);

    std::vector<ColumnDefinition> columns;

    while (true)
    {
        std::string columnName =
            parseIdentifier();

        std::string columnType =
            parseIdentifier();

        columns.push_back({
            columnName,
            columnType
        });

        if (match(TokenType::COMMA))
        {
            continue;
        }

        break;
    }

    expect(TokenType::RIGHT_PAREN);

    match(TokenType::SEMICOLON);

    if (!check(TokenType::END))
    {
        throw std::runtime_error(
            "Unexpected token: " +
            currentToken().value
        );
    }

    CreateTableQuery query;

    query.tableName = tableName;
    query.columns = columns;

    return query;
}

InsertQuery Parser::parseInsert()
{
    expect(TokenType::INSERT);

    expect(TokenType::INTO);

    std::string tableName =
        parseIdentifier();

    expect(TokenType::VALUES);

    expect(TokenType::LEFT_PAREN);

    std::vector<Literal> values;

    values.push_back(
        parseLiteral()
    );

    while (match(TokenType::COMMA))
    {
        values.push_back(
            parseLiteral()
        );
    }

    expect(TokenType::RIGHT_PAREN);

    match(TokenType::SEMICOLON);

    if (!check(TokenType::END))
    {
        throw std::runtime_error(
            "Unexpected token: " +
            currentToken().value
        );
    }

    InsertQuery query;

    query.tableName = tableName;
    query.values = values;

    return query;
<<<<<<< HEAD
}


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
    {
        throw std::runtime_error(
            "Unexpected token: " + currentToken().value
        );
    }

    return query;
}

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
    {
        throw std::runtime_error(
            "Unexpected token: " + currentToken().value
        );
    }

    return query;
}
=======
}
>>>>>>> 4b0779494586eeddfd737f5f686076f8c696dc22
