#include "../../src/index/b_plus_tree.h"

#include <cassert>
#include <iostream>

int main()
{
    BPlusTree tree(4);

    // Insert records with keys 10, 20, 30.
    tree.insert(10, RecordId(1, 0, 1));
    tree.insert(20, RecordId(1, 1, 1));
    tree.insert(30, RecordId(1, 2, 1));

    // Insert a duplicate key.
    tree.insert(20, RecordId(1, 3, 1));

    // Exact search.
    auto result = tree.search(20);

    assert(result.size() == 2);

    assert(result[0] == RecordId(1, 1, 1));
    assert(result[1] == RecordId(1, 3, 1));

    // Search missing key.
    assert(tree.search(99).empty());

    // Metadata.
    assert(tree.size() == 4);
    assert(tree.distinctSize() == 3);

    std::cout << "[PASS] Exact search\n";
    std::cout << "[PASS] Duplicate keys\n";
    std::cout << "[PASS] Missing key search\n";
    std::cout << "[PASS] Tree metadata\n";

    std::cout << "\nB+ Tree basic tests passed!\n";

    return 0;
}
