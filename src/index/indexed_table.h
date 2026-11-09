#ifndef INDEXED_TABLE_H
#define INDEXED_TABLE_H

#include "index_manager.h"
#include "../core/table/generic_table.h"

#include <cstdint>
#include <utility>
#include <vector>

class IndexedTable
{
private:
    GenericTable& table;
    IndexManager& indexManager;
    int indexedColumn;

    bool extractKey(
        const Row& row,
        int64_t& key
    ) const;

public:
    IndexedTable(
        GenericTable& table,
        IndexManager& indexManager,
        int indexedColumn
    );

    RecordId insert(const Row& row);

    std::vector<Row> search(int64_t key);

    std::vector<std::pair<int64_t, Row>> rangeSearch(
        int64_t lower,
        int64_t upper,
        bool includeLower = true,
        bool includeUpper = true
    );

    bool update(
        const RecordId& rid,
        const Row& newRow
    );

    bool erase(const RecordId& rid);

    size_t indexSize() const;

    size_t distinctKeyCount() const;
};

#endif