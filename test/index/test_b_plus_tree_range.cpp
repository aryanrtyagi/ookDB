#include "../../src/index/b_plus_tree.h"

#include <cassert>
#include <iostream>

int main()
{
    BPlusTree tree(3);

    for (int i = 1; i <= 100; i++)
    {
        tree.insert(
            i,
            RecordId(i, 0, 1)
        );
    }

    // Add duplicate keys.
    tree.insert(25, RecordId(200, 0, 1));
    tree.insert(50, RecordId(201, 0, 1));

    // --------------------------------------------------------
    // Inclusive range: 20 to 30
    // --------------------------------------------------------

    auto inclusive = tree.rangeSearch(
        20,
        30,
        true,
        true
    );

    assert(inclusive.size() == 12);

    // --------------------------------------------------------
    // Exclusive range: 20 to 30
    // --------------------------------------------------------

    auto exclusive = tree.rangeSearch(
        20,
        30,
        false,
        false
    );

    assert(exclusive.size() == 10);

    // --------------------------------------------------------
    // Lower inclusive, upper exclusive
    // --------------------------------------------------------

    auto mixed = tree.rangeSearch(
        20,
        30,
        true,
        false
    );

    assert(mixed.size() == 11);

    // --------------------------------------------------------
    // Entire range
    // --------------------------------------------------------

    auto all = tree.rangeSearch(
        1,
        100,
        true,
        true
    );

    assert(all.size() == 102);

    // --------------------------------------------------------
    // Invalid range
    // --------------------------------------------------------

    auto empty = tree.rangeSearch(
        100,
        10
    );

    assert(empty.empty());

    // Check sorted order.
    for (size_t i = 1; i < all.size(); i++)
    {
        assert(all[i - 1].key <= all[i].key);
    }

    std::cout << "[PASS] Inclusive range\n";
    std::cout << "[PASS] Exclusive range\n";
    std::cout << "[PASS] Mixed range boundaries\n";
    std::cout << "[PASS] Duplicate keys in ranges\n";
    std::cout << "[PASS] Sorted range results\n";
    std::cout << "[PASS] Invalid range\n";

    std::cout << "\nB+ Tree range tests passed!\n";

    return 0;
}
