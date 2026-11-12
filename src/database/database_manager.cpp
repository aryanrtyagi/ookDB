
#include "database_manager.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <regex>
#include <stdexcept>

#include "../sql/executor/query_result.h"

namespace fs = std::filesystem;

namespace
{
    std::string trim(const std::string& value)
    {
        const std::string whitespace = " \t\n\r";

        std::size_t start = value.find_first_not_of(whitespace);

        if (start == std::string::npos)
            return "";

        std::size_t end = value.find_last_not_of(whitespace);

        return value.substr(start, end - start + 1);
    }

    std::string toUpper(std::string value)
    {
        std::transform(
            value.begin(),
            value.end(),
            value.begin(),
            [](unsigned char c)
            {
                return static_cast<char>(std::toupper(c));
            }
        );

        return value;
    }

    QueryResult makeResult(
        const std::string& message,
        bool success = true
    )
    {
        QueryResult result;
        result.success = success;
        result.message = message;
        return result;
    }
}

// --------------------------------------------------
// Constructor
// --------------------------------------------------

DatabaseManager::DatabaseManager(const std::string& directory)
    : dataDirectory(directory),
      activeDatabase(nullptr),
      activeDatabaseName("")
{
    fs::create_directories(dataDirectory);

    loadDatabases();
}

// --------------------------------------------------
// Validate database name
// --------------------------------------------------

bool DatabaseManager::isValidDatabaseName(
    const std::string& name
) const
{
    if (name.empty())
        return false;

    if (!std::isalpha(static_cast<unsigned char>(name[0])) &&
        name[0] != '_')
    {
        return false;
    }

    for (unsigned char c : name)
    {
        if (!std::isalnum(c) && c != '_')
            return false;
    }

    return true;
}

// --------------------------------------------------
// Get database file path
// --------------------------------------------------

std::string DatabaseManager::getDatabasePath(
    const std::string& name
) const
{
    return (
        fs::path(dataDirectory) /
        name /
        (name + ".db")
    ).string();
}

// --------------------------------------------------
// Discover existing databases
// --------------------------------------------------

void DatabaseManager::loadDatabases()
{
    if (!fs::exists(dataDirectory))
        return;

    for (const auto& entry : fs::directory_iterator(dataDirectory))
    {
        if (!entry.is_directory())
            continue;

        std::string name = entry.path().filename().string();

        if (!isValidDatabaseName(name))
            continue;

        fs::path databaseFile =
            entry.path() / (name + ".db");

        if (fs::exists(databaseFile))
        {
            // Discover names without opening every database.
            // Actual Database instances are opened when USE is executed.
        }
    }
}

// --------------------------------------------------
// CREATE DATABASE
// --------------------------------------------------

QueryResult DatabaseManager::createDatabase(
    const std::string& name
)
{
    if (!isValidDatabaseName(name))
    {
        return makeResult(
            "Invalid database name.",
            false
        );
    }

    fs::path databaseDirectory =
        fs::path(dataDirectory) / name;

    if (fs::exists(databaseDirectory))
    {
        return makeResult(
            "Database already exists: " + name,
            false
        );
    }

    try
    {
        fs::create_directories(databaseDirectory);

        auto database = std::make_unique<Database>(
            getDatabasePath(name)
        );

        databases[name] = std::move(database);

        return makeResult(
            "Database created successfully: " + name
        );
    }
    catch (const std::exception& e)
    {
        std::error_code error;
        fs::remove_all(databaseDirectory, error);

        return makeResult(
            std::string("Could not create database: ") + e.what(),
            false
        );
    }
}

// --------------------------------------------------
// USE DATABASE
// --------------------------------------------------

QueryResult DatabaseManager::useDatabase(
    const std::string& name
)
{
    if (!isValidDatabaseName(name))
    {
        return makeResult(
            "Invalid database name.",
            false
        );
    }

    fs::path databaseDirectory =
        fs::path(dataDirectory) / name;

    fs::path databaseFile = getDatabasePath(name);

    if (!fs::exists(databaseDirectory) ||
        !fs::exists(databaseFile))
    {
        return makeResult(
            "Database does not exist: " + name,
            false
        );
    }

    try
    {
        auto it = databases.find(name);

        if (it == databases.end())
        {
            auto database = std::make_unique<Database>(
                databaseFile.string()
            );

            it = databases.emplace(
                name,
                std::move(database)
            ).first;
        }

        activeDatabase = it->second.get();
        activeDatabaseName = name;

        return makeResult(
            "Database changed to: " + name
        );
    }
    catch (const std::exception& e)
    {
        return makeResult(
            std::string("Could not open database: ") + e.what(),
            false
        );
    }
}

// --------------------------------------------------
// SHOW DATABASES
// --------------------------------------------------

QueryResult DatabaseManager::showDatabases() const
{
    QueryResult result;

    result.columns.push_back("Database");

    if (fs::exists(dataDirectory))
    {
        for (const auto& entry : fs::directory_iterator(dataDirectory))
        {
            if (!entry.is_directory())
                continue;

            std::string name =
                entry.path().filename().string();

            if (!isValidDatabaseName(name))
                continue;

            fs::path databaseFile =
                entry.path() / (name + ".db");

            if (fs::exists(databaseFile))
            {
                result.rows.push_back({
                    Value(name)
                });
            }
        }
    }

    result.message =
        std::to_string(result.rows.size()) +
        " database(s) found.";

    return result;
}

// --------------------------------------------------
// DROP DATABASE
// --------------------------------------------------

QueryResult DatabaseManager::dropDatabase(
    const std::string& name
)
{
    if (!isValidDatabaseName(name))
    {
        return makeResult(
            "Invalid database name.",
            false
        );
    }

    fs::path databaseDirectory =
        fs::path(dataDirectory) / name;

    if (!fs::exists(databaseDirectory))
    {
        return makeResult(
            "Database does not exist: " + name,
            false
        );
    }

    if (activeDatabaseName == name)
    {
        activeDatabase->close();
        activeDatabase = nullptr;
        activeDatabaseName.clear();
    }

    auto it = databases.find(name);

    if (it != databases.end())
    {
        it->second->close();
        databases.erase(it);
    }

    std::error_code error;

    fs::remove_all(databaseDirectory, error);

    if (error)
    {
        return makeResult(
            "Could not drop database: " + error.message(),
            false
        );
    }

    return makeResult(
        "Database dropped successfully: " + name
    );
}

// --------------------------------------------------
// Execute SQL
// --------------------------------------------------

QueryResult DatabaseManager::execute(
    const std::string& sql
)
{
    std::string statement = trim(sql);

    if (statement.empty())
    {
        return makeResult(
            "SQL query cannot be empty.",
            false
        );
    }

    if (!statement.empty() && statement.back() == ';')
    {
        statement.pop_back();
        statement = trim(statement);
    }

    std::smatch match;

    // CREATE DATABASE name
    static const std::regex createPattern(
        R"(^CREATE\s+DATABASE\s+([A-Za-z_][A-Za-z0-9_]*)$)",
        std::regex_constants::icase
    );

    if (std::regex_match(statement, match, createPattern))
    {
        return createDatabase(match[1].str());
    }

    // USE name
    static const std::regex usePattern(
        R"(^USE\s+([A-Za-z_][A-Za-z0-9_]*)$)",
        std::regex_constants::icase
    );

    if (std::regex_match(statement, match, usePattern))
    {
        return useDatabase(match[1].str());
    }

    // SHOW DATABASES
    static const std::regex showPattern(
        R"(^SHOW\s+DATABASES$)",
        std::regex_constants::icase
    );

    if (std::regex_match(statement, showPattern))
    {
        return showDatabases();
    }

    // DROP DATABASE name
    static const std::regex dropPattern(
        R"(^DROP\s+DATABASE\s+([A-Za-z_][A-Za-z0-9_]*)$)",
        std::regex_constants::icase
    );

    if (std::regex_match(statement, match, dropPattern))
    {
        return dropDatabase(match[1].str());
    }

    // All other SQL goes to the active database.
    if (activeDatabase == nullptr)
    {
        return makeResult(
            "No database selected. Execute USE database_name first.",
            false
        );
    }

    try
    {
        return activeDatabase->execute(sql);
    }
    catch (const std::exception& e)
    {
        return makeResult(e.what(), false);
    }
}

// --------------------------------------------------
// Get active database
// --------------------------------------------------

std::string DatabaseManager::getActiveDatabaseName() const
{
    return activeDatabaseName;
}

// --------------------------------------------------
// Close all databases
// --------------------------------------------------

void DatabaseManager::close()
{
    for (auto& entry : databases)
    {
        if (entry.second)
            entry.second->close();
    }

    databases.clear();

    activeDatabase = nullptr;
    activeDatabaseName.clear();
}

// --------------------------------------------------
// Destructor
// --------------------------------------------------

DatabaseManager::~DatabaseManager()
{
    try
    {
        close();
    }
    catch (...)
    {
        // Do not throw exceptions from destructor.
    }
}
