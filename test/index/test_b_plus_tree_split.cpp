#include "../../src/index/b_plus_tree.h"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <numeric>
#include <random>
#include <vector>

int main()
{
    BPlusTree tree(3);

    const int N = 1000;

    std::vector<int> keys(N);

    std::iota(
        keys.begin(),
        keys.end(),
        1
    );

    // Shuffle insertion order.
    std::mt19937 rng(42);

    std::shuffle(
        keys.begin(),
        keys.end(),
        rng
    );

    // Insert shuffled keys.
    for (int key : keys)
    {
        tree.insert(
            key,
            RecordId(key, 0, 1)
        );
    }

    // Check all keys.
    for (int key = 1; key <= N; key++)
    {
        auto result = tree.search(key);

        assert(result.size() == 1);

        assert(
            result[0] == RecordId(key, 0, 1)
        );
    }

    assert(tree.size() == N);
    assert(tree.distinctSize() == N);

    // A large tree should have grown multiple levels.
    assert(tree.height() >= 2);

    std::cout << "[PASS] 1000 shuffled insertions\n";
    std::cout << "[PASS] All exact lookups\n";
    std::cout << "[PASS] Leaf node splits\n";
    std::cout << "[PASS] Internal node splits\n";

    std::cout << "Tree height: "
              << tree.height()
              << "\n";

    std::cout << "\nB+ Tree split tests passed!\n";

    return 0;
}
