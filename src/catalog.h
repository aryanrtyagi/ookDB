#ifndef CATALOG_H
#define CATALOG_H

#include <string>
#include <map>
#include <vector>

#include "record/schema.h"

struct TableMetadata
{
    std::string name;
    Schema schema;
    int firstPage;
};

class Catalog
{
private:
    std::map<std::string, TableMetadata> tables;

public:

    Catalog();

    bool createTable(
        const std::string& name,
        const Schema& schema,
        int firstPage
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

    const std::map<std::string, TableMetadata>&
    getTables() const;

    void clear();
};

#endif