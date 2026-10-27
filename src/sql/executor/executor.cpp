#include "executor.h"

#include <iostream>
#include <stdexcept>
#include <string>
#include <algorithm>
#include <cctype>

// Convert SQL datatype text into the core DataType enum.
static DataType parseDataType(const std::string &typeName)
{
    std::string type = typeName;

    std::transform(
        type.begin(),
        type.end(),
        type.begin(),
        [](unsigned char c)
        {
            return static_cast<char>(std::toupper(c));
        });

    if (type == "INT" || type == "INTEGER")
        return DataType::INT;

    if (type == "DOUBLE" || type == "FLOAT")
        return DataType::DOUBLE;

    if (type == "BOOLEAN" || type == "BOOL")
        return DataType::BOOLEAN;

    if (type == "VARCHAR" || type == "STRING" || type == "TEXT")
        return DataType::VARCHAR;

    throw std::runtime_error("Unsupported datatype: " + typeName);
}

// Constructor
Executor::Executor(
    Pager &pager,
    BufferPool &bufferPool,
    PageAllocator &pageAllocator,
    Catalog &catalog,
    CatalogStorage &catalogStorage)
    : pager(pager),
      bufferPool(bufferPool),
      pageAllocator(pageAllocator),
      catalog(catalog),
      catalogStorage(catalogStorage)
{
}

// Open an existing table using Phase 1 storage APIs.
GenericTable Executor::openTable(const std::string &tableName)
{
    if (!catalog.tableExists(tableName))
        throw std::runtime_error("Table does not exist: " + tableName);

    Schema schema = catalog.getSchema(tableName);
    int firstPage = catalog.getFirstPage(tableName);

    return GenericTable(
        pager,
        bufferPool,
        pageAllocator,
        schema,
        firstPage);
}

// Evaluate a WHERE condition for a row.

bool Executor::evaluateCondition(
    const Row &row,
    const Schema &schema,
    const Condition &condition)
{
    int columnIndex = schema.getColumnIndex(condition.column);

    if (columnIndex < 0)
    {
        throw std::runtime_error(
            "Column does not exist: " + condition.column);
    }

    const Value &actual = row.getValue(columnIndex);

    Value expected = convertLiteral(
        condition.value,
        schema.getColumn(columnIndex).getType());

    if (actual.getType() != expected.getType())
    {
        throw std::runtime_error("WHERE value type mismatch");
    }

    ComparisonOperator op = condition.op;

    switch (actual.getType())
    {
    case DataType::INT:
    {
        int a = actual.asInt();
        int b = expected.asInt();

        switch (op)
        {
        case ComparisonOperator::EQUAL:
            return a == b;
        case ComparisonOperator::NOT_EQUAL:
            return a != b;
        case ComparisonOperator::GREATER:
            return a > b;
        case ComparisonOperator::LESS:
            return a < b;
        case ComparisonOperator::GREATER_EQUAL:
            return a >= b;
        case ComparisonOperator::LESS_EQUAL:
            return a <= b;
        }
        break;
    }

    case DataType::DOUBLE:
    {
        double a = actual.asDouble();
        double b = expected.asDouble();

        switch (op)
        {
        case ComparisonOperator::EQUAL:
            return a == b;
        case ComparisonOperator::NOT_EQUAL:
            return a != b;
        case ComparisonOperator::GREATER:
            return a > b;
        case ComparisonOperator::LESS:
            return a < b;
        case ComparisonOperator::GREATER_EQUAL:
            return a >= b;
        case ComparisonOperator::LESS_EQUAL:
            return a <= b;
        }
        break;
    }

    case DataType::VARCHAR:
    {
        const std::string &a = actual.asString();
        const std::string &b = expected.asString();

        switch (op)
        {
        case ComparisonOperator::EQUAL:
            return a == b;
        case ComparisonOperator::NOT_EQUAL:
            return a != b;
        case ComparisonOperator::GREATER:
            return a > b;
        case ComparisonOperator::LESS:
            return a < b;
        case ComparisonOperator::GREATER_EQUAL:
            return a >= b;
        case ComparisonOperator::LESS_EQUAL:
            return a <= b;
        }
        break;
    }

    case DataType::BOOLEAN:
    {
        bool a = actual.asBool();
        bool b = expected.asBool();

        switch (op)
        {
        case ComparisonOperator::EQUAL:
            return a == b;
        case ComparisonOperator::NOT_EQUAL:
            return a != b;
        default:
            throw std::runtime_error(
                "Ordering comparisons are not supported for BOOLEAN");
        }
    }
    }

    throw std::runtime_error("Unsupported comparison operator");
}

// Convert a SQL literal into the requested core Value type.
Value Executor::convertLiteral(
    const Literal &literal,
    DataType expectedType)
{
    try
    {
        switch (expectedType)
        {
        case DataType::INT:
        {
            if (literal.type != LiteralType::NUMBER)
                throw std::runtime_error("Expected an integer");

            std::size_t pos = 0;
            int value = std::stoi(literal.value, &pos);

            if (pos != literal.value.size())
                throw std::runtime_error("Invalid integer: " + literal.value);

            return Value(value);
        }

        case DataType::DOUBLE:
        {
            if (literal.type != LiteralType::NUMBER)
                throw std::runtime_error("Expected a number");

            std::size_t pos = 0;
            double value = std::stod(literal.value, &pos);

            if (pos != literal.value.size())
                throw std::runtime_error("Invalid number: " + literal.value);

            return Value(value);
        }

        case DataType::VARCHAR:
        {
            if (literal.type != LiteralType::STRING)
                throw std::runtime_error("Expected a quoted string");

            return Value(literal.value);
        }

        case DataType::BOOLEAN:
        {
            std::string value = literal.value;

            std::transform(
                value.begin(),
                value.end(),
                value.begin(),
                [](unsigned char c)
                {
                    return static_cast<char>(std::tolower(c));
                });

            if (value == "true")
                return Value(true);

            if (value == "false")
                return Value(false);

            throw std::runtime_error(
                "Expected TRUE or FALSE for BOOLEAN");
        }
        }
    }
    catch (const std::invalid_argument &)
    {
        throw std::runtime_error(
            "Invalid literal: " + literal.value);
    }
    catch (const std::out_of_range &)
    {
        throw std::runtime_error(
            "Literal is out of range: " + literal.value);
    }

    throw std::runtime_error("Unsupported literal datatype");
}

// Execute SELECT queries.
void Executor::executeSelect(const SelectQuery &query)
{
    GenericTable table = openTable(query.tableName);
    const Schema &schema = table.getSchema();

    std::vector<int> selectedColumns;

    if (query.columns.size() == 1 && query.columns[0] == "*")
    {
        for (int i = 0; i < schema.size(); ++i)
            selectedColumns.push_back(i);
    }
    else
    {
        for (const std::string &columnName : query.columns)
        {
            int index = schema.getColumnIndex(columnName);

            if (index < 0)
                throw std::runtime_error(
                    "Column does not exist: " + columnName);

            selectedColumns.push_back(index);
        }
    }

    // Print selected column headers.
    for (std::size_t i = 0; i < selectedColumns.size(); ++i)
    {
        if (i > 0)
            std::cout << " | ";

        std::cout << schema.getColumn(selectedColumns[i]).getName();
    }

    std::cout << '\n';

    for (std::size_t i = 0; i < selectedColumns.size(); ++i)
    {
        if (i > 0)
            std::cout << "-+-";

        std::cout << "--------";
    }

    std::cout << '\n';

    int totalRows = table.size();
    int printedRows = 0;

    for (int i = 0; i < totalRows; ++i)
    {
        if (query.limit >= 0 && printedRows >= query.limit)
            break;

        Row row = table.get(i);

        if (query.hasCondition &&
            !evaluateCondition(row, schema, query.condition))
        {
            continue;
        }

        for (std::size_t j = 0; j < selectedColumns.size(); ++j)
        {
            if (j > 0)
                std::cout << " | ";

            const Value &value = row.getValue(selectedColumns[j]);

            switch (value.getType())
            {
            case DataType::INT:
                std::cout << value.asInt();
                break;

            case DataType::DOUBLE:
                std::cout << value.asDouble();
                break;

            case DataType::BOOLEAN:
                std::cout << (value.asBool() ? "TRUE" : "FALSE");
                break;

            case DataType::VARCHAR:
                std::cout << value.asString();
                break;
            }
        }

        std::cout << '\n';
        ++printedRows;
    }

    std::cout << printedRows << " row(s) selected.\n";
}

// Execute INSERT queries.
void Executor::executeInsert(const InsertQuery &query)
{
    if (!catalog.tableExists(query.tableName))
        throw std::runtime_error(
            "Table does not exist: " + query.tableName);

    Schema schema = catalog.getSchema(query.tableName);

    if (query.values.size() != static_cast<std::size_t>(schema.size()))
        throw std::runtime_error(
            "Column count does not match value count");

    Row row;

    for (int i = 0; i < schema.size(); ++i)
    {
        DataType expectedType = schema.getColumn(i).getType();

        Value value = convertLiteral(
            query.values[i],
            expectedType);

        row.addValue(value);
    }

    GenericTable table = openTable(query.tableName);

    if (!table.insert(row))
        throw std::runtime_error("Failed to insert row");

    std::cout << "1 row inserted.\n";
}

// Dispatch parsed SQL queries to their execution methods.
void Executor::execute(const Query &query)
{
    if (query.type == QueryType::SELECT)
    {
        executeSelect(query.select);
        return;
    }

    if (query.type == QueryType::CREATE_TABLE)
    {
        const CreateTableQuery &createQuery = query.createTable;

        if (catalog.tableExists(createQuery.tableName))
            throw std::runtime_error(
                "Table already exists: " + createQuery.tableName);

        Schema schema;

        for (const ColumnDefinition &column : createQuery.columns)
        {
            DataType type = parseDataType(column.type);
            schema.addColumn(column.name, type);
        }

        bool success = catalog.createTable(
            createQuery.tableName,
            schema);

        if (!success)
            throw std::runtime_error(
                "Failed to create table: " + createQuery.tableName);

        catalogStorage.save(catalog);
        
        std::cout << "Table '"
                  << createQuery.tableName
                  << "' created successfully.\n";
        return;
    }

    if (query.type == QueryType::INSERT)
    {
        executeInsert(query.insert);
        return;
    }

    if (query.type == QueryType::UPDATE)
    {
        executeUpdate(query.update);
        return;
    }

    if (query.type == QueryType::DELETE)
    {
        executeDelete(query.remove);
        return;
    }
    throw std::runtime_error("Unsupported query type");
}

void Executor::executeUpdate(const UpdateQuery &query)
{
    GenericTable table = openTable(query.tableName);
    const Schema &schema = table.getSchema();

    int columnIndex = schema.getColumnIndex(query.column);

    if (columnIndex < 0)
    {
        throw std::runtime_error(
            "Column does not exist: " + query.column);
    }

    Value newValue = convertLiteral(
        query.value,
        schema.getColumn(columnIndex).getType());

    int updatedCount = 0;
    int totalRows = table.size();

    for (int i = 0; i < totalRows; ++i)
    {
        Row currentRow = table.get(i);

        if (query.hasCondition &&
            !evaluateCondition(currentRow, schema, query.condition))
        {
            continue;
        }

        Row updatedRow;

        for (int column = 0; column < schema.size(); ++column)
        {
            if (column == columnIndex)
                updatedRow.addValue(newValue);
            else
                updatedRow.addValue(currentRow.getValue(column));
        }

        if (!table.updateRow(i, updatedRow))
        {
            throw std::runtime_error(
                "Could not update row " + std::to_string(i) +
                ". The updated record may not fit in its page.");
        }

        ++updatedCount;
    }

    std::cout << updatedCount << " row(s) updated.\n";
}

void Executor::executeDelete(const DeleteQuery &query)
{
    GenericTable table = openTable(query.tableName);

    int deletedCount = 0;
    int index = 0;

    while (index < table.size())
    {
        Row row = table.get(index);

        if (query.hasCondition &&
            !evaluateCondition(row, table.getSchema(), query.condition))
        {
            ++index;
            continue;
        }

        if (!table.deleteRow(index))
        {
            throw std::runtime_error(
                "Could not delete row " + std::to_string(index));
        }

        ++deletedCount;
    }

    std::cout << deletedCount << " row(s) deleted.\n";
}
