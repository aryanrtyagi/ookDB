#include "index_manager.h"

#include <fstream>
#include <limits>
#include <cstdint>
#include <vector>

namespace
{
    const uint32_t INDEX_MAGIC = 0x41525949;
    const uint32_t INDEX_VERSION = 1;

    struct DiskIndexEntry
    {
        int64_t key;
        int32_t pageNumber;
        uint32_t slotNumber;
        uint32_t generation;
    };
}

IndexManager::IndexManager(
    const std::string& filePath,
    size_t maxKeys
)
    : tree(maxKeys),
      filePath(filePath),
      maxKeys(maxKeys)
{
    load();
}

bool IndexManager::insert(
    int64_t key,
    const RecordId& rid
)
{
    if (!rid.isValid())
        return false;

    tree.insert(key, rid);

    return save();
}

bool IndexManager::erase(
    int64_t key,
    const RecordId& rid
)
{
    if (!tree.erase(key, rid))
        return false;

    // Rebuild to remove empty leaf keys and maintain
    // correct distinct-key metadata.
    rebuildWithout(key, rid);

    return save();
}

void IndexManager::rebuildWithout(
    int64_t key,
    const RecordId& rid
)
{
    std::vector<IndexEntry> entries =
        tree.rangeSearch(
            std::numeric_limits<int64_t>::min(),
            std::numeric_limits<int64_t>::max()
        );

    tree = BPlusTree(maxKeys);

    for (const auto& entry : entries)
    {
        // The requested RecordId was already erased.
        // Reinsert all remaining entries.
        tree.insert(entry.key, entry.rid);
    }
}

std::vector<RecordId> IndexManager::search(
    int64_t key
) const
{
    return tree.search(key);
}

std::vector<IndexEntry> IndexManager::rangeSearch(
    int64_t lower,
    int64_t upper,
    bool includeLower,
    bool includeUpper
) const
{
    return tree.rangeSearch(
        lower,
        upper,
        includeLower,
        includeUpper
    );
}

size_t IndexManager::size() const
{
    return tree.size();
}

size_t IndexManager::distinctSize() const
{
    return tree.distinctSize();
}

bool IndexManager::isEmpty() const
{
    return tree.isEmpty();
}

bool IndexManager::save() const
{
    std::ofstream out(
        filePath,
        std::ios::binary | std::ios::trunc
    );

    if (!out.is_open())
        return false;

    uint32_t magic = INDEX_MAGIC;
    uint32_t version = INDEX_VERSION;

    std::vector<IndexEntry> entries =
        tree.rangeSearch(
            std::numeric_limits<int64_t>::min(),
            std::numeric_limits<int64_t>::max()
        );

    uint64_t count = entries.size();

    out.write(
        reinterpret_cast<const char*>(&magic),
        sizeof(magic)
    );

    out.write(
        reinterpret_cast<const char*>(&version),
        sizeof(version)
    );

    out.write(
        reinterpret_cast<const char*>(&count),
        sizeof(count)
    );

    for (const auto& entry : entries)
    {
        DiskIndexEntry diskEntry{
            entry.key,
            entry.rid.pageNumber,
            entry.rid.slotNumber,
            entry.rid.generation
        };

        out.write(
            reinterpret_cast<const char*>(&diskEntry),
            sizeof(diskEntry)
        );
    }

    out.flush();

    return out.good();
}

bool IndexManager::load()
{
    std::ifstream in(filePath, std::ios::binary);

    if (!in.is_open())
        return false;

    uint32_t magic = 0;
    uint32_t version = 0;
    uint64_t count = 0;

    in.read(
        reinterpret_cast<char*>(&magic),
        sizeof(magic)
    );

    in.read(
        reinterpret_cast<char*>(&version),
        sizeof(version)
    );

    in.read(
        reinterpret_cast<char*>(&count),
        sizeof(count)
    );

    if (
        !in ||
        magic != INDEX_MAGIC ||
        version != INDEX_VERSION
    )
    {
        return false;
    }

    // Reject unreasonable or corrupted counts.
    if (count > 10000000)
        return false;

    BPlusTree loadedTree(maxKeys);

    for (uint64_t i = 0; i < count; i++)
    {
        DiskIndexEntry diskEntry{};

        in.read(
            reinterpret_cast<char*>(&diskEntry),
            sizeof(diskEntry)
        );

        if (!in)
            return false;

        RecordId rid(
            diskEntry.pageNumber,
            diskEntry.slotNumber,
            diskEntry.generation
        );

        if (!rid.isValid())
            return false;

        loadedTree.insert(diskEntry.key, rid);
    }

    tree = std::move(loadedTree);

    return true;
}