#ifndef B_PLUS_TREE_NODE_H
#define B_PLUS_TREE_NODE_H

#include <cstdint>
#include <memory>
#include <vector>

#include "../record/record_id.h"

class BPlusTreeNode
{
public:
    bool isLeaf;

    // Keys are always sorted.
    std::vector<int64_t> keys;

    // Used only by leaf nodes.
    // One key may have multiple Record IDs.
    std::vector<std::vector<RecordId>> values;

    // Used only by internal nodes.
    std::vector<std::unique_ptr<BPlusTreeNode>> children;

    // Linked list between leaf nodes.
    BPlusTreeNode* next;

    explicit BPlusTreeNode(bool leaf);
};

#endif
