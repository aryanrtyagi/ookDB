
#ifndef QUERY_H
#define QUERY_H

#include <string>
#include <vector>

#include "expression.h"

enum class QueryType
{
    CREATE_TABLE,
    CREATE_INDEX,
    INSERT,
    SELECT,
    UPDATE,
    DELETE,
    SHOW_TABLES,
    DESCRIBE,
    DROP_TABLE,
    ALTER_TABLE
};

struct ColumnDefinition
{
    std::string name;
    std::string type;
};

struct CreateTableQuery
{
    std::string tableName;
    std::vector<ColumnDefinition> columns;
};

struct CreateIndexQuery
{
    std::string indexName;
    std::string tableName;
    std::string columnName;
};

struct InsertQuery
{
    std::string tableName;
    std::vector<std::vector<Literal>> rows;
};

enum class AggregateType
{
    NONE,
    COUNT,
    SUM,
    AVG,
    MIN,
    MAX
};

struct SelectColumn
{
    std::string name;
    AggregateType aggregate = AggregateType::NONE;
    std::string alias;
    bool isStar = false;
};

struct OrderByClause
{
    std::string column;
    bool ascending = true;
};

struct JoinClause
{
    std::string tableName;
    std::string leftColumn;
    std::string rightColumn;
    bool leftJoin = false;
};

struct SelectQuery
{
    std::string tableName;

    std::vector<std::string> columns;
    std::vector<SelectColumn> selectColumns;

    bool distinct = false;

    bool hasCondition = false;
    Condition condition;

    std::vector<std::string> groupBy;
    bool hasHaving = false;
    Condition having;

    std::vector<OrderByClause> orderBy;
    std::vector<JoinClause> joins;

    int limit = -1;
};

struct UpdateQuery
{
    std::string tableName;
    std::string column;
    Literal value;

    bool hasCondition = false;
    Condition condition;
};

struct DeleteQuery
{
    std::string tableName;

    bool hasCondition = false;
    Condition condition;
};

struct DescribeQuery
{
    std::string tableName;
};

struct DropTableQuery
{
    std::string tableName;
};

struct AlterTableQuery
{
    std::string tableName;
    std::string columnName;
    std::string newColumnName;
    std::string columnType;

    enum class Operation
    {
        ADD_COLUMN,
        DROP_COLUMN,
        RENAME_COLUMN
    };

    Operation operation = Operation::ADD_COLUMN;
};

struct Query
{
    QueryType type;

    CreateTableQuery createTable;
    CreateIndexQuery createIndex;
    InsertQuery insert;
    SelectQuery select;
    UpdateQuery update;
    DeleteQuery remove;

    DescribeQuery describe;
    DropTableQuery dropTable;
    AlterTableQuery alterTable;
};

#endif
