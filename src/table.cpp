#include "table.h"

#include <cstring>
#include <iostream>

Table::Table(const std::string &filename)
    : pager(filename),
      bufferPool(pager)
{
    rowsPerPage =
        PAGE_SIZE / ROW_SIZE;

    loadMetadata();
}

Table::~Table()
{
    saveMetadata();
    bufferPool.flushAll();
}

void Table::loadMetadata()
{
    long long fileSize = pager.getFileSize();

    if (fileSize < PAGE_SIZE)
    {
        numRows = 0;
        saveMetadata();
        return;
    }

    char page[PAGE_SIZE];

    pager.readPage(0, page);

    DatabaseHeader header{};

    std::memcpy(
        &header,
        page,
        sizeof(DatabaseHeader)
    );

    if (
        header.magic != DATABASE_MAGIC ||
        header.version != DATABASE_VERSION)
    {
        std::cerr
            << "Error: incompatible database format.\n"
            << "Existing database was not modified.\n";

        numRows = 0;
        return;
    }

    numRows = header.numRows;
}

void Table::saveMetadata()
{
    char page[PAGE_SIZE];

    std::memset(
        page,
        0,
        PAGE_SIZE);

    DatabaseHeader header{};

    header.magic =
        DATABASE_MAGIC;

    header.version =
        DATABASE_VERSION;

    header.numRows =
        numRows;

    std::memcpy(
        page,
        &header,
        sizeof(DatabaseHeader));

    pager.writePage(
        0,
        page);
}

void Table::insert(
    int id,
    const char *name,
    int age,
    const char *city,
    int salary)
{
    User user{};

    user.id = id;

    std::strncpy(
        user.name,
        name,
        sizeof(user.name) - 1);

    user.age = age;

    std::strncpy(
        user.city,
        city,
        sizeof(user.city) - 1);

    user.salary = salary;

    user.deleted = false;

    long long rowNumber =
        numRows;

    int pageNumber =
        1 +
        static_cast<int>(
            rowNumber / rowsPerPage);

    int rowOffset =
        static_cast<int>(
            rowNumber % rowsPerPage);

    char *page =
        bufferPool.getPage(
            pageNumber);

    if (page == nullptr)
    {
        std::cerr
            << "Error: could not load page.\n";

        return;
    }

    std::memcpy(
        page +
            rowOffset * ROW_SIZE,

        &user,

        ROW_SIZE);

    bufferPool.markDirty(
        pageNumber);

    ++numRows;

    std::cout
        << "Record inserted successfully.\n";
}

void Table::selectAll()
{
    if (numRows == 0)
    {
        std::cout
            << "Database is empty.\n";

        return;
    }

    std::cout << "\n";
    std::cout
        << "ID | Name | Age | City | Salary\n";

    std::cout
        << "---------------------------------------------\n";

    int currentPage = -1;

    char *page = nullptr;

    for (
        long long rowNumber = 0;
        rowNumber < numRows;
        ++rowNumber)
    {
        int pageNumber =
            1 +
            static_cast<int>(
                rowNumber / rowsPerPage);

        int rowOffset =
            static_cast<int>(
                rowNumber % rowsPerPage);

        if (pageNumber != currentPage)
        {
            page =
                bufferPool.getPage(
                    pageNumber);

            if (page == nullptr)
            {
                std::cerr
                    << "Error: could not load page.\n";

                return;
            }

            currentPage =
                pageNumber;
        }

        User user{};

        std::memcpy(
            &user,

            page +
                rowOffset * ROW_SIZE,

            ROW_SIZE);
        if (user.deleted){
            continue;
        }

        std::cout
            << user.id
            << " | "
            << user.name
            << " | "
            << user.age
            << " | "
            << user.city
            << " | "
            << user.salary
            << '\n';
    }

    std::cout << '\n';
}

void Table::selectWhere(int id)
{
    bool found = false;

    int currentPage = -1;
    char* page = nullptr;

    for (
        long long rowNumber = 0;
        rowNumber < numRows;
        ++rowNumber)
    {
        int pageNumber =
            1 +
            static_cast<int>(
                rowNumber / rowsPerPage
            );

        int rowOffset =
            static_cast<int>(
                rowNumber % rowsPerPage
            );

        if (pageNumber != currentPage)
        {
            page =
                bufferPool.getPage(pageNumber);

            if (page == nullptr)
                return;

            currentPage = pageNumber;
        }

        User user{};

        std::memcpy(
            &user,
            page + rowOffset * ROW_SIZE,
            ROW_SIZE
        );

        if (user.deleted)
            continue;

        if (user.id == id)
        {
            std::cout
                << "\nID | Name | Age | City | Salary\n";

            std::cout
                << user.id
                << " | "
                << user.name
                << " | "
                << user.age
                << " | "
                << user.city
                << " | "
                << user.salary
                << '\n';

            found = true;
            break;
        }
    }

    if (!found)
    {
        std::cout
            << "Record not found.\n";
    }
}

void Table::update(
    int id,
    const char* name,
    int age,
    const char* city,
    int salary)
{
    int currentPage = -1;
    char* page = nullptr;

    for (
        long long rowNumber = 0;
        rowNumber < numRows;
        ++rowNumber)
    {
        int pageNumber =
            1 +
            static_cast<int>(
                rowNumber / rowsPerPage
            );

        int rowOffset =
            static_cast<int>(
                rowNumber % rowsPerPage
            );

        if (pageNumber != currentPage)
        {
            page =
                bufferPool.getPage(pageNumber);

            if (page == nullptr)
                return;

            currentPage = pageNumber;
        }

        User user{};

        std::memcpy(
            &user,
            page + rowOffset * ROW_SIZE,
            ROW_SIZE
        );

        if (user.deleted)
            continue;

        if (user.id == id)
        {
            user.id = id;

            std::strncpy(
                user.name,
                name,
                sizeof(user.name) - 1
            );

            user.age = age;

            std::strncpy(
                user.city,
                city,
                sizeof(user.city) - 1
            );

            user.salary = salary;

            std::memcpy(
                page + rowOffset * ROW_SIZE,
                &user,
                ROW_SIZE
            );

            bufferPool.markDirty(
                pageNumber
            );

            std::cout
                << "Record updated successfully.\n";

            return;
        }
    }

    std::cout
        << "Record not found.\n";
}

void Table::remove(int id)
{
    int currentPage = -1;
    char* page = nullptr;

    for (
        long long rowNumber = 0;
        rowNumber < numRows;
        ++rowNumber)
    {
        int pageNumber =
            1 +
            static_cast<int>(
                rowNumber / rowsPerPage
            );

        int rowOffset =
            static_cast<int>(
                rowNumber % rowsPerPage
            );

        if (pageNumber != currentPage)
        {
            page =
                bufferPool.getPage(pageNumber);

            if (page == nullptr)
                return;

            currentPage = pageNumber;
        }

        User user{};

        std::memcpy(
            &user,
            page + rowOffset * ROW_SIZE,
            ROW_SIZE
        );

        if (user.deleted)
            continue;

        if (user.id == id)
        {
            user.deleted = true;

            std::memcpy(
                page + rowOffset * ROW_SIZE,
                &user,
                ROW_SIZE
            );

            bufferPool.markDirty(
                pageNumber
            );

            std::cout
                << "Record deleted successfully.\n";

            return;
        }
    }

    std::cout
        << "Record not found.\n";
}