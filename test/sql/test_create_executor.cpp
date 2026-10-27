#include <iostream>

#include "../../src/sql/lexer/lexer.h"
#include "../../src/sql/parser/parser.h"
#include "../../src/sql/executor/executor.h"

#include "../../src/core/pager/pager.h"
#include "../../src/core/buffer/buffer_pool.h"
#include "../../src/core/allocation/page_allocator.h"
#include "../../src/core/catalog/catalog.h"

int main()
{
    Pager pager("data/database.db");

    BufferPool bufferPool(pager);

    PageAllocator pageAllocator(pager);

    Catalog catalog(pageAllocator);

    std::string input =
        "CREATE TABLE employees "
        "(id INT, name VARCHAR, salary DOUBLE);";

    Lexer lexer(input);

    std::vector<Token> tokens =
        lexer.tokenize();

    Parser parser(tokens);

    Query query =
        parser.parse();

    Executor executor(
        pager,
        bufferPool,
        pageAllocator,
        catalog
    );

    executor.execute(query);

    if (catalog.tableExists("employees"))
    {
        std::cout
            << "CREATE TABLE TEST PASSED\n";
    }
    else
    {
        std::cout
            << "CREATE TABLE TEST FAILED\n";
    }

    return 0;
}