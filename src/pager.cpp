#include "pager.h"

#include <cstring>
#include <iostream>
#include <stdexcept>

#if defined(_WIN32)
    #include <direct.h>
    #define MKDIR(path) _mkdir(path)
#else
    #include <sys/stat.h>
    #define MKDIR(path) mkdir(path, 0755)
#endif

// Creates the parent directory of `filename` if it doesn't exist.
// Doesn't need <filesystem>, so it works with older compilers too.
static void ensureParentDirectoryExists(const std::string& filename)
{
    std::size_t lastSlash = filename.find_last_of("/\\");

    if (lastSlash == std::string::npos)
    {
        return; // no parent directory in the path, nothing to do
    }

    std::string dir = filename.substr(0, lastSlash);

    if (dir.empty())
    {
        return;
    }

    // MKDIR fails harmlessly if the directory already exists -
    // we only care about failing to open the file afterward.
    MKDIR(dir.c_str());
}

Pager::Pager(const std::string& filename)
{
    ensureParentDirectoryExists(filename);

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
        throw std::runtime_error(
            "Could not open database file: " + filename
        );
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