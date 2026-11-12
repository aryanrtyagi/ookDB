
#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include <memory>
#include <string>
#include <unordered_map>

#include "database.h"

class DatabaseManager
{
private:
    std::string dataDirectory;

    std::unordered_map<std::string, std::unique_ptr<Database>> databases;

    Database* activeDatabase;
    std::string activeDatabaseName;

    bool isValidDatabaseName(const std::string& name) const;

    std::string getDatabasePath(const std::string& name) const;

    void loadDatabases();

    QueryResult createDatabase(const std::string& name);

    QueryResult useDatabase(const std::string& name);

    QueryResult dropDatabase(const std::string& name);

    QueryResult showDatabases() const;

public:
    explicit DatabaseManager(const std::string& directory);

    QueryResult execute(const std::string& sql);

    std::string getActiveDatabaseName() const;

    void close();

    ~DatabaseManager();

    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;
};

#endif
