#include "catalog.h"

#include <stdexcept>


Catalog::Catalog()
{
}


// ============================================================
// Create table
// ============================================================

bool Catalog::createTable(
    const std::string& name,
    const Schema& schema,
    int firstPage
)
{
    if (tableExists(name))
    {
        return false;
    }

    TableMetadata metadata;

    metadata.name = name;
    metadata.schema = schema;
    metadata.firstPage = firstPage;

    tables[name] = metadata;

    return true;
}


// ============================================================
// Check whether table exists
// ============================================================

bool Catalog::tableExists(
    const std::string& name
) const
{
    return tables.find(name) != tables.end();
}


// ============================================================
// Get schema
// ============================================================

const Schema& Catalog::getSchema(
    const std::string& name
) const
{
    auto it = tables.find(name);

    if (it == tables.end())
    {
        throw std::runtime_error(
            "Table does not exist: " + name
        );
    }

    return it->second.schema;
}


// ============================================================
// Get first page
// ============================================================

int Catalog::getFirstPage(
    const std::string& name
) const
{
    auto it = tables.find(name);

    if (it == tables.end())
    {
        throw std::runtime_error(
            "Table does not exist: " + name
        );
    }

    return it->second.firstPage;
}


// ============================================================
// List tables
// ============================================================

std::vector<std::string>
Catalog::listTables() const
{
    std::vector<std::string> result;

    for (const auto& entry : tables)
    {
        result.push_back(entry.first);
    }

    return result;
}


// ============================================================
// Get all metadata
// ============================================================

const std::map<std::string, TableMetadata>&
Catalog::getTables() const
{
    return tables;
}


// ============================================================
// Clear catalog
// ============================================================

void Catalog::clear()
{
    tables.clear();
}