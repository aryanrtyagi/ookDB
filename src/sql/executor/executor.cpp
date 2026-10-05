#include "executor.h"

#include <iostream>
#include <stdexcept>

Executor::Executor(
    Pager &pager,
    BufferPool &bufferPool,
    PageAllocator &pageAllocator,
    Catalog &catalog)
    : pager(pager),
      bufferPool(bufferPool),
      pageAllocator(pageAllocator),
      catalog(catalog)
{
}

GenericTable Executor::openTable(
    const std::string &tableName)
{
    if (!catalog.tableExists(tableName))
    {
        throw std::runtime_error(
            "Table does not exist: " +
            tableName);
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
        firstPage);
}

static void printValue(
    const Value &value)
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

    // NUMBER literal
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

    // STRING literal
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

void Executor::execute(
    const SelectQuery &query)
{
    GenericTable table =
        openTable(query.tableName);

    const Schema &schema =
        table.getSchema();

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
    // SELECT column1, column2, ...
    else
    {
        for (const std::string &column :
             query.columns)
        {
            int index =
                schema.getColumnIndex(column);

            if (index == -1)
            {
                throw std::runtime_error(
                    "Unknown column: " + column);
            }

            columnIndexes.push_back(index);
        }
    }

    int rowCount =
        table.size();

    for (int i = 0; i < rowCount; i++)
    {
        Row row =
            table.get(i);

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

        for (int j = 0;
             j < columnIndexes.size();
             j++)
        {
            Value value =
                row.getValue(
                    columnIndexes[j]);

            printValue(value);

            if (j + 1 < columnIndexes.size())
            {
                std::cout << " | ";
            }
        }

        std::cout << "\n";
    }
}