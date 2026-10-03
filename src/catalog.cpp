#include "catalog.h"
#include "page_allocator.h"

#include <stdexcept>


Catalog::Catalog(PageAllocator& pageAllocator)
    : pageAllocator(pageAllocator)
{
}


bool Catalog::createTable(
    const std::string& name,
    const Schema& schema
)
{
    // Don't allow duplicate table names.
    if (tableExists(name))
    {
        return false;
    }

    /*
        Ask PageAllocator for the first page
        belonging to this table.
    */
    int firstPage =
        pageAllocator.allocatePage();


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
        {
            return true;
        }
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
        {
            return table.schema;
        }
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
        {
            return table.firstPage;
        }
    }

    throw std::runtime_error(
        "Table not found: " + name
    );
}


std::vector<std::string>
Catalog::listTables() const
{
    std::vector<std::string> names;

    for (const auto& table : tables)
    {
        names.push_back(table.name);
    }

    return names;
}


const std::vector<TableMetadata>&
Catalog::getTables() const
{
    return tables;
}


void Catalog::clear()
{
    tables.clear();
}