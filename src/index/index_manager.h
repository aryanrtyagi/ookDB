#ifndef INDEX_MANAGER_H
#define INDEX_MANAGER_H

#include "b_plus_tree.h"

#include <cstdint>
#include <string>
#include <vector>

class IndexManager
{
private:
    BPlusTree tree;
    std::string filePath;
    size_t maxKeys;

    void rebuildWithout(
        int64_t key,
        const RecordId& rid
    );

public:
    explicit IndexManager(
        const std::string& filePath,
        size_t maxKeys = 4
    );

    bool insert(int64_t key, const RecordId& rid);

    bool erase(int64_t key, const RecordId& rid);

    std::vector<RecordId> search(int64_t key) const;

    std::vector<IndexEntry> rangeSearch(
        int64_t lower,
        int64_t upper,
        bool includeLower = true,
        bool includeUpper = true
    ) const;

    size_t size() const;

    size_t distinctSize() const;

    bool isEmpty() const;

    bool save() const;

    bool load();
};

#endif