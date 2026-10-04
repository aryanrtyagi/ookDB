#ifndef BUFFER_POOL_H
#define BUFFER_POOL_H

#include "pager.h"

#include <array>

constexpr int BUFFER_POOL_SIZE = 3;

struct Frame
{
    int pageNumber;
    bool dirty;
    bool occupied;
    long long lastUsed;

    char data[PAGE_SIZE];
};

class BufferPool
{
private:
    Pager& pager;

    std::array<Frame, BUFFER_POOL_SIZE> frames;

    long long usageCounter;

    int findLRUFrame() const;

public:
    explicit BufferPool(Pager& pager);

    char* getPage(int pageNumber);

    void markDirty(int pageNumber);

    void flushPage(int pageNumber);

    void flushAll();
};

#endif