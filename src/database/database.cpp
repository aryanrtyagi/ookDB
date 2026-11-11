#include "database.h"

#include <stdexcept>
#include <utility>

#include "../sql/lexer/lexer.h"
#include "../sql/parser/parser.h"

Database::Database(const std::string& databasePath)
    : pager(databasePath),
      bufferPool(pager),
      pageAllocator(pager),
      catalog(pageAllocator),
      catalogStorage(bufferPool),
      executor(
          pager,
          bufferPool,
          pageAllocator,
          catalog,
          catalogStorage
      ),
      closed(false)
{
    // Restore table and index metadata from disk.
    catalogStorage.load(catalog);
}

QueryResult Database::execute(const std::string& sql)
{
    if (closed)
    {
        throw std::runtime_error(
            "Cannot execute query: database is closed."
        );
    }

    if (sql.empty())
    {
        throw std::invalid_argument(
            "SQL query cannot be empty."
        );
    }

    Lexer lexer(sql);
    std::vector<Token> tokens = lexer.tokenize();

    Parser parser(tokens);
    Query query = parser.parse();

    return executor.execute(query);
}

void Database::close()
{
    if (closed)
        return;

    bufferPool.flushAll();
    closed = true;
}

Database::~Database()
{
    try
    {
        close();
    }
    catch (...)
    {
        // Destructors must not propagate exceptions.
    }
}
