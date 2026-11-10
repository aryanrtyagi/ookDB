#include <cassert>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <cstdlib>

#include "../../src/core/pager/pager.h"
#include "../../src/core/buffer/buffer_pool.h"
#include "../../src/core/allocation/page_allocator.h"
#include "../../src/core/catalog/catalog.h"
#include "../../src/core/catalog/catalog_storage.h"
#include "../../src/sql/executor/executor.h"
#include "../../src/sql/lexer/lexer.h"
#include "../../src/sql/parser/parser.h"

static void executeSQL(Executor &executor, const std::string &sql)
{
    Lexer lexer(sql);
    Parser parser(lexer.tokenize());
    executor.execute(parser.parse());
}

static std::string captureSQL(Executor &executor, const std::string &sql)
{
    std::ostringstream output;
    std::streambuf *old = std::cout.rdbuf(output.rdbuf());

    try
    {
        executeSQL(executor, sql);
    }
    catch (...)
    {
        std::cout.rdbuf(old);
        throw;
    }

    std::cout.rdbuf(old);
    return output.str();
}

int main()
{
    const std::string dbPath = "data/test_index_executor.db";
    const std::string indexPath = "data/students_id_idx.idx";

    #ifdef _WIN32
        system("if not exist data mkdir data");
    #else
        system("mkdir -p data");
    #endif

    std::remove(dbPath.c_str());
    std::remove(indexPath.c_str());

    // Phase 1: create table, insert rows, build index,
    // and verify indexed SELECT.
    {
        Pager pager(dbPath);
        BufferPool bufferPool(pager);
        PageAllocator allocator(pager);
        Catalog catalog(allocator);
        CatalogStorage catalogStorage(bufferPool);

        catalogStorage.load(catalog);

        Executor executor(
            pager,
            bufferPool,
            allocator,
            catalog,
            catalogStorage);

        executeSQL(
            executor,
            "CREATE TABLE students (id INT, name VARCHAR);");

        executeSQL(
            executor,
            "INSERT INTO students VALUES (10, 'Aryan');");

        executeSQL(
            executor,
            "INSERT INTO students VALUES (10, 'Aman');");

        executeSQL(
            executor,
            "INSERT INTO students VALUES (20, 'Riya');");

        executeSQL(
            executor,
            "CREATE INDEX students_id_idx ON students(id);");

        assert(catalog.hasIndex("students", "id"));

        std::cout
            << "[PASS] CREATE INDEX builds index over existing records\n";

        std::string indexed = captureSQL(
            executor,
            "SELECT * FROM students WHERE id = 10;");

        assert(indexed.find("[B+ Tree index]") != std::string::npos);
        assert(indexed.find("Aryan") != std::string::npos);
        assert(indexed.find("Aman") != std::string::npos);
        assert(indexed.find("2 row(s) selected") != std::string::npos);

        std::cout
            << "[PASS] Indexed equality SELECT supports duplicate keys\n";

        std::string missing = captureSQL(
            executor,
            "SELECT * FROM students WHERE id = 999;");

        assert(missing.find("[B+ Tree index]") != std::string::npos);
        assert(missing.find("0 row(s) selected") != std::string::npos);

        std::cout
            << "[PASS] Indexed SELECT handles missing key\n";

        std::string fallback = captureSQL(
            executor,
            "SELECT * FROM students WHERE name = 'Riya';");

        assert(fallback.find("[Full table scan]") != std::string::npos);
        assert(fallback.find("Riya") != std::string::npos);

        std::cout
            << "[PASS] Unindexed column falls back to table scan\n";

        executeSQL(
            executor,
            "INSERT INTO students VALUES (30, 'Neha');");

        std::string inserted = captureSQL(
            executor,
            "SELECT * FROM students WHERE id = 30;");

        assert(inserted.find("Neha") != std::string::npos);
        assert(inserted.find("[B+ Tree index]") != std::string::npos);

        std::cout
            << "[PASS] INSERT maintains index\n";

        executeSQL(
            executor,
            "UPDATE students SET id = 40 WHERE name = 'Neha';");

        std::string oldKey = captureSQL(
            executor,
            "SELECT * FROM students WHERE id = 30;");

        std::string newKey = captureSQL(
            executor,
            "SELECT * FROM students WHERE id = 40;");

        assert(oldKey.find("0 row(s) selected") != std::string::npos);
        assert(newKey.find("Neha") != std::string::npos);

        std::cout
            << "[PASS] UPDATE moves index entry to new key\n";

        executeSQL(
            executor,
            "DELETE FROM students WHERE name = 'Aman';");

        std::string deleted = captureSQL(
            executor,
            "SELECT * FROM students WHERE id = 10;");

        assert(deleted.find("Aryan") != std::string::npos);
        assert(deleted.find("Aman") == std::string::npos);
        assert(deleted.find("1 row(s) selected") != std::string::npos);

        std::cout
            << "[PASS] DELETE removes index entry\n";

        bufferPool.flushAll();
    }

    // Phase 2: reopen database; catalog metadata and B+ Tree file
    // must reload.
    {
        Pager pager(dbPath);
        BufferPool bufferPool(pager);
        PageAllocator allocator(pager);
        Catalog catalog(allocator);
        CatalogStorage catalogStorage(bufferPool);

        catalogStorage.load(catalog);

        assert(catalog.hasIndex("students", "id"));

        Executor executor(
            pager,
            bufferPool,
            allocator,
            catalog,
            catalogStorage);

        std::string afterRestart = captureSQL(
            executor,
            "SELECT * FROM students WHERE id = 40;");

        assert(
            afterRestart.find("[B+ Tree index]") != std::string::npos);

        assert(
            afterRestart.find("Neha") != std::string::npos);

        std::cout
            << "[PASS] Index metadata and entries survive restart\n";

        bufferPool.flushAll();
    }

    std::cout << "\nALL SQL INDEX TESTS PASSED\n";

    return 0;
}