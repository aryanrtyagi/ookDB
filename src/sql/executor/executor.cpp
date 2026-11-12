
#include "executor.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <cmath>
#include <sstream>

static DataType parseDataType(const std::string& name)
{
    std::string type = name;

    std::transform(type.begin(), type.end(), type.begin(),
        [](unsigned char c) {
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

    throw std::runtime_error("Unsupported datatype: " + name);
}

static std::string valueToString(const Value& value)
{
    switch (value.getType())
    {
        case DataType::INT:
            return std::to_string(value.asInt());

        case DataType::DOUBLE:
        {
            std::ostringstream out;
            out << value.asDouble();
            return out.str();
        }

        case DataType::BOOLEAN:
            return value.asBool() ? "TRUE" : "FALSE";

        case DataType::VARCHAR:
            return value.asString();
    }

    return "";
}

static bool likeMatch(
    const std::string& text,
    const std::string& pattern,
    std::size_t i = 0,
    std::size_t j = 0)
{
    if (j == pattern.size())
        return i == text.size();

    if (pattern[j] == '%')
    {
        for (std::size_t k = i; k <= text.size(); ++k)
            if (likeMatch(text, pattern, k, j + 1))
                return true;

        return false;
    }

    if (pattern[j] == '_')
        return i < text.size() &&
               likeMatch(text, pattern, i + 1, j + 1);

    return i < text.size() &&
           text[i] == pattern[j] &&
           likeMatch(text, pattern, i + 1, j + 1);
}

Executor::Executor(
    Pager& pager,
    BufferPool& bufferPool,
    PageAllocator& pageAllocator,
    Catalog& catalog,
    CatalogStorage& catalogStorage)
    : pager(pager),
      bufferPool(bufferPool),
      pageAllocator(pageAllocator),
      catalog(catalog),
      catalogStorage(catalogStorage)
{
    loadIndexManagers();
}

void Executor::loadIndexManagers()
{
    indexManagers.clear();

    for (const IndexMetadata& metadata : catalog.getIndexes())
    {
        indexManagers[metadata.indexName] =
            std::make_unique<IndexManager>(metadata.filePath);
    }
}

GenericTable Executor::openTable(const std::string& tableName)
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
        firstPage
    );
}

Value Executor::convertLiteral(
    const Literal& literal,
    DataType expectedType)
{
    try
    {
        switch (expectedType)
        {
            case DataType::INT:
            {
                std::size_t pos = 0;
                int v = std::stoi(literal.value, &pos);

                if (pos != literal.value.size())
                    throw std::runtime_error("Invalid integer");

                return Value(v);
            }

            case DataType::DOUBLE:
            {
                std::size_t pos = 0;
                double v = std::stod(literal.value, &pos);

                if (pos != literal.value.size())
                    throw std::runtime_error("Invalid number");

                return Value(v);
            }

            case DataType::VARCHAR:
                return Value(literal.value);

            case DataType::BOOLEAN:
            {
                std::string v = literal.value;

                std::transform(v.begin(), v.end(), v.begin(),
                    [](unsigned char c) {
                        return static_cast<char>(std::tolower(c));
                    });

                if (v == "true")
                    return Value(true);

                if (v == "false")
                    return Value(false);

                throw std::runtime_error("Expected TRUE or FALSE");
            }
        }
    }
    catch (const std::invalid_argument&)
    {
        throw std::runtime_error("Invalid literal: " + literal.value);
    }
    catch (const std::out_of_range&)
    {
        throw std::runtime_error("Literal out of range: " + literal.value);
    }

    throw std::runtime_error("Unsupported literal");
}

bool Executor::evaluateCondition(
    const Row& row,
    const Schema& schema,
    const Condition& condition)
{
    if (condition.type == ConditionType::AND)
        return condition.left && condition.right &&
            evaluateCondition(row, schema, *condition.left) &&
            evaluateCondition(row, schema, *condition.right);

    if (condition.type == ConditionType::OR)
        return condition.left && condition.right &&
            (evaluateCondition(row, schema, *condition.left) ||
             evaluateCondition(row, schema, *condition.right));

    if (condition.type == ConditionType::NOT)
        return condition.left &&
            !evaluateCondition(row, schema, *condition.left);

    int index = schema.getColumnIndex(condition.column);

    if (index < 0)
        throw std::runtime_error(
            "Column does not exist: " + condition.column
        );

    const Value& actual = row.getValue(index);
    DataType type = schema.getColumn(index).getType();

    auto convert = [&](const Literal& literal) {
        return convertLiteral(literal, type);
    };

    auto compare = [&](const Value& a, const Value& b) -> int
    {
        if (a.getType() != b.getType())
            throw std::runtime_error("Comparison type mismatch");

        switch (a.getType())
        {
            case DataType::INT:
                return a.asInt() < b.asInt() ? -1 :
                       a.asInt() > b.asInt() ? 1 : 0;

            case DataType::DOUBLE:
                return a.asDouble() < b.asDouble() ? -1 :
                       a.asDouble() > b.asDouble() ? 1 : 0;

            case DataType::VARCHAR:
                return a.asString() < b.asString() ? -1 :
                       a.asString() > b.asString() ? 1 : 0;

            case DataType::BOOLEAN:
                return a.asBool() == b.asBool() ? 0 :
                       a.asBool() ? 1 : -1;
        }

        return 0;
    };

    if (condition.op == ComparisonOperator::IN ||
        condition.op == ComparisonOperator::NOT_IN)
    {
        bool found = false;

        for (const Literal& literal : condition.values)
            if (compare(actual, convert(literal)) == 0)
                found = true;

        return condition.op == ComparisonOperator::IN
            ? found : !found;
    }

    if (condition.op == ComparisonOperator::BETWEEN ||
        condition.op == ComparisonOperator::NOT_BETWEEN)
    {
        int low = compare(actual, convert(condition.value));
        int high = compare(actual, convert(condition.secondValue));

        bool inside = low >= 0 && high <= 0;

        return condition.op == ComparisonOperator::BETWEEN
            ? inside : !inside;
    }

    if (condition.op == ComparisonOperator::LIKE ||
        condition.op == ComparisonOperator::NOT_LIKE)
    {
        if (type != DataType::VARCHAR)
            throw std::runtime_error("LIKE requires a VARCHAR column");

        bool matched = likeMatch(
            actual.asString(),
            condition.value.value
        );

        return condition.op == ComparisonOperator::LIKE
            ? matched : !matched;
    }

    Value expected = convert(condition.value);
    int cmp = compare(actual, expected);

    switch (condition.op)
    {
        case ComparisonOperator::EQUAL: return cmp == 0;
        case ComparisonOperator::NOT_EQUAL: return cmp != 0;
        case ComparisonOperator::GREATER: return cmp > 0;
        case ComparisonOperator::LESS: return cmp < 0;
        case ComparisonOperator::GREATER_EQUAL: return cmp >= 0;
        case ComparisonOperator::LESS_EQUAL: return cmp <= 0;
        default: break;
    }

    throw std::runtime_error("Unsupported comparison operator");
}

QueryResult Executor::executeInsert(const InsertQuery& query)
{
    QueryResult result;

    GenericTable table = openTable(query.tableName);
    const Schema& schema = table.getSchema();

    if (query.rows.empty())
        throw std::runtime_error("INSERT has no rows");

    for (const auto& values : query.rows)
    {
        if (values.size() != static_cast<std::size_t>(schema.size()))
            throw std::runtime_error(
                "Column count does not match value count"
            );

        Row row;

        for (int i = 0; i < schema.size(); ++i)
            row.addValue(
                convertLiteral(values[i], schema.getColumn(i).getType())
            );

        RecordId rid = table.insertWithId(row);

        if (!rid.isValid())
            throw std::runtime_error("Failed to insert row");

        std::vector<std::pair<IndexManager*, int64_t>> inserted;

        try
        {
            for (const IndexMetadata& metadata : catalog.getIndexes())
            {
                if (metadata.tableName != query.tableName)
                    continue;

                int column = schema.getColumnIndex(metadata.columnName);

                if (column < 0 ||
                    schema.getColumn(column).getType() != DataType::INT)
                    throw std::runtime_error("Invalid indexed column");

                auto it = indexManagers.find(metadata.indexName);

                if (it == indexManagers.end())
                    throw std::runtime_error("Index manager not loaded");

                int64_t key = row.getValue(column).asInt();

                if (!it->second->insert(key, rid))
                    throw std::runtime_error("Index update failed");

                inserted.emplace_back(it->second.get(), key);
            }
        }
        catch (...)
        {
            for (auto& entry : inserted)
                entry.first->erase(entry.second, rid);

            table.deleteRow(rid);
            throw;
        }

        ++result.affectedRows;
    }

    result.message =
        std::to_string(result.affectedRows) + " row(s) inserted.";

    return result;
}

QueryResult Executor::executeSelect(const SelectQuery& query)
{
    if (!query.joins.empty())
        throw std::runtime_error(
            "JOIN execution requires a combined-row schema. "
            "The current storage Row represents one table only."
        );

    QueryResult result;
    GenericTable table = openTable(query.tableName);
    const Schema& schema = table.getSchema();

    std::vector<int> selected;

    if (query.selectColumns.empty())
    {
        if (query.columns.size() == 1 && query.columns[0] == "*")
        {
            for (int i = 0; i < schema.size(); ++i)
                selected.push_back(i);
        }
        else
        {
            for (const std::string& name : query.columns)
            {
                int index = schema.getColumnIndex(name);

                if (index < 0)
                    throw std::runtime_error("Unknown column: " + name);

                selected.push_back(index);
            }
        }

        for (int index : selected)
            result.columns.push_back(schema.getColumn(index).getName());
    }
    else
    {
        for (const SelectColumn& column : query.selectColumns)
        {
            if (column.aggregate != AggregateType::NONE)
                continue;

            if (column.isStar)
            {
                for (int i = 0; i < schema.size(); ++i)
                {
                    selected.push_back(i);
                    result.columns.push_back(
                        schema.getColumn(i).getName()
                    );
                }
                continue;
            }

            int index = schema.getColumnIndex(column.name);

            if (index < 0)
                throw std::runtime_error("Unknown column: " + column.name);

            selected.push_back(index);
            result.columns.push_back(
                column.alias.empty()
                    ? column.name
                    : column.alias
            );
        }
    }

    struct SourceRow
    {
        Row row;
        std::vector<Value> projected;
    };

    std::vector<SourceRow> matched;

    for (const RecordId& rid : table.getAllRecordIds())
    {
        Row row = table.get(rid);

        if (query.hasCondition &&
            !evaluateCondition(row, schema, query.condition))
            continue;

        SourceRow item;
        item.row = row;

        for (int index : selected)
            item.projected.push_back(row.getValue(index));

        matched.push_back(std::move(item));
    }

    // DISTINCT
    if (query.distinct)
    {
        std::set<std::string> seen;
        std::vector<SourceRow> uniqueRows;

        for (auto& item : matched)
        {
            std::string key;

            for (const Value& value : item.projected)
                key += valueToString(value) + '\x1f';

            if (seen.insert(key).second)
                uniqueRows.push_back(std::move(item));
        }

        matched = std::move(uniqueRows);
    }

    // GROUP BY and aggregate execution
    bool hasAggregate = false;

    for (const SelectColumn& column : query.selectColumns)
        if (column.aggregate != AggregateType::NONE)
            hasAggregate = true;

    if (!query.groupBy.empty() || hasAggregate)
    {
        if (query.hasHaving)
            throw std::runtime_error(
                "HAVING currently requires aggregate alias evaluation "
                "which is not represented in Condition."
            );

        std::vector<int> groupIndexes;

        for (const std::string& name : query.groupBy)
        {
            int index = schema.getColumnIndex(name);

            if (index < 0)
                throw std::runtime_error("Unknown GROUP BY column: " + name);

            groupIndexes.push_back(index);
        }

        std::map<std::string, std::vector<Row>> groups;

        for (const auto& item : matched)
        {
            std::string key;

            for (int index : groupIndexes)
                key += valueToString(item.row.getValue(index)) + '\x1f';

            groups[key].push_back(item.row);
        }

        if (groups.empty() && groupIndexes.empty())
            groups[""] = {};

        result.columns.clear();

        for (const SelectColumn& column : query.selectColumns)
        {
            if (column.aggregate == AggregateType::NONE)
                result.columns.push_back(
                    column.alias.empty() ? column.name : column.alias
                );
            else
            {
                std::string name;

                switch (column.aggregate)
                {
                    case AggregateType::COUNT: name = "COUNT"; break;
                    case AggregateType::SUM: name = "SUM"; break;
                    case AggregateType::AVG: name = "AVG"; break;
                    case AggregateType::MIN: name = "MIN"; break;
                    case AggregateType::MAX: name = "MAX"; break;
                    default: break;
                }

                result.columns.push_back(
                    column.alias.empty()
                        ? name + "(" + column.name + ")"
                        : column.alias
                );
            }
        }

        for (const auto& group : groups)
        {
            const std::vector<Row>& rows = group.second;
            std::vector<Value> output;

            for (const SelectColumn& column : query.selectColumns)
            {
                if (column.aggregate == AggregateType::NONE)
                {
                    int index = schema.getColumnIndex(column.name);

                    if (index < 0)
                        throw std::runtime_error(
                            "Unknown selected column: " + column.name
                        );

                    if (rows.empty())
                        throw std::runtime_error("Empty grouped row");

                    output.push_back(rows.front().getValue(index));
                    continue;
                }

                if (column.aggregate == AggregateType::COUNT)
                {
                    output.emplace_back(static_cast<int>(rows.size()));
                    continue;
                }

                if (column.isStar)
                    throw std::runtime_error(
                        "Only COUNT(*) supports aggregate star"
                    );

                int index = schema.getColumnIndex(column.name);

                if (index < 0)
                    throw std::runtime_error(
                        "Unknown aggregate column: " + column.name
                    );

                if (rows.empty())
                {
                    output.emplace_back(0);
                    continue;
                }

                const Value& first = rows.front().getValue(index);

                if (first.getType() == DataType::VARCHAR)
                {
                    if (column.aggregate != AggregateType::MIN &&
                        column.aggregate != AggregateType::MAX)
                        throw std::runtime_error(
                            "SUM and AVG require numeric columns"
                        );

                    std::string chosen = first.asString();

                    for (const Row& row : rows)
                    {
                        const std::string& current =
                            row.getValue(index).asString();

                        if (column.aggregate == AggregateType::MIN)
                            chosen = std::min(chosen, current);
                        else
                            chosen = std::max(chosen, current);
                    }

                    output.emplace_back(chosen);
                    continue;
                }

                double sum = 0.0;
                double minValue = std::numeric_limits<double>::max();
                double maxValue = std::numeric_limits<double>::lowest();

                for (const Row& row : rows)
                {
                    const Value& value = row.getValue(index);

                    double number =
                        value.getType() == DataType::INT
                            ? value.asInt()
                            : value.asDouble();

                    sum += number;
                    minValue = std::min(minValue, number);
                    maxValue = std::max(maxValue, number);
                }

                switch (column.aggregate)
                {
                    case AggregateType::SUM:
                        output.emplace_back(sum);
                        break;

                    case AggregateType::AVG:
                        output.emplace_back(sum / rows.size());
                        break;

                    case AggregateType::MIN:
                        output.emplace_back(minValue);
                        break;

                    case AggregateType::MAX:
                        output.emplace_back(maxValue);
                        break;

                    default:
                        break;
                }
            }

            result.rows.push_back(std::move(output));
        }
    }
    else
    {
        for (auto& item : matched)
            result.rows.push_back(std::move(item.projected));
    }

    // ORDER BY
    for (auto it = query.orderBy.rbegin();
         it != query.orderBy.rend(); ++it)
    {
        int orderIndex = -1;

        for (std::size_t i = 0; i < result.columns.size(); ++i)
        {
            if (result.columns[i] == it->column)
            {
                orderIndex = static_cast<int>(i);
                break;
            }
        }

        if (orderIndex < 0)
        {
            int sourceIndex = schema.getColumnIndex(it->column);

            if (sourceIndex < 0)
                throw std::runtime_error(
                    "Unknown ORDER BY column: " + it->column
                );

            for (std::size_t i = 0; i < selected.size(); ++i)
                if (selected[i] == sourceIndex)
                    orderIndex = static_cast<int>(i);
        }

        if (orderIndex < 0)
            throw std::runtime_error(
                "ORDER BY column must be selected in this implementation"
            );

        std::sort(
            result.rows.begin(),
            result.rows.end(),
            [orderIndex, ascending = it->ascending](
                const std::vector<Value>& a,
                const std::vector<Value>& b)
            {
                const Value& x = a[orderIndex];
                const Value& y = b[orderIndex];

                int cmp = 0;

                if (x.getType() == DataType::INT)
                    cmp = x.asInt() < y.asInt() ? -1 :
                          x.asInt() > y.asInt() ? 1 : 0;
                else if (x.getType() == DataType::DOUBLE)
                    cmp = x.asDouble() < y.asDouble() ? -1 :
                          x.asDouble() > y.asDouble() ? 1 : 0;
                else if (x.getType() == DataType::VARCHAR)
                    cmp = x.asString() < y.asString() ? -1 :
                          x.asString() > y.asString() ? 1 : 0;
                else
                    cmp = x.asBool() == y.asBool() ? 0 :
                          x.asBool() ? 1 : -1;

                return ascending ? cmp < 0 : cmp > 0;
            }
        );
    }

    if (query.limit >= 0 &&
        result.rows.size() > static_cast<std::size_t>(query.limit))
        result.rows.resize(query.limit);

    result.affectedRows = static_cast<int>(result.rows.size());
    result.message = "SELECT completed.";

    return result;
}

QueryResult Executor::executeUpdate(const UpdateQuery& query)
{
    QueryResult result;

    GenericTable table = openTable(query.tableName);
    const Schema& schema = table.getSchema();

    int columnIndex = schema.getColumnIndex(query.column);

    if (columnIndex < 0)
        throw std::runtime_error("Unknown column: " + query.column);

    Value newValue = convertLiteral(
        query.value,
        schema.getColumn(columnIndex).getType()
    );

    for (const RecordId& rid : table.getAllRecordIds())
    {
        Row oldRow = table.get(rid);

        if (query.hasCondition &&
            !evaluateCondition(oldRow, schema, query.condition))
            continue;

        Row newRow;

        for (int i = 0; i < schema.size(); ++i)
            newRow.addValue(
                i == columnIndex ? newValue : oldRow.getValue(i)
            );

        if (!table.updateRow(rid, newRow))
            throw std::runtime_error("Could not update row");

        ++result.affectedRows;
    }

    result.message = "UPDATE completed.";
    return result;
}

QueryResult Executor::executeDelete(const DeleteQuery& query)
{
    QueryResult result;

    GenericTable table = openTable(query.tableName);
    const Schema& schema = table.getSchema();

    for (const RecordId& rid : table.getAllRecordIds())
    {
        Row row = table.get(rid);

        if (query.hasCondition &&
            !evaluateCondition(row, schema, query.condition))
            continue;

        if (!table.deleteRow(rid))
            throw std::runtime_error("Could not delete row");

        ++result.affectedRows;
    }

    result.message = "DELETE completed.";
    return result;
}

QueryResult Executor::executeCreateIndex(const CreateIndexQuery& query)
{
    QueryResult result;

    if (!catalog.tableExists(query.tableName))
        throw std::runtime_error("Table does not exist");

    if (catalog.indexExists(query.indexName))
        throw std::runtime_error("Index already exists");

    if (catalog.hasIndex(query.tableName, query.columnName))
        throw std::runtime_error("Column already indexed");

    const Schema& schema = catalog.getSchema(query.tableName);
    int column = schema.getColumnIndex(query.columnName);

    if (column < 0)
        throw std::runtime_error("Column does not exist");

    if (schema.getColumn(column).getType() != DataType::INT)
        throw std::runtime_error("Only INT indexes are supported");

    std::string path = "data/" + query.indexName + ".idx";
    std::remove(path.c_str());

    auto manager = std::make_unique<IndexManager>(path);
    GenericTable table = openTable(query.tableName);

    for (const RecordId& rid : table.getAllRecordIds())
    {
        Row row = table.get(rid);

        if (!manager->insert(row.getValue(column).asInt(), rid))
            throw std::runtime_error("Failed to build index");
    }

    if (!catalog.createIndex(
            query.indexName,
            query.tableName,
            query.columnName,
            path))
    {
        std::remove(path.c_str());
        throw std::runtime_error("Could not register index");
    }

    catalogStorage.save(catalog);
    indexManagers[query.indexName] = std::move(manager);

    result.message = "Index created.";
    return result;
}

QueryResult Executor::execute(const Query& query)
{
    switch (query.type)
    {
        case QueryType::CREATE_INDEX:
            return executeCreateIndex(query.createIndex);

        case QueryType::SELECT:
            return executeSelect(query.select);

        case QueryType::INSERT:
            return executeInsert(query.insert);

        case QueryType::UPDATE:
            return executeUpdate(query.update);

        case QueryType::DELETE:
            return executeDelete(query.remove);

        case QueryType::CREATE_TABLE:
        {
            QueryResult result;

            if (catalog.tableExists(query.createTable.tableName))
                throw std::runtime_error("Table already exists");

            Schema schema;

            for (const ColumnDefinition& column : query.createTable.columns)
                schema.addColumn(column.name, parseDataType(column.type));

            if (!catalog.createTable(query.createTable.tableName, schema))
                throw std::runtime_error("Failed to create table");

            catalogStorage.save(catalog);
            result.message = "Table created.";
            return result;
        }

        case QueryType::SHOW_TABLES:
        case QueryType::DESCRIBE:
        case QueryType::DROP_TABLE:
        case QueryType::ALTER_TABLE:
            throw std::runtime_error(
                "This DDL operation needs matching Catalog support. "
                "Parser support is present, but storage/catalog mutation "
                "has not been implemented."
            );
    }

    throw std::runtime_error("Unsupported query type");
}
