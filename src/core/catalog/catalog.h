#ifndef CATALOG_H
#define CATALOG_H

#include <string>
#include <vector>

#include "../../record/schema.h"

class PageAllocator;

struct TableMetadata
{
    std::string name;
    Schema schema;
    int firstPage;
};

struct IndexMetadata
{
    std::string indexName;
    std::string tableName;
    std::string columnName;
    std::string filePath;
};

class Catalog
{
private:
    std::vector<TableMetadata> tables;
    std::vector<IndexMetadata> indexes;

    PageAllocator& pageAllocator;

public:

    explicit Catalog(PageAllocator& pageAllocator);

    // Table operations.
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

    void restoreTable(
        const std::string& name,
        const Schema& schema,
        int firstPage
    );

    // Index operations.
    bool createIndex(
        const std::string& indexName,
        const std::string& tableName,
        const std::string& columnName,
        const std::string& filePath
    );

    bool indexExists(
        const std::string& indexName
    ) const;

    bool hasIndex(
        const std::string& tableName,
        const std::string& columnName
    ) const;

    const IndexMetadata*
    findIndex(
        const std::string& tableName,
        const std::string& columnName
    ) const;

    const std::vector<IndexMetadata>&
    getIndexes() const;

    void restoreIndex(
        const std::string& indexName,
        const std::string& tableName,
        const std::string& columnName,
        const std::string& filePath
    );

    void clear();
};

#endif
