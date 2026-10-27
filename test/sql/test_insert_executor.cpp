<<<<<<< HEAD

=======
>>>>>>> 4b0779494586eeddfd737f5f686076f8c696dc22
#include <iostream>
#include <cstdio>
#include <stdexcept>
#include <string>
<<<<<<< HEAD
=======
#include <vector>
>>>>>>> 4b0779494586eeddfd737f5f686076f8c696dc22

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

<<<<<<< HEAD
        // STEP 1: Create a table.
=======
>>>>>>> 4b0779494586eeddfd737f5f686076f8c696dc22
        std::string createSQL =
            "CREATE TABLE employees "
            "(id INT, name VARCHAR, salary DOUBLE);";

        Lexer createLexer(createSQL);
        Parser createParser(createLexer.tokenize());
        Query createQuery = createParser.parse();

        executor.execute(createQuery);

<<<<<<< HEAD
        // STEP 2: Insert a row.
        std::string insertSQL =
            "INSERT INTO employees VALUES (1, 'Aryan', 75000);";
=======
        std::string insertSQL =
            "INSERT INTO employees "
            "VALUES (1, 'Aryan', 75000);";
>>>>>>> 4b0779494586eeddfd737f5f686076f8c696dc22

        Lexer insertLexer(insertSQL);
        Parser insertParser(insertLexer.tokenize());
        Query insertQuery = insertParser.parse();

<<<<<<< HEAD
        std::cout << "Parsed query type: "
                  << static_cast<int>(insertQuery.type)
                  << '\n';

        if (insertQuery.type != QueryType::INSERT)
        {
            throw std::runtime_error(
                "Parser did not produce an INSERT query"
            );
        }

        executor.execute(insertQuery);

        // STEP 3: Open the table and verify the stored row.
=======
        executor.execute(insertQuery);

>>>>>>> 4b0779494586eeddfd737f5f686076f8c696dc22
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
<<<<<<< HEAD
            throw std::runtime_error(
                "Inserted row does not match expected values"
            );
=======
            throw std::runtime_error("Inserted row does not match");
>>>>>>> 4b0779494586eeddfd737f5f686076f8c696dc22
        }

        std::cout << "INSERT EXECUTOR TEST PASSED\n";
    }
    catch (const std::exception& e)
    {
<<<<<<< HEAD
        std::cerr << "TEST FAILED: " << e.what() << '\n';
=======
        std::cerr << "TEST FAILED: " << e.what() << "\n";
>>>>>>> 4b0779494586eeddfd737f5f686076f8c696dc22
        return 1;
    }

    return 0;
}
