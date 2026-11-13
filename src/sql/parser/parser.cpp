#include "parser.h"

#include <stdexcept>
#include <utility>

Parser::Parser(const std::vector<Token>& input)
    : tokens(input), position(0) {}

const Token& Parser::currentToken() const
{
    static const Token endToken{TokenType::END, ""};

    return position < tokens.size()
        ? tokens[position]
        : endToken;
}

bool Parser::check(TokenType type) const
{
    return currentToken().type == type;
}

Token Parser::consume()
{
    Token token = currentToken();

    if (position < tokens.size())
        ++position;

    return token;
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
        throw std::runtime_error(
            "Expected " + tokenTypeToString(type) +
            ", found " + tokenTypeToString(currentToken().type) +
            " (" + currentToken().value + ")"
        );

    return consume();
}

std::string Parser::parseIdentifier()
{
    return expect(TokenType::IDENTIFIER).value;
}

std::string Parser::parseQualifiedIdentifier()
{
    std::string name = parseIdentifier();

    while (match(TokenType::DOT))
        name += "." + parseIdentifier();

    return name;
}

Literal Parser::parseLiteral()
{
    if (check(TokenType::NUMBER))
        return {LiteralType::NUMBER, consume().value};

    if (check(TokenType::STRING))
        return {LiteralType::STRING, consume().value};

    if (match(TokenType::NULL_VALUE))
        return {LiteralType::NULL_VALUE, "NULL"};

    if (check(TokenType::IDENTIFIER))
    {
        std::string value = consume().value;

        if (value == "TRUE" || value == "FALSE" ||
            value == "true" || value == "false")
            return {LiteralType::STRING, value};
    }

    throw std::runtime_error("Expected a literal value");
}

std::vector<Literal> Parser::parseLiteralList()
{
    std::vector<Literal> values;

    expect(TokenType::LEFT_PAREN);
    values.push_back(parseLiteral());

    while (match(TokenType::COMMA))
        values.push_back(parseLiteral());

    expect(TokenType::RIGHT_PAREN);
    return values;
}

// ---------------- Conditions ----------------

Condition Parser::parseComparisonCondition()
{
    Condition condition;
    condition.type = ConditionType::COMPARISON;
    condition.column = parseQualifiedIdentifier();

    bool negated = false;

    if (match(TokenType::IS))
    {
        bool isNot = match(TokenType::NOT);
        expect(TokenType::NULL_VALUE);
        condition.op = isNot ? ComparisonOperator::IS_NOT_NULL
                             : ComparisonOperator::IS_NULL;
        return condition;
    }

    if (check(TokenType::NOT) &&
        position + 1 < tokens.size() &&
        (tokens[position + 1].type == TokenType::IN ||
         tokens[position + 1].type == TokenType::BETWEEN ||
         tokens[position + 1].type == TokenType::LIKE))
    {
        consume();
        negated = true;
    }

    if (match(TokenType::IN))
    {
        condition.op = negated
            ? ComparisonOperator::NOT_IN
            : ComparisonOperator::IN;

        condition.values = parseLiteralList();
        return condition;
    }

    if (match(TokenType::BETWEEN))
    {
        condition.op = negated
            ? ComparisonOperator::NOT_BETWEEN
            : ComparisonOperator::BETWEEN;

        condition.value = parseLiteral();
        expect(TokenType::AND);
        condition.secondValue = parseLiteral();
        return condition;
    }

    if (match(TokenType::LIKE))
    {
        condition.op = negated
            ? ComparisonOperator::NOT_LIKE
            : ComparisonOperator::LIKE;

        condition.value = parseLiteral();
        return condition;
    }

    if (negated)
        throw std::runtime_error("Expected IN, BETWEEN or LIKE after NOT");

    if (match(TokenType::EQUAL))
        condition.op = ComparisonOperator::EQUAL;
    else if (match(TokenType::NOT_EQUAL))
        condition.op = ComparisonOperator::NOT_EQUAL;
    else if (match(TokenType::GREATER_EQUAL))
        condition.op = ComparisonOperator::GREATER_EQUAL;
    else if (match(TokenType::LESS_EQUAL))
        condition.op = ComparisonOperator::LESS_EQUAL;
    else if (match(TokenType::GREATER))
        condition.op = ComparisonOperator::GREATER;
    else if (match(TokenType::LESS))
        condition.op = ComparisonOperator::LESS;
    else
        throw std::runtime_error("Expected comparison operator");

    condition.value = parseLiteral();
    return condition;
}

Condition Parser::parsePrimaryExpression()
{
    if (match(TokenType::LEFT_PAREN))
    {
        Condition condition = parseOrExpression();
        expect(TokenType::RIGHT_PAREN);
        return condition;
    }

    return parseComparisonCondition();
}

Condition Parser::parseNotExpression()
{
    if (match(TokenType::NOT))
    {
        Condition condition;
        condition.type = ConditionType::NOT;
        condition.left =
            std::make_shared<Condition>(parseNotExpression());
        return condition;
    }

    return parsePrimaryExpression();
}

Condition Parser::parseAndExpression()
{
    Condition left = parseNotExpression();

    while (match(TokenType::AND))
    {
        Condition combined;
        combined.type = ConditionType::AND;
        combined.left = std::make_shared<Condition>(std::move(left));
        combined.right =
            std::make_shared<Condition>(parseNotExpression());

        left = std::move(combined);
    }

    return left;
}

Condition Parser::parseOrExpression()
{
    Condition left = parseAndExpression();

    while (match(TokenType::OR))
    {
        Condition combined;
        combined.type = ConditionType::OR;
        combined.left = std::make_shared<Condition>(std::move(left));
        combined.right =
            std::make_shared<Condition>(parseAndExpression());

        left = std::move(combined);
    }

    return left;
}

Condition Parser::parseCondition()
{
    return parseOrExpression();
}

// ---------------- Column lists ----------------

std::vector<std::string> Parser::parseColumnList()
{
    std::vector<std::string> columns;

    columns.push_back(parseQualifiedIdentifier());

    while (match(TokenType::COMMA))
        columns.push_back(parseQualifiedIdentifier());

    return columns;
}

// ---------------- SELECT ----------------

SelectQuery Parser::parseSelect()
{
    SelectQuery query;

    query.distinct = match(TokenType::DISTINCT);

    do
    {
        SelectColumn column;

        if (match(TokenType::COUNT))
            column.aggregate = AggregateType::COUNT;
        else if (match(TokenType::SUM))
            column.aggregate = AggregateType::SUM;
        else if (match(TokenType::AVG))
            column.aggregate = AggregateType::AVG;
        else if (match(TokenType::MIN))
            column.aggregate = AggregateType::MIN;
        else if (match(TokenType::MAX))
            column.aggregate = AggregateType::MAX;

        if (column.aggregate != AggregateType::NONE)
        {
            expect(TokenType::LEFT_PAREN);

            if (match(TokenType::STAR))
            {
                column.isStar = true;
                column.name = "*";
            }
            else
                column.name = parseQualifiedIdentifier();

            expect(TokenType::RIGHT_PAREN);
        }
        else if (match(TokenType::STAR))
        {
            column.name = "*";
            column.isStar = true;
        }
        else
            column.name = parseQualifiedIdentifier();

        if (match(TokenType::AS))
            column.alias = parseIdentifier();

        query.selectColumns.push_back(column);
        query.columns.push_back(column.name);

    } while (match(TokenType::COMMA));

    expect(TokenType::FROM);
    query.tableName = parseIdentifier();

    while (check(TokenType::INNER) || check(TokenType::LEFT))
    {
        JoinClause join;

        join.leftJoin = match(TokenType::LEFT);

        if (!join.leftJoin)
            expect(TokenType::INNER);

        expect(TokenType::JOIN);
        join.tableName = parseIdentifier();

        // ON table.column = table.column
        expect(TokenType::ON);
        join.leftColumn = parseQualifiedIdentifier();
        expect(TokenType::EQUAL);
        join.rightColumn = parseQualifiedIdentifier();

        query.joins.push_back(join);
    }

    if (match(TokenType::WHERE))
    {
        query.hasCondition = true;
        query.condition = parseCondition();
    }

    if (match(TokenType::GROUP))
    {
        expect(TokenType::BY);
        query.groupBy = parseColumnList();
    }

    if (match(TokenType::HAVING))
    {
        query.hasHaving = true;
        query.having = parseCondition();
    }

    if (match(TokenType::ORDER))
    {
        expect(TokenType::BY);

        do
        {
            OrderByClause order;
            order.column = parseQualifiedIdentifier();

            if (match(TokenType::DESC))
                order.ascending = false;
            else
                match(TokenType::ASC);

            query.orderBy.push_back(order);

        } while (match(TokenType::COMMA));
    }

    if (match(TokenType::LIMIT))
    {
        Literal limit = parseLiteral();

        if (limit.type != LiteralType::NUMBER)
            throw std::runtime_error("LIMIT requires a number");

        query.limit = std::stoi(limit.value);
    }

    return query;
}

// ---------------- CREATE TABLE ----------------

CreateTableQuery Parser::parseCreateTable()
{
    CreateTableQuery query;

    expect(TokenType::TABLE);
    query.tableName = parseIdentifier();
    expect(TokenType::LEFT_PAREN);

    do
    {
        ColumnDefinition column;
        column.name = parseIdentifier();
        column.type = parseIdentifier();

        query.columns.push_back(column);

    } while (match(TokenType::COMMA));

    expect(TokenType::RIGHT_PAREN);
    return query;
}

// ---------------- CREATE INDEX ----------------

CreateIndexQuery Parser::parseCreateIndex()
{
    CreateIndexQuery query;

    expect(TokenType::INDEX);
    query.indexName = parseIdentifier();
    expect(TokenType::ON);

    query.tableName = parseIdentifier();

    expect(TokenType::LEFT_PAREN);
    query.columnName = parseIdentifier();
    expect(TokenType::RIGHT_PAREN);

    return query;
}

// ---------------- INSERT ----------------

InsertQuery Parser::parseInsert()
{
    InsertQuery query;

    expect(TokenType::INTO);
    query.tableName = parseIdentifier();

    expect(TokenType::VALUES);

    do
    {
        std::vector<Literal> row;
        expect(TokenType::LEFT_PAREN);

        row.push_back(parseLiteral());

        while (match(TokenType::COMMA))
            row.push_back(parseLiteral());

        expect(TokenType::RIGHT_PAREN);
        query.rows.push_back(std::move(row));

    } while (match(TokenType::COMMA));

    return query;
}

// ---------------- UPDATE ----------------

UpdateQuery Parser::parseUpdate()
{
    UpdateQuery query;

    query.tableName = parseIdentifier();

    expect(TokenType::SET);
    query.column = parseIdentifier();

    expect(TokenType::EQUAL);
    query.value = parseLiteral();

    if (match(TokenType::WHERE))
    {
        query.hasCondition = true;
        query.condition = parseCondition();
    }

    return query;
}

// ---------------- DELETE ----------------

DeleteQuery Parser::parseDelete()
{
    DeleteQuery query;

    expect(TokenType::FROM);
    query.tableName = parseIdentifier();

    if (match(TokenType::WHERE))
    {
        query.hasCondition = true;
        query.condition = parseCondition();
    }

    return query;
}

// ---------------- DDL ----------------

DescribeQuery Parser::parseDescribe()
{
    DescribeQuery query;
    query.tableName = parseIdentifier();
    return query;
}

DropTableQuery Parser::parseDropTable()
{
    DropTableQuery query;
    expect(TokenType::TABLE);
    query.tableName = parseIdentifier();
    return query;
}

AlterTableQuery Parser::parseAlterTable()
{
    AlterTableQuery query;

    expect(TokenType::TABLE);
    query.tableName = parseIdentifier();

    if (match(TokenType::ADD))
    {
        match(TokenType::COLUMN);
        query.operation = AlterTableQuery::Operation::ADD_COLUMN;
        query.columnName = parseIdentifier();
        query.columnType = parseIdentifier();
    }
    else if (match(TokenType::DROP))
    {
        match(TokenType::COLUMN);
        query.operation = AlterTableQuery::Operation::DROP_COLUMN;
        query.columnName = parseIdentifier();
    }
    else if (match(TokenType::RENAME))
    {
        match(TokenType::COLUMN);
        query.operation = AlterTableQuery::Operation::RENAME_COLUMN;
        query.columnName = parseIdentifier();
        expect(TokenType::TO);
        query.newColumnName = parseIdentifier();
    }
    else
        throw std::runtime_error("Expected ADD, DROP or RENAME in ALTER TABLE");

    return query;
}

// ---------------- Main parser ----------------

Query Parser::parse()
{
    Query query{};

    if (match(TokenType::SELECT))
    {
        query.type = QueryType::SELECT;
        query.select = parseSelect();
    }
    else if (match(TokenType::INSERT))
    {
        query.type = QueryType::INSERT;
        query.insert = parseInsert();
    }
    else if (match(TokenType::UPDATE))
    {
        query.type = QueryType::UPDATE;
        query.update = parseUpdate();
    }
    else if (match(TokenType::DELETE))
    {
        query.type = QueryType::DELETE;
        query.remove = parseDelete();
    }
    else if (match(TokenType::CREATE))
    {
        if (check(TokenType::TABLE))
        {
            query.type = QueryType::CREATE_TABLE;
            query.createTable = parseCreateTable();
        }
        else if (check(TokenType::INDEX))
        {
            query.type = QueryType::CREATE_INDEX;
            query.createIndex = parseCreateIndex();
        }
        else
            throw std::runtime_error("Expected TABLE or INDEX after CREATE");
    }
    else if (match(TokenType::SHOW))
    {
        expect(TokenType::TABLES);
        query.type = QueryType::SHOW_TABLES;
    }
    else if (match(TokenType::DESCRIBE))
    {
        query.type = QueryType::DESCRIBE;
        query.describe = parseDescribe();
    }
    else if (match(TokenType::DROP))
    {
        query.type = QueryType::DROP_TABLE;
        query.dropTable = parseDropTable();
    }
    else if (match(TokenType::ALTER))
    {
        query.type = QueryType::ALTER_TABLE;
        query.alterTable = parseAlterTable();
    }
    else
        throw std::runtime_error(
            "Unsupported SQL statement: " + currentToken().value
        );

    match(TokenType::SEMICOLON);

    if (!check(TokenType::END))
        throw std::runtime_error(
            "Unexpected token after statement: " + currentToken().value
        );

    return query;
}
