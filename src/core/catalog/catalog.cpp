
#include "catalog.h"
#include "../allocation/page_allocator.h"

#include <stdexcept>

Catalog::Catalog(PageAllocator& pageAllocator)
    : pageAllocator(pageAllocator)
{
}

// ============================================================
// TABLE OPERATIONS
// ============================================================

bool Catalog::createTable(
    const std::string& name,
    const Schema& schema
)
{
    if (tableExists(name))
        return false;

    int firstPage = pageAllocator.allocatePage();

    if (firstPage < 0)
        return false;

    TableMetadata metadata;

    metadata.name = name;
    metadata.schema = schema;
    metadata.firstPage = firstPage;

    tables.push_back(metadata);

    return true;
}

bool Catalog::tableExists(
    const std::string& name
) const
{
    for (const auto& table : tables)
    {
        if (table.name == name)
            return true;
    }

    return false;
}

const Schema& Catalog::getSchema(
    const std::string& name
) const
{
    for (const auto& table : tables)
    {
        if (table.name == name)
            return table.schema;
    }

    throw std::runtime_error(
        "Table not found: " + name
    );
}

int Catalog::getFirstPage(
    const std::string& name
) const
{
    for (const auto& table : tables)
    {
        if (table.name == name)
            return table.firstPage;
    }

    throw std::runtime_error(
        "Table not found: " + name
    );
}

std::vector<std::string> Catalog::listTables() const
{
    std::vector<std::string> names;

    for (const auto& table : tables)
        names.push_back(table.name);

    return names;
}

const std::vector<TableMetadata>&
Catalog::getTables() const
{
    return tables;
}

void Catalog::restoreTable(
    const std::string& name,
    const Schema& schema,
    int firstPage
)
{
    if (tableExists(name))
        throw std::runtime_error(
            "Duplicate table in catalog: " + name
        );

    TableMetadata metadata;

    metadata.name = name;
    metadata.schema = schema;
    metadata.firstPage = firstPage;

    tables.push_back(metadata);
}

// ============================================================
// INDEX OPERATIONS
// ============================================================

bool Catalog::createIndex(
    const std::string& indexName,
    const std::string& tableName,
    const std::string& columnName,
    const std::string& filePath
)
{
    if (!tableExists(tableName))
        return false;

    if (indexExists(indexName))
        return false;

    if (hasIndex(tableName, columnName))
        return false;

    const Schema& schema = getSchema(tableName);

    if (schema.getColumnIndex(columnName) < 0)
        return false;

    IndexMetadata metadata;

    metadata.indexName = indexName;
    metadata.tableName = tableName;
    metadata.columnName = columnName;
    metadata.filePath = filePath;

    indexes.push_back(metadata);

    return true;
}

bool Catalog::indexExists(
    const std::string& indexName
) const
{
    for (const auto& index : indexes)
    {
        if (index.indexName == indexName)
            return true;
    }

    return false;
}

bool Catalog::hasIndex(
    const std::string& tableName,
    const std::string& columnName
) const
{
    return findIndex(tableName, columnName) != nullptr;
}

const IndexMetadata* Catalog::findIndex(
    const std::string& tableName,
    const std::string& columnName
) const
{
    for (const auto& index : indexes)
    {
        if (index.tableName == tableName &&
            index.columnName == columnName)
        {
            return &index;
        }
    }

    return nullptr;
}

const std::vector<IndexMetadata>&
Catalog::getIndexes() const
{
    return indexes;
}

void Catalog::restoreIndex(
    const std::string& indexName,
    const std::string& tableName,
    const std::string& columnName,
    const std::string& filePath
)
{
    if (!tableExists(tableName))
        throw std::runtime_error(
            "Index references an unknown table: " + tableName
        );

    if (indexExists(indexName))
        throw std::runtime_error(
            "Duplicate index name: " + indexName
        );

    if (hasIndex(tableName, columnName))
        throw std::runtime_error(
            "Duplicate index for table column"
        );

    if (getSchema(tableName).getColumnIndex(columnName) < 0)
        throw std::runtime_error(
            "Index references an unknown column: " + columnName
        );

    IndexMetadata metadata;

    metadata.indexName = indexName;
    metadata.tableName = tableName;
    metadata.columnName = columnName;
    metadata.filePath = filePath;

    indexes.push_back(metadata);
}

// ============================================================
// CLEAR CATALOG
// ============================================================

void Catalog::clear()
{
    tables.clear();
    indexes.clear();
}
