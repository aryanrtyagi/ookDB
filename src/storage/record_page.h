#ifndef RECORD_PAGE_H
#define RECORD_PAGE_H

#include "../pager.h"
#include "../record/row.h"

#include <cstdint>

class RecordPage
{
private:
    char* pageData;

    static constexpr int HEADER_SIZE = sizeof(int) * 2;
    static constexpr int RECORD_SIZE_HEADER = sizeof(int);

    int getRecordOffset(int index) const;

public:

    explicit RecordPage(char* page);

    bool hasSpace(const Row& row) const;

    bool insert(const Row& row);

    Row get(int index) const;

    int size() const;

    int usedSpace() const;

    int freeSpace() const;

    int getNextPage() const;

    void setNextPage(int pageNumber);

    void clear();
};

#endif