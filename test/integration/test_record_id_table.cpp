#include "../../src/core/table/generic_table.h"

#include <cassert>
#include <cstdio>
#include <iostream>
#include <stdexcept>

int main()
{
    std::remove("data/rid_test.db");

    Pager pager("data/rid_test.db");
    BufferPool bufferPool(pager);
    PageAllocator allocator(pager);

    Schema schema;
    schema.addColumn("id", DataType::INT);
    schema.addColumn("name", DataType::VARCHAR);

    int firstPage = allocator.allocatePage();

    GenericTable table(
        pager,
        bufferPool,
        allocator,
        schema,
        firstPage
    );

    // --------------------------------------------------------
    // 1. Insert
    // --------------------------------------------------------

    Row row1;
    row1.addValue(Value(101));
    row1.addValue(Value("Aryan"));

    RecordId rid1 = table.insertWithId(row1);

    assert(rid1.isValid());

    std::cout << "[PASS] Record inserted with stable RID\n";

    // --------------------------------------------------------
    // 2. Retrieve
    // --------------------------------------------------------

    Row fetched = table.get(rid1);

    assert(fetched.getValue(0).asInt() == 101);
    assert(fetched.getValue(1).asString() == "Aryan");

    std::cout << "[PASS] Retrieved record using RID\n";

    // --------------------------------------------------------
    // 3. Update
    // --------------------------------------------------------

    Row updated;
    updated.addValue(Value(102));
    updated.addValue(Value("Updated Aryan"));

    assert(table.updateRow(rid1, updated));

    Row afterUpdate = table.get(rid1);

    assert(afterUpdate.getValue(0).asInt() == 102);
    assert(
        afterUpdate.getValue(1).asString()
        == "Updated Aryan"
    );

    std::cout << "[PASS] Updated record using RID\n";

    // --------------------------------------------------------
    // 4. Delete
    // --------------------------------------------------------

    assert(table.deleteRow(rid1));

    bool staleRejected = false;

    try
    {
        table.get(rid1);
    }
    catch (const std::out_of_range&)
    {
        staleRejected = true;
    }

    assert(staleRejected);

    std::cout << "[PASS] Deleted RID rejected\n";

    // --------------------------------------------------------
    // 5. Reuse slot
    // --------------------------------------------------------

    Row row2;
    row2.addValue(Value(200));
    row2.addValue(Value("New Record"));

    RecordId rid2 = table.insertWithId(row2);

    assert(rid2.isValid());

    assert(rid1.slotNumber == rid2.slotNumber);
    assert(rid1.generation != rid2.generation);

    assert(table.get(rid2).getValue(0).asInt() == 200);

    staleRejected = false;

    try
    {
        table.get(rid1);
    }
    catch (const std::out_of_range&)
    {
        staleRejected = true;
    }

    assert(staleRejected);

    std::cout << "[PASS] Slot reused with new generation\n";
    std::cout << "[PASS] Old RID cannot access new record\n";

    // --------------------------------------------------------
    // 6. Existing logical API
    // --------------------------------------------------------

    assert(table.size() == 1);

    std::cout << "[PASS] Existing table API still works\n";

    bufferPool.flushAll();

    std::cout << "\nAll RecordId table tests passed!\n";

    return 0;
}
