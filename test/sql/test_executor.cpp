#include <iostream>

#include "../../src/core/pager/pager.h"
#include "../../src/core/buffer/buffer_pool.h"
#include "../../src/core/allocation/page_allocator.h"
#include "../../src/core/catalog/catalog.h"

#include "../../src/sql/lexer/lexer.h"
#include "../../src/sql/parser/parser.h"
#include "../../src/sql/executor/executor.h"

int main()
{
    Pager pager(
        "data/database.db"
    );

    BufferPool bufferPool(pager);

    PageAllocator pageAllocator(
        pager
    );

    Catalog catalog(
        pageAllocator
    );

    Schema schema;

    schema.addColumn(
        "id",
        DataType::INT
    );

    schema.addColumn(
        "name",
        DataType::VARCHAR
    );

    schema.addColumn(
        "age",
        DataType::INT
    );

    if (!catalog.tableExists("students"))
    {
        catalog.createTable(
            "students",
            schema
        );
    }

    GenericTable table(
        pager,
        bufferPool,
        pageAllocator,
        catalog.getSchema("students"),
        catalog.getFirstPage("students")
    );

    if (table.size() == 0)
    {
        Row row1;

        row1.addValue(Value(1));
        row1.addValue(Value("Aryan"));
        row1.addValue(Value(21));

        table.insert(row1);

        Row row2;

        row2.addValue(Value(2));
        row2.addValue(Value("Rahul"));
        row2.addValue(Value(22));

        table.insert(row2);
    }

    std::string sql =
    "SELECT name, age "
    "FROM students "
    "WHERE age < 22;";

    Lexer lexer(sql);

    std::vector<Token> tokens =
        lexer.tokenize();

    Parser parser(tokens);

    SelectQuery query =
        parser.parse();

    Executor executor(
        pager,
        bufferPool,
        pageAllocator,
        catalog
    );

    executor.execute(query);

    bufferPool.flushAll();

    return 0;
}