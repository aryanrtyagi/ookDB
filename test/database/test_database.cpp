#include <iostream>
#include "../../src/database/database.h"

int main()
{
    try
    {
        Database db("data/database.db");

        QueryResult result = db.execute(
            "SELECT * FROM students;"
        );

        std::cout << "Columns: ";

        for (const auto& column : result.columns)
        {
            std::cout << column << " ";
        }

        std::cout << "\nRows: " << result.rows.size() << '\n';
        std::cout << "Message: " << result.message << '\n';

        db.close();

        std::cout << "Database facade test passed!\n";
    }
    catch (const std::exception& e)
    {
        std::cerr << "Test failed: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
