#ifndef B_PLUS_TREE_H
#define B_PLUS_TREE_H

#include "b_plus_tree_node.h"
#include "index_entry.h"

#include <cstdint>
#include <memory>
#include <vector>

class BPlusTree
{
private:
    struct SplitResult
    {
        bool didSplit;
        int64_t separator;
        std::unique_ptr<BPlusTreeNode> right;

        SplitResult()
            : didSplit(false),
              separator(0),
              right(nullptr)
        {
        }
    };

    std::unique_ptr<BPlusTreeNode> root;

    // Maximum number of keys allowed in a node
    size_t maxKeys;

    // Number of indexed records, including duplicates
    size_t recordCount;

    // Number of distinct indexed keys
    size_t distinctKeyCount;

    SplitResult insertRecursive(
        BPlusTreeNode* node,
        int64_t key,
        const RecordId& rid
    );

    BPlusTreeNode* findLeaf(int64_t key);

    const BPlusTreeNode* findLeaf(int64_t key) const;

    void splitLeaf(
        BPlusTreeNode* node,
        SplitResult& result
    );

    void splitInternal(
        BPlusTreeNode* node,
        SplitResult& result
    );

public:
    explicit BPlusTree(size_t maxKeys = 4);

    // Insert key -> RecordId
    void insert(
        int64_t key,
        const RecordId& rid
    );
    
    bool erase(int64_t key, const RecordId& rid);
    // Return all records matching an exact key
    std::vector<RecordId> search(int64_t key) const;

    // Inclusive/exclusive range search
    std::vector<IndexEntry> rangeSearch(
        int64_t lower,
        int64_t upper,
        bool includeLower = true,
        bool includeUpper = true
    ) const;

    // Metadata
    bool isEmpty() const;

    size_t size() const;

    size_t distinctSize() const;

    size_t height() const;
};

#endif
