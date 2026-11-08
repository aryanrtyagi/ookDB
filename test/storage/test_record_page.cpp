#include "../../src/storage/record_page.h"
#include "../../src/record/value.h"

#include <iostream>
#include <cassert>

Row makeRow(int id, const char* name)
{
    Row row;
    row.addValue(Value(id));
    row.addValue(Value(name));
    return row;
}

int main()
{
    char rawPage[PAGE_SIZE] = {};

    RecordPage page(rawPage);

    uint32_t slot1, slot2, slot3;
    uint32_t gen1, gen2, gen3;

    Row aman = makeRow(1, "Aman");
    Row rahul = makeRow(2, "Rahul");
    Row priya = makeRow(3, "Priya");

    // Test insertion
    assert(page.insert(aman, slot1, gen1));
    assert(page.insert(rahul, slot2, gen2));
    assert(page.insert(priya, slot3, gen3));

    assert(page.size() == 3);

    std::cout << "[PASS] Three records inserted\n";

    // Test stable slot retrieval
    Row result = page.getBySlot(slot2, gen2);

    assert(result.getValue(1).asString() == "Rahul");

    std::cout << "[PASS] RID-based retrieval\n";

    // Delete Rahul
    assert(page.eraseBySlot(slot2, gen2));

    assert(page.size() == 2);
    assert(!page.isValidSlot(slot2, gen2));

    std::cout << "[PASS] Deletion and stale RID detection\n";

    // Insert another record
    Row rohan = makeRow(4, "Rohan");

    uint32_t newSlot, newGeneration;

    assert(page.insert(
        rohan,
        newSlot,
        newGeneration
    ));

    // Deleted slot should be reused.
    assert(newSlot == slot2);

    // But its generation must change.
    assert(newGeneration > gen2);

    assert(!page.isValidSlot(slot2, gen2));
    assert(page.isValidSlot(newSlot, newGeneration));

    std::cout << "[PASS] Slot reuse and generation tracking\n";

    // Existing records must retain their identities.
    assert(page.getBySlot(slot1, gen1)
               .getValue(1).asString() == "Aman");

    assert(page.getBySlot(slot3, gen3)
               .getValue(1).asString() == "Priya");

    std::cout << "[PASS] Existing RIDs remain valid\n";

    // Test update without changing RID.
    Row updated = makeRow(4, "Rohan Updated");

    assert(page.updateBySlot(
        newSlot,
        newGeneration,
        updated
    ));

    assert(page.getBySlot(newSlot, newGeneration)
               .getValue(1).asString() == "Rohan Updated");

    std::cout << "[PASS] Update preserves RID\n";

    std::cout << "\nAll Slotted Page tests passed!\n";

    return 0;
}
