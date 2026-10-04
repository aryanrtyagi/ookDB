#ifndef CATALOG_H
#define CATALOG_H

#include <string>
#include <vector>

#include "record/schema.h"

class PageAllocator;

struct TableMetadata
{
    std::string name;
    Schema schema;
    int firstPage;
};

class Catalog
{
private:
    std::vector<TableMetadata> tables;

    // Page allocator used to assign storage pages to tables.
    PageAllocator& pageAllocator;

public:

    explicit Catalog(PageAllocator& pageAllocator);

    bool createTable(
        const std::string& name,
        const Schema& schema
    );

    bool tableExists(
        const std::string& name
    ) const;

    const Schema& getSchema(
        const std::string& name
    ) const;

    int getFirstPage(
        const std::string& name
    ) const;

    std::vector<std::string> listTables() const;

    const std::vector<TableMetadata>&
    getTables() const;

    void clear();

    // Restore an existing table from persisted catalog metadata.
    // Does NOT allocate a new page.
    void restoreTable(
        const std::string& name,
        const Schema& schema,
        int firstPage
    );
};

#endif