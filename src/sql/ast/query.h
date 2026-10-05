#ifndef QUERY_H
#define QUERY_H

#include <string>
#include <vector>

#include "expression.h"

// ============================================================
// Query Type
// ============================================================

enum class QueryType
{
    CREATE_TABLE,
    INSERT,
    SELECT,
    UPDATE,
    DELETE
};


// ============================================================
// Column Definition
// ============================================================

struct ColumnDefinition
{
    std::string name;
    std::string type;
};


// ============================================================
// CREATE TABLE
// ============================================================

struct CreateTableQuery
{
    std::string tableName;
    std::vector<ColumnDefinition> columns;
};


// ============================================================
// INSERT
// ============================================================

struct InsertQuery
{
    std::string tableName;
    std::vector<Literal> values;
};


// ============================================================
// SELECT
// ============================================================

struct SelectQuery
{
    std::string tableName;

    // Empty means SELECT *
    std::vector<std::string> columns;

    bool hasCondition;

    Condition condition;

    int limit;
};


// ============================================================
// UPDATE
// ============================================================

struct UpdateQuery
{
    std::string tableName;

    std::string column;

    Literal value;

    bool hasCondition;

    Condition condition;
};


// ============================================================
// DELETE
// ============================================================

struct DeleteQuery
{
    std::string tableName;

    bool hasCondition;

    Condition condition;
};

#endif