#ifndef TABLE_H
#define TABLE_H

#include "pager.h"
#include "buffer_pool.h"

#include <string>

struct User
{
    int id;
    char name[32];
    int age;
    char city[32];
    int salary;
    bool deleted;
};

constexpr int ROW_SIZE = sizeof(User);

struct DatabaseHeader
{
    int magic;
    int version;
    long long numRows;
    int rowSize; // must match sizeof(User) for THIS build
};

constexpr int DATABASE_MAGIC = 0x41525944;
constexpr int DATABASE_VERSION = 1;

class Table
{
private:
    Pager pager;
    BufferPool bufferPool;

    int rowsPerPage;
    long long numRows;

    void loadMetadata();
    void saveMetadata();

public:
    explicit Table(const std::string& filename);
    ~Table();

    void insert(
        int id,
        const char* name,
        int age,
        const char* city,
        int salary
    );

    void selectAll();

    void selectWhere(int id);

    void update(
        int id,
        const char* name,
        int age,
        const char* city,
        int salary
    );

    void remove(int id);
};

#endif