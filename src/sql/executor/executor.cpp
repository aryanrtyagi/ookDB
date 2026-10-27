#include "executor.h"

#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <cctype>

static DataType parseDataType(
    const std::string& type)
{
    std::string upper = type;

    for (char& c : upper)
    {
        c = static_cast<char>(
            std::toupper(
                static_cast<unsigned char>(c)
            )
        );
    }

    if (upper == "INT")
        return DataType::INT;

    if (upper == "DOUBLE")
        return DataType::DOUBLE;

    if (upper == "BOOLEAN")
        return DataType::BOOLEAN;

    if (upper == "VARCHAR")
        return DataType::VARCHAR;

    throw std::runtime_error(
        "Unknown data type: " + type
    );
}

// ----------------------------------------
// Constructor
// ----------------------------------------

Executor::Executor(
    Pager& pager,
    BufferPool& bufferPool,
    PageAllocator& pageAllocator,
    Catalog& catalog
)
    : pager(pager),
      bufferPool(bufferPool),
      pageAllocator(pageAllocator),
      catalog(catalog)
{
}

// ----------------------------------------
// Open table
// ----------------------------------------

GenericTable Executor::openTable(
    const std::string& tableName
)
{
    if (!catalog.tableExists(tableName))
    {
        throw std::runtime_error(
            "Table does not exist: " +
            tableName
        );
    }

    Schema schema =
        catalog.getSchema(tableName);

    int firstPage =
        catalog.getFirstPage(tableName);

    return GenericTable(
        pager,
        bufferPool,
        pageAllocator,
        schema,
        firstPage
    );
}

// ----------------------------------------
// Print a Value according to its type
// ----------------------------------------

static void printValue(
    const Value& value
)
{
    switch (value.getType())
    {
        case DataType::INT:
            std::cout << value.asInt();
            break;

        case DataType::DOUBLE:
            std::cout << value.asDouble();
            break;

        case DataType::BOOLEAN:
            std::cout
                << (value.asBool()
                    ? "true"
                    : "false");
            break;

        case DataType::VARCHAR:
            std::cout << value.asString();
            break;
    }
}

// ----------------------------------------
// Evaluate WHERE condition
// ----------------------------------------

bool Executor::evaluateCondition(
    const Row& row,
    const Schema& schema,
    const Condition& condition
)
{
    int columnIndex =
        schema.getColumnIndex(
            condition.column
        );

    if (columnIndex == -1)
    {
        throw std::runtime_error(
            "Unknown column: " +
            condition.column
        );
    }

    Value rowValue =
        row.getValue(columnIndex);

    // ------------------------------------
    // NUMBER literal
    // ------------------------------------

    if (condition.value.type ==
        LiteralType::NUMBER)
    {
        int right =
            std::stoi(
                condition.value.value
            );

        int left =
            rowValue.asInt();

        switch (condition.op)
        {
            case ComparisonOperator::EQUAL:
                return left == right;

            case ComparisonOperator::NOT_EQUAL:
                return left != right;

            case ComparisonOperator::GREATER:
                return left > right;

            case ComparisonOperator::LESS:
                return left < right;

            case ComparisonOperator::GREATER_EQUAL:
                return left >= right;

            case ComparisonOperator::LESS_EQUAL:
                return left <= right;
        }
    }

    // ------------------------------------
    // STRING literal
    // ------------------------------------

    if (condition.value.type ==
        LiteralType::STRING)
    {
        std::string left =
            rowValue.asString();

        std::string right =
            condition.value.value;

        switch (condition.op)
        {
            case ComparisonOperator::EQUAL:
                return left == right;

            case ComparisonOperator::NOT_EQUAL:
                return left != right;

            case ComparisonOperator::GREATER:
                return left > right;

            case ComparisonOperator::LESS:
                return left < right;

            case ComparisonOperator::GREATER_EQUAL:
                return left >= right;

            case ComparisonOperator::LESS_EQUAL:
                return left <= right;
        }
    }

    return false;
}

// ----------------------------------------
// Execute SELECT
// ----------------------------------------

void Executor::executeSelect(
    const SelectQuery& query
)
{
    // ------------------------------------
    // Open table
    // ------------------------------------

    GenericTable table =
        openTable(query.tableName);

    const Schema& schema =
        table.getSchema();

    // ------------------------------------
    // Determine requested columns
    // ------------------------------------

    std::vector<int> columnIndexes;

    // SELECT *
    if (query.columns.size() == 1 &&
        query.columns[0] == "*")
    {
        for (int i = 0;
             i < schema.size();
             i++)
        {
            columnIndexes.push_back(i);
        }
    }
    // SELECT name, age, ...
    else
    {
        for (const std::string& column :
             query.columns)
        {
            int index =
                schema.getColumnIndex(column);

            if (index == -1)
            {
                throw std::runtime_error(
                    "Unknown column: " +
                    column
                );
            }

            columnIndexes.push_back(index);
        }
    }

    // ------------------------------------
    // Scan rows
    // ------------------------------------

    int rowCount =
        table.size();

    int rowsReturned = 0;

    for (int i = 0;
         i < rowCount;
         i++)
    {
        Row row =
            table.get(i);

        // --------------------------------
        // WHERE
        // --------------------------------

        if (query.hasCondition)
        {
            if (!evaluateCondition(
                    row,
                    schema,
                    query.condition))
            {
                continue;
            }
        }

        // --------------------------------
        // Projection
        // --------------------------------

        for (int j = 0;
             j < columnIndexes.size();
             j++)
        {
            Value value =
                row.getValue(
                    columnIndexes[j]
                );

            printValue(value);

            if (j + 1 < columnIndexes.size())
            {
                std::cout << " | ";
            }
        }

        std::cout << "\n";

        // --------------------------------
        // LIMIT
        // --------------------------------

        rowsReturned++;

        if (query.limit >= 0 &&
            rowsReturned >= query.limit)
        {
            break;
        }
    }
}

void Executor::execute(
    const Query& query
)
{
    if (query.type == QueryType::SELECT)
    {
        executeSelect(query.select);
        return;
    }

    if (query.type == QueryType::CREATE_TABLE)
    {
        const CreateTableQuery& createQuery =
            query.createTable;

        if (catalog.tableExists(
                createQuery.tableName))
        {
            throw std::runtime_error(
                "Table already exists: " +
                createQuery.tableName
            );
        }

        Schema schema;

        for (const ColumnDefinition& column :
             createQuery.columns)
        {
            DataType type =
                parseDataType(column.type);

            schema.addColumn(
                column.name,
                type
            );
        }

        bool success =
            catalog.createTable(
                createQuery.tableName,
                schema
            );

        if (!success)
        {
            throw std::runtime_error(
                "Failed to create table: " +
                createQuery.tableName
            );
        }

        std::cout
            << "Table '"
            << createQuery.tableName
            << "' created successfully.\n";

        return;
    }

    throw std::runtime_error(
        "Unsupported query type"
    );
}