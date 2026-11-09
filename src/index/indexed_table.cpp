#include "indexed_table.h"

#include <limits>

IndexedTable::IndexedTable(
    GenericTable& table,
    IndexManager& indexManager,
    int indexedColumn
)
    : table(table),
      indexManager(indexManager),
      indexedColumn(indexedColumn)
{
}

bool IndexedTable::extractKey(
    const Row& row,
    int64_t& key
) const
{
    if (
        indexedColumn < 0 ||
        indexedColumn >= row.size()
    )
    {
        return false;
    }

    const Value& value = row.getValue(indexedColumn);

    if (value.getType() != DataType::INT)
        return false;

    key = value.asInt();

    return true;
}

RecordId IndexedTable::insert(const Row& row)
{
    int64_t key;

    if (!extractKey(row, key))
        return RecordId();

    RecordId rid = table.insertWithId(row);

    if (!rid.isValid())
        return RecordId();

    if (!indexManager.insert(key, rid))
    {
        // Best-effort rollback if index persistence fails.
        table.deleteRow(rid);
        return RecordId();
    }

    return rid;
}

std::vector<Row> IndexedTable::search(int64_t key)
{
    std::vector<Row> result;

    std::vector<RecordId> ids =
        indexManager.search(key);

    for (const RecordId& rid : ids)
    {
        result.push_back(table.get(rid));
    }

    return result;
}

std::vector<std::pair<int64_t, Row>>
IndexedTable::rangeSearch(
    int64_t lower,
    int64_t upper,
    bool includeLower,
    bool includeUpper
)
{
    std::vector<std::pair<int64_t, Row>> result;

    std::vector<IndexEntry> entries =
        indexManager.rangeSearch(
            lower,
            upper,
            includeLower,
            includeUpper
        );

    for (const auto& entry : entries)
    {
        result.emplace_back(
            entry.key,
            table.get(entry.rid)
        );
    }

    return result;
}

bool IndexedTable::update(
    const RecordId& rid,
    const Row& newRow
)
{
    int64_t newKey;

    if (!extractKey(newRow, newKey))
        return false;

    // Read the old row to determine its previous index key.
    Row oldRow = table.get(rid);

    int64_t oldKey;

    if (!extractKey(oldRow, oldKey))
        return false;

    if (oldKey == newKey)
    {
        return table.updateRow(rid, newRow);
    }

    // Update the underlying table first.
    if (!table.updateRow(rid, newRow))
        return false;

    // Remove old index entry.
    if (!indexManager.erase(oldKey, rid))
    {
        // Restore the old row if index removal failed.
        table.updateRow(rid, oldRow);
        return false;
    }

    // Add the new index entry.
    if (!indexManager.insert(newKey, rid))
    {
        // Best-effort rollback.
        indexManager.insert(oldKey, rid);
        table.updateRow(rid, oldRow);
        return false;
    }

    return true;
}

bool IndexedTable::erase(const RecordId& rid)
{
    Row oldRow = table.get(rid);

    int64_t oldKey;

    if (!extractKey(oldRow, oldKey))
        return false;

    // Remove the index entry first.
    if (!indexManager.erase(oldKey, rid))
        return false;

    // Then remove the actual record.
    if (!table.deleteRow(rid))
    {
        // Restore index if table deletion failed.
        indexManager.insert(oldKey, rid);
        return false;
    }

    return true;
}

size_t IndexedTable::indexSize() const
{
    return indexManager.size();
}

size_t IndexedTable::distinctKeyCount() const
{
    return indexManager.distinctSize();
}