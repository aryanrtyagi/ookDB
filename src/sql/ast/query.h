
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
    DELETE
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
    std::vector<Literal> values;
};

struct SelectQuery
{
    std::string tableName;
    std::vector<std::string> columns;
    bool hasCondition = false;
    Condition condition;
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

struct Query
{
    QueryType type;

    CreateTableQuery createTable;
    CreateIndexQuery createIndex;
    InsertQuery insert;
    SelectQuery select;
    UpdateQuery update;
    DeleteQuery remove;
};

#endif
