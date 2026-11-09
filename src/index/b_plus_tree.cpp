#include "b_plus_tree.h"

#include <algorithm>
#include <stdexcept>
#include <iterator>

// ============================================================
// Constructor
// ============================================================

BPlusTree::BPlusTree(size_t maxKeys)
    : root(nullptr),
      maxKeys(maxKeys),
      recordCount(0),
      distinctKeyCount(0)
{
    if (maxKeys < 3)
    {
        throw std::invalid_argument(
            "B+ Tree maxKeys must be at least 3"
        );
    }
}

// ============================================================
// Find leaf node
// ============================================================

BPlusTreeNode* BPlusTree::findLeaf(int64_t key)
{
    if (!root)
        return nullptr;

    BPlusTreeNode* current = root.get();

    while (!current->isLeaf)
    {
        size_t childIndex =
            std::upper_bound(
                current->keys.begin(),
                current->keys.end(),
                key
            ) - current->keys.begin();

        current = current->children[childIndex].get();
    }

    return current;
}


const BPlusTreeNode* BPlusTree::findLeaf(
    int64_t key
) const
{
    if (!root)
        return nullptr;

    const BPlusTreeNode* current = root.get();

    while (!current->isLeaf)
    {
        size_t childIndex =
            std::upper_bound(
                current->keys.begin(),
                current->keys.end(),
                key
            ) - current->keys.begin();

        current = current->children[childIndex].get();
    }

    return current;
}


// ============================================================
// Split leaf node
// ============================================================

void BPlusTree::splitLeaf(
    BPlusTreeNode* node,
    SplitResult& result
)
{
    auto right =
        std::make_unique<BPlusTreeNode>(true);

    size_t middle = node->keys.size() / 2;

    // Move the upper half of the keys.
    right->keys.assign(
        node->keys.begin() + middle,
        node->keys.end()
    );

    // Move their corresponding Record IDs.
    right->values.assign(
        std::make_move_iterator(
            node->values.begin() + middle
        ),
        std::make_move_iterator(
            node->values.end()
        )
    );

    node->keys.erase(
        node->keys.begin() + middle,
        node->keys.end()
    );

    node->values.erase(
        node->values.begin() + middle,
        node->values.end()
    );

    // Maintain linked leaf nodes.
    right->next = node->next;
    node->next = right.get();

    // The first key of the right leaf
    // becomes the separator in the parent.
    result.didSplit = true;
    result.separator = right->keys.front();
    result.right = std::move(right);
}


// ============================================================
// Split internal node
// ============================================================

void BPlusTree::splitInternal(
    BPlusTreeNode* node,
    SplitResult& result
)
{
    auto right =
        std::make_unique<BPlusTreeNode>(false);

    size_t middle = node->keys.size() / 2;

    // Promote the middle key to the parent.
    int64_t promotedKey = node->keys[middle];

    // Keys after the promoted key move right.
    right->keys.assign(
        node->keys.begin() + middle + 1,
        node->keys.end()
    );

    // Move corresponding child pointers.
    for (
        size_t i = middle + 1;
        i < node->children.size();
        i++
    )
    {
        right->children.push_back(
            std::move(node->children[i])
        );
    }

    // Keep keys before the promoted key.
    node->keys.erase(
        node->keys.begin() + middle,
        node->keys.end()
    );

    // The left node retains middle + 1 children.
    node->children.resize(middle + 1);

    result.didSplit = true;
    result.separator = promotedKey;
    result.right = std::move(right);
}


// ============================================================
// Recursive insertion
// ============================================================

BPlusTree::SplitResult BPlusTree::insertRecursive(
    BPlusTreeNode* node,
    int64_t key,
    const RecordId& rid
)
{
    SplitResult result;

    // --------------------------------------------------------
    // Leaf node
    // --------------------------------------------------------

    if (node->isLeaf)
    {
        auto it = std::lower_bound(
            node->keys.begin(),
            node->keys.end(),
            key
        );

        size_t position =
            it - node->keys.begin();

        // Duplicate key: append another RecordId.
        if (
            it != node->keys.end() &&
            *it == key
        )
        {
            node->values[position].push_back(rid);
            return result;
        }

        // Insert new key and its RecordId.
        node->keys.insert(
            it,
            key
        );

        node->values.insert(
            node->values.begin() + position,
            std::vector<RecordId>{rid}
        );

        // Overflow: split the leaf.
        if (node->keys.size() > maxKeys)
        {
            splitLeaf(node, result);
        }

        return result;
    }

    // --------------------------------------------------------
    // Internal node
    // --------------------------------------------------------

    size_t childIndex =
        std::upper_bound(
            node->keys.begin(),
            node->keys.end(),
            key
        ) - node->keys.begin();

    SplitResult childResult =
        insertRecursive(
            node->children[childIndex].get(),
            key,
            rid
        );

    // No child split: nothing to propagate.
    if (!childResult.didSplit)
        return result;

    // Insert separator key into this internal node.
    node->keys.insert(
        node->keys.begin() + childIndex,
        childResult.separator
    );

    // Insert the new right child.
    node->children.insert(
        node->children.begin() + childIndex + 1,
        std::move(childResult.right)
    );

    // This internal node may now overflow.
    if (node->keys.size() > maxKeys)
    {
        splitInternal(node, result);
    }

    return result;
}


// ============================================================
// Public insert
// ============================================================

void BPlusTree::insert(
    int64_t key,
    const RecordId& rid
)
{
    if (!rid.isValid())
    {
        throw std::invalid_argument(
            "Cannot index an invalid RecordId"
        );
    }

    bool keyAlreadyExists =
        !search(key).empty();

    // First insertion.
    if (!root)
    {
        root =
            std::make_unique<BPlusTreeNode>(true);

        root->keys.push_back(key);

        root->values.push_back(
            std::vector<RecordId>{rid}
        );

        recordCount++;
        distinctKeyCount++;

        return;
    }

    SplitResult result =
        insertRecursive(
            root.get(),
            key,
            rid
        );

    recordCount++;

    if (!keyAlreadyExists)
        distinctKeyCount++;

    // Root split: create a new root.
    if (result.didSplit)
    {
        auto newRoot =
            std::make_unique<BPlusTreeNode>(false);

        newRoot->keys.push_back(
            result.separator
        );

        newRoot->children.push_back(
            std::move(root)
        );

        newRoot->children.push_back(
            std::move(result.right)
        );

        root = std::move(newRoot);
    }
}

bool BPlusTree::erase(int64_t key, const RecordId& rid)
{
    BPlusTreeNode* leaf = findLeaf(key);

    if (leaf == nullptr)
        return false;

    auto it = std::lower_bound(
        leaf->keys.begin(),
        leaf->keys.end(),
        key
    );

    if (it == leaf->keys.end() || *it != key)
        return false;

    size_t position = it - leaf->keys.begin();

    auto& ids = leaf->values[position];

    auto ridIt = std::find(
        ids.begin(),
        ids.end(),
        rid
    );

    if (ridIt == ids.end())
        return false;

    ids.erase(ridIt);

    if (recordCount > 0)
        recordCount--;

    if (ids.empty() && distinctKeyCount > 0)
        distinctKeyCount--;
    
    // Retain the key to preserve internal separator structure.
    return true;
}

// ============================================================
// Exact search
// ============================================================

std::vector<RecordId> BPlusTree::search(
    int64_t key
) const
{
    const BPlusTreeNode* leaf = findLeaf(key);

    if (!leaf)
        return {};

    auto it = std::lower_bound(
        leaf->keys.begin(),
        leaf->keys.end(),
        key
    );

    if (
        it == leaf->keys.end() ||
        *it != key
    )
    {
        return {};
    }

    size_t position =
        it - leaf->keys.begin();

    return leaf->values[position];
}


// ============================================================
// Range search
// ============================================================

std::vector<IndexEntry> BPlusTree::rangeSearch(
    int64_t lower,
    int64_t upper,
    bool includeLower,
    bool includeUpper
) const
{
    std::vector<IndexEntry> result;

    if (!root || lower > upper)
        return result;

    const BPlusTreeNode* leaf = findLeaf(lower);

    while (leaf != nullptr)
    {
        for (size_t i = 0; i < leaf->keys.size(); i++)
        {
            int64_t key = leaf->keys[i];

            // Lower boundary.
            if (
                key < lower ||
                (!includeLower && key == lower)
            )
            {
                continue;
            }

            // Upper boundary.
            if (
                key > upper ||
                (!includeUpper && key == upper)
            )
            {
                return result;
            }

            for (const RecordId& rid : leaf->values[i])
            {
                result.emplace_back(key, rid);
            }
        }

        leaf = leaf->next;
    }

    return result;
}


// ============================================================
// Metadata
// ============================================================

bool BPlusTree::isEmpty() const
{
    return recordCount == 0;
}


size_t BPlusTree::size() const
{
    return recordCount;
}


size_t BPlusTree::distinctSize() const
{
    return distinctKeyCount;
}


size_t BPlusTree::height() const
{
    if (!root)
        return 0;

    size_t result = 1;

    const BPlusTreeNode* current = root.get();

    while (!current->isLeaf)
    {
        current = current->children.front().get();
        result++;
    }

    return result;
}
