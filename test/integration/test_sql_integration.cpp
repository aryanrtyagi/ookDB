
#include <cassert>
#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <string>

#include "../../src/core/pager/pager.h"
#include "../../src/core/buffer/buffer_pool.h"
#include "../../src/core/allocation/page_allocator.h"
#include "../../src/core/catalog/catalog.h"
#include "../../src/core/catalog/catalog_storage.h"
#include "../../src/core/table/generic_table.h"

#include "../../src/record/row.h"
#include "../../src/record/schema.h"

#include "../../src/sql/lexer/lexer.h"
#include "../../src/sql/parser/parser.h"
#include "../../src/sql/ast/query.h"
#include "../../src/sql/executor/executor.h"

using namespace std;

void executeSQL(Executor& executor, const string& sql) {
    Lexer lexer(sql);
    auto tokens = lexer.tokenize();

    Parser parser(tokens);
    Query query = parser.parse();

    executor.execute(query);
}

void testCRUDAndPersistence() {
    const string dbPath = "data/arydb_integration_test.db";
    std::remove(dbPath.c_str());

    {
        Pager pager(dbPath);
        BufferPool bufferPool(pager);
        PageAllocator pageAllocator(pager);
        Catalog catalog(pageAllocator);
        CatalogStorage catalogStorage(bufferPool);

        catalogStorage.load(catalog);

        Executor executor(
            pager,
            bufferPool,
            pageAllocator,
            catalog,
            catalogStorage
        );

        // 1. CREATE TABLE
        executeSQL(
            executor,
            "CREATE TABLE integration_test "
            "(id INT, name VARCHAR, age INT);"
        );

        assert(catalog.tableExists("integration_test"));
        cout << "[PASS] CREATE TABLE\n";

        // 2. INSERT
        executeSQL(
            executor,
            "INSERT INTO integration_test VALUES (1, 'Aman', 20);"
        );

        executeSQL(
            executor,
            "INSERT INTO integration_test VALUES (2, 'Riya', 21);"
        );

        GenericTable table(
            pager,
            bufferPool,
            pageAllocator,
            catalog.getSchema("integration_test"),
            catalog.getFirstPage("integration_test")
        );

        assert(table.size() == 2);
        cout << "[PASS] INSERT\n";

        // 3. Single-row UPDATE
        executeSQL(
            executor,
            "UPDATE integration_test SET age = 22 WHERE id = 1;"
        );

        assert(table.get(0).getValue(2).asInt() == 22);
        cout << "[PASS] Single-row UPDATE\n";

        // 4. Single-row DELETE
        executeSQL(
            executor,
            "DELETE FROM integration_test WHERE id = 2;"
        );

        assert(table.size() == 1);
        cout << "[PASS] Single-row DELETE\n";

        // 5. Invalid SELECT column
        bool invalidSelectRejected = false;

        try {
            executeSQL(
                executor,
                "SELECT salary FROM integration_test;"
            );
        } catch (const std::exception&) {
            invalidSelectRejected = true;
        }

        assert(invalidSelectRejected);
        assert(table.size() == 1);
        cout << "[PASS] Invalid SELECT column\n";

        // 6. Duplicate table creation
        bool duplicateRejected = false;

        try {
            executeSQL(
                executor,
                "CREATE TABLE integration_test "
                "(id INT, name VARCHAR, age INT);"
            );
        } catch (const std::exception&) {
            duplicateRejected = true;
        }

        assert(duplicateRejected);
        assert(catalog.tableExists("integration_test"));
        assert(table.size() == 1);
        cout << "[PASS] Duplicate table creation\n";

        // 7. Invalid value type
        bool wrongTypeRejected = false;

        try {
            executeSQL(
                executor,
                "INSERT INTO integration_test "
                "VALUES (3, 'Sam', 'not-a-number');"
            );
        } catch (const std::exception&) {
            wrongTypeRejected = true;
        }

        assert(wrongTypeRejected);
        assert(table.size() == 1);
        cout << "[PASS] Invalid value type\n";

        // 8. Multi-row UPDATE
        executeSQL(
            executor,
            "CREATE TABLE regression_test (id INT, age INT);"
        );

        executeSQL(
            executor,
            "INSERT INTO regression_test VALUES (1, 30);"
        );

        executeSQL(
            executor,
            "INSERT INTO regression_test VALUES (2, 30);"
        );

        executeSQL(
            executor,
            "INSERT INTO regression_test VALUES (3, 40);"
        );

        GenericTable regressionTable(
            pager,
            bufferPool,
            pageAllocator,
            catalog.getSchema("regression_test"),
            catalog.getFirstPage("regression_test")
        );

        executeSQL(
            executor,
            "UPDATE regression_test SET age = 31 WHERE age = 30;"
        );

        assert(regressionTable.size() == 3);
        assert(regressionTable.get(0).getValue(1).asInt() == 31);
        assert(regressionTable.get(1).getValue(1).asInt() == 31);
        assert(regressionTable.get(2).getValue(1).asInt() == 40);
        cout << "[PASS] Multi-row UPDATE\n";

        // 9. DELETE all rows
        executeSQL(executor, "DELETE FROM regression_test;");

        assert(regressionTable.size() == 0);
        cout << "[PASS] DELETE all rows\n";

        // 10. Malformed SQL
        bool malformedRejected = false;

        try {
            executeSQL(
                executor,
                "SELEC * FROM integration_test;"
            );
        } catch (const std::exception&) {
            malformedRejected = true;
        }

        assert(malformedRejected);

        // A valid query should still execute after the error.
        executeSQL(
            executor,
            "SELECT * FROM integration_test;"
        );

        cout << "[PASS] Malformed SQL handling\n";

        // 11. Comparison operators
        executeSQL(
            executor,
            "CREATE TABLE comparison_test "
            "(id INT, score INT, matched INT);"
        );

        executeSQL(
            executor,
            "INSERT INTO comparison_test VALUES (1, 10, 0);"
        );

        executeSQL(
            executor,
            "INSERT INTO comparison_test VALUES (2, 20, 0);"
        );

        executeSQL(
            executor,
            "INSERT INTO comparison_test VALUES (3, 30, 0);"
        );

        GenericTable comparisonTable(
            pager,
            bufferPool,
            pageAllocator,
            catalog.getSchema("comparison_test"),
            catalog.getFirstPage("comparison_test")
        );

        auto checkOperator = [&](const string& condition,
                                 bool expected0,
                                 bool expected1,
                                 bool expected2) {
            executeSQL(
                executor,
                "UPDATE comparison_test SET matched = 0;"
            );

            executeSQL(
                executor,
                "UPDATE comparison_test SET matched = 1 WHERE " +
                condition + ";"
            );

            assert(
                comparisonTable.get(0).getValue(2).asInt()
                == (expected0 ? 1 : 0)
            );

            assert(
                comparisonTable.get(1).getValue(2).asInt()
                == (expected1 ? 1 : 0)
            );

            assert(
                comparisonTable.get(2).getValue(2).asInt()
                == (expected2 ? 1 : 0)
            );
        };

        checkOperator("score = 20",  false, true,  false);
        checkOperator("score != 20", true,  false, true);
        checkOperator("score > 20",  false, false, true);
        checkOperator("score < 20",  true,  false, false);
        checkOperator("score >= 20", false, true,  true);
        checkOperator("score <= 20", true,  true,  false);

        cout << "[PASS] All six comparison operators\n";

        // 12. Multi-page data test
        executeSQL(
            executor,
            "CREATE TABLE bulk_test (id INT, label VARCHAR);"
        );

        for (int i = 0; i < 300; ++i) {
            executeSQL(
                executor,
                "INSERT INTO bulk_test VALUES (" +
                to_string(i) + ", 'row');"
            );
        }

        GenericTable bulkTable(
            pager,
            bufferPool,
            pageAllocator,
            catalog.getSchema("bulk_test"),
            catalog.getFirstPage("bulk_test")
        );

        assert(bulkTable.size() == 300);
        cout << "[PASS] 300-row insertion\n";

        // Flush dirty record pages before closing.
        bufferPool.flushAll();
    }

    // 13. Reopen and verify persistence
    {
        Pager pager(dbPath);
        BufferPool bufferPool(pager);
        PageAllocator pageAllocator(pager);
        Catalog catalog(pageAllocator);
        CatalogStorage catalogStorage(bufferPool);

        catalogStorage.load(catalog);

        // integration_test persists
        assert(catalog.tableExists("integration_test"));

        GenericTable table(
            pager,
            bufferPool,
            pageAllocator,
            catalog.getSchema("integration_test"),
            catalog.getFirstPage("integration_test")
        );

        assert(table.size() == 1);

        Row persisted = table.get(0);

        assert(persisted.getValue(0).asInt() == 1);
        assert(persisted.getValue(1).asString() == "Aman");
        assert(persisted.getValue(2).asInt() == 22);

        cout << "[PASS] Catalog persistence\n";
        cout << "[PASS] Record persistence\n";

        // regression_test was emptied before closing.
        assert(catalog.tableExists("regression_test"));

        GenericTable regressionTable(
            pager,
            bufferPool,
            pageAllocator,
            catalog.getSchema("regression_test"),
            catalog.getFirstPage("regression_test")
        );

        assert(regressionTable.size() == 0);
        cout << "[PASS] Deleted rows remain deleted\n";

        // Comparison results from score <= 20.
        assert(catalog.tableExists("comparison_test"));

        GenericTable comparisonTable(
            pager,
            bufferPool,
            pageAllocator,
            catalog.getSchema("comparison_test"),
            catalog.getFirstPage("comparison_test")
        );

        assert(comparisonTable.size() == 3);
        assert(comparisonTable.get(0).getValue(2).asInt() == 1);
        assert(comparisonTable.get(1).getValue(2).asInt() == 1);
        assert(comparisonTable.get(2).getValue(2).asInt() == 0);

        cout << "[PASS] Comparison results after restart\n";

        // Bulk table persists across reopening.
        assert(catalog.tableExists("bulk_test"));

        GenericTable bulkTable(
            pager,
            bufferPool,
            pageAllocator,
            catalog.getSchema("bulk_test"),
            catalog.getFirstPage("bulk_test")
        );

        assert(bulkTable.size() == 300);
        assert(bulkTable.get(0).getValue(0).asInt() == 0);
        assert(bulkTable.get(299).getValue(0).asInt() == 299);

        cout << "[PASS] 300-row persistence after restart\n";
    }

    // Delete the dedicated test database after successful checks.
    std::remove(dbPath.c_str());

    cout << "\nAll AryDB integration tests passed!\n";
}

int main() {
    testCRUDAndPersistence();
    return 0;
}
