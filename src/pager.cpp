#include "pager.h"

#include <cstring>
#include <iostream>

Pager::Pager(const std::string& filename)
{
    file.open(
        filename,
        std::ios::in |
        std::ios::out |
        std::ios::binary
    );

    if (!file.is_open())
    {
        file.clear();

        file.open(
            filename,
            std::ios::out |
            std::ios::binary
        );

        file.close();

        file.open(
            filename,
            std::ios::in |
            std::ios::out |
            std::ios::binary
        );
    }

    if (!file.is_open())
    {
        std::cerr << "Error: could not open database file.\n";
    }
}

Pager::~Pager()
{
    if (file.is_open())
    {
        file.flush();
        file.close();
    }
}

void Pager::readPage(
    int pageNumber,
    char* destination)
{
    long long offset =
        static_cast<long long>(pageNumber) * PAGE_SIZE;

    file.clear();

    file.seekg(offset, std::ios::beg);

    file.read(destination, PAGE_SIZE);

    std::streamsize bytesRead = file.gcount();

    if (bytesRead < PAGE_SIZE)
    {
        std::memset(
            destination + bytesRead,
            0,
            PAGE_SIZE - bytesRead
        );

        file.clear();
    }
}

void Pager::writePage(
    int pageNumber,
    const char* source)
{
    long long offset =
        static_cast<long long>(pageNumber) * PAGE_SIZE;

    file.clear();

    file.seekp(offset, std::ios::beg);

    file.write(source, PAGE_SIZE);

    file.flush();
}

long long Pager::getFileSize() const
{
    std::fstream& stream =
        const_cast<std::fstream&>(file);

    stream.clear();

    stream.seekg(0, std::ios::end);

    long long size =
        static_cast<long long>(stream.tellg());

    stream.seekg(0, std::ios::beg);

    return size;
}