#include <iostream>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <vector>

#include "../../src/sql/lexer/lexer.h"
#include "../../src/sql/parser/parser.h"
#include "../../src/sql/executor/executor.h"

int main()
{
    const std::string dbPath = "data/insert_test.db";
    std::remove(dbPath.c_str());

    try
    {
        Pager pager(dbPath);
        BufferPool bufferPool(pager);
        PageAllocator pageAllocator(pager);
        Catalog catalog(pageAllocator);

        Executor executor(
            pager,
            bufferPool,
            pageAllocator,
            catalog
        );

        std::string createSQL =
            "CREATE TABLE employees "
            "(id INT, name VARCHAR, salary DOUBLE);";

        Lexer createLexer(createSQL);
        Parser createParser(createLexer.tokenize());
        Query createQuery = createParser.parse();

        executor.execute(createQuery);

        std::string insertSQL =
            "INSERT INTO employees "
            "VALUES (1, 'Aryan', 75000);";

        Lexer insertLexer(insertSQL);
        Parser insertParser(insertLexer.tokenize());
        Query insertQuery = insertParser.parse();

        executor.execute(insertQuery);

        Schema schema = catalog.getSchema("employees");
        int firstPage = catalog.getFirstPage("employees");

        GenericTable table(
            pager,
            bufferPool,
            pageAllocator,
            schema,
            firstPage
        );

        if (table.size() != 1)
            throw std::runtime_error("Expected exactly one row");

        Row row = table.get(0);

        if (row.getValue(0).asInt() != 1 ||
            row.getValue(1).asString() != "Aryan" ||
            row.getValue(2).asDouble() != 75000.0)
        {
            throw std::runtime_error("Inserted row does not match");
        }

        std::cout << "INSERT EXECUTOR TEST PASSED\n";
    }
    catch (const std::exception& e)
    {
        std::cerr << "TEST FAILED: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
