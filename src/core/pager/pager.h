#ifndef PAGER_H
#define PAGER_H

#include <fstream>
#include <string>

constexpr int PAGE_SIZE = 4096;

class Pager
{
private:
    std::fstream file;

public:
    explicit Pager(const std::string& filename);
    ~Pager();

    void readPage(int pageNumber, char* destination);
    void writePage(int pageNumber, const char* source);

    long long getFileSize() const;
};

#endif