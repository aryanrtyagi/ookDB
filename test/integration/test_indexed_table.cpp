
#include "../../src/core/pager/pager.h"
#include "../../src/core/buffer/buffer_pool.h"
#include "../../src/core/allocation/page_allocator.h"
#include "../../src/core/table/generic_table.h"
#include "../../src/index/index_manager.h"
#include "../../src/index/indexed_table.h"

#include <cstdio>
#include <iostream>
#include <string>
#include <stdexcept>

static Row makeRow(int id, const char* name)
{
    Row row;
    row.addValue(Value(id));
    row.addValue(Value(name));
    return row;
}

static void require(bool condition, const char* message)
{
    if (!condition)
        throw std::runtime_error(message);

    std::cout << "[PASS] " << message << "\n";
}

int main()
{
    const char* dbPath = "data/indexed_table_test.db";
    const char* indexPath = "data/students_id.idx";

    std::remove(dbPath);
    std::remove(indexPath);

    RecordId firstRid;
    RecordId duplicateRid;

    // =====================================================
    // PHASE 1: INSERT, SEARCH, UPDATE AND DELETE
    // =====================================================

    {
        Pager pager(dbPath);
        BufferPool pool(pager);
        PageAllocator allocator(pager);

        Schema schema;
        schema.addColumn("id", DataType::INT);
        schema.addColumn("name", DataType::VARCHAR);

        int firstPage = allocator.allocatePage();

        GenericTable table(
            pager,
            pool,
            allocator,
            schema,
            firstPage
        );

        IndexManager index(indexPath);

        IndexedTable indexed(
            table,
            index,
            0
        );

        // Insert three records.
        firstRid = indexed.insert(
            makeRow(10, "Aryan")
        );

        duplicateRid = indexed.insert(
            makeRow(10, "Aman")
        );

        RecordId thirdRid = indexed.insert(
            makeRow(20, "Riya")
        );

        require(
            firstRid.isValid() &&
            duplicateRid.isValid() &&
            thirdRid.isValid(),
            "Insert records and receive valid RecordIds"
        );

        require(
            indexed.indexSize() == 3,
            "Index contains three records"
        );

        require(
            indexed.distinctKeyCount() == 2,
            "Index contains two distinct keys"
        );

        // Exact search with duplicate keys.
        require(
            indexed.search(10).size() == 2,
            "Exact search returns both duplicate-key records"
        );

        require(
            indexed.search(20).size() == 1,
            "Exact search returns the correct record"
        );

        require(
            indexed.search(99).empty(),
            "Search for missing key returns empty result"
        );

        // Range search.
        require(
            indexed.rangeSearch(10, 20).size() == 3,
            "Inclusive range search returns three records"
        );

        require(
            indexed.rangeSearch(10, 20, false, true).size() == 1,
            "Exclusive lower-bound range search works"
        );

        // Update the indexed key from 10 to 15.
        require(
            indexed.update(
                firstRid,
                makeRow(15, "Aryan")
            ),
            "Update indexed record"
        );

        require(
            indexed.search(10).size() == 1 &&
            indexed.search(15).size() == 1,
            "Index follows indexed-column update"
        );

        // Delete the remaining record with key 10.
        require(
            indexed.erase(duplicateRid),
            "Delete indexed record"
        );

        require(
            indexed.search(10).empty(),
            "Deleted key is no longer searchable"
        );

        require(
            indexed.search(15).size() == 1,
            "Updated record remains searchable"
        );

        require(
            indexed.indexSize() == 2,
            "Index count is correct after deletion"
        );

        require(
            indexed.distinctKeyCount() == 2,
            "Distinct-key count is correct after deletion"
        );

        // Flush database pages before closing.
        pool.flushAll();
    }

    // =====================================================
    // PHASE 2: REOPEN AND TEST PERSISTENCE
    // =====================================================

    {
        Pager pager(dbPath);
        BufferPool pool(pager);
        PageAllocator allocator(pager);

        Schema schema;
        schema.addColumn("id", DataType::INT);
        schema.addColumn("name", DataType::VARCHAR);

        // Reopen the same table page.
        GenericTable table(
            pager,
            pool,
            allocator,
            schema,
            1
        );

        // IndexManager loads the saved index automatically.
        IndexManager index(indexPath);

        IndexedTable indexed(
            table,
            index,
            0
        );

        require(
            indexed.search(15).size() == 1,
            "Updated record survives reopen"
        );

        require(
            indexed.search(10).empty(),
            "Deleted key stays absent after reopen"
        );

        require(
            indexed.search(20).size() == 1,
            "Other indexed record survives reopen"
        );

        require(
            indexed.indexSize() == 2,
            "Index record count survives reopen"
        );

        require(
            indexed.distinctKeyCount() == 2,
            "Distinct-key count survives reopen"
        );

        pool.flushAll();
    }

    std::cout << "\n====================================\n";
    std::cout << "ALL INDEXED TABLE TESTS COMPLETED\n";
    std::cout << "====================================\n";

    return 0;
}
