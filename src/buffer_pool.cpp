#include "buffer_pool.h"

#include <cstring>

BufferPool::BufferPool(Pager& pager)
    : pager(pager),
      usageCounter(0)
{
    for (auto& frame : frames)
    {
        frame.pageNumber = -1;
        frame.dirty = false;
        frame.occupied = false;
        frame.lastUsed = 0;

        std::memset(frame.data, 0, PAGE_SIZE);
    }
}

int BufferPool::findLRUFrame() const
{
    int lruFrame = -1;

    for (int i = 0; i < BUFFER_POOL_SIZE; ++i)
    {
        if (!frames[i].occupied)
            continue;

        if (
            lruFrame == -1 ||
            frames[i].lastUsed <
            frames[lruFrame].lastUsed)
        {
            lruFrame = i;
        }
    }

    return lruFrame;
}

char* BufferPool::getPage(int pageNumber)
{
    for (int i = 0; i < BUFFER_POOL_SIZE; ++i)
    {
        if (
            frames[i].occupied &&
            frames[i].pageNumber == pageNumber)
        {
            ++usageCounter;

            frames[i].lastUsed =
                usageCounter;

            return frames[i].data;
        }
    }

    int frameIndex = -1;

    for (int i = 0; i < BUFFER_POOL_SIZE; ++i)
    {
        if (!frames[i].occupied)
        {
            frameIndex = i;
            break;
        }
    }

    if (frameIndex == -1)
    {
        frameIndex = findLRUFrame();

        if (frameIndex == -1)
            return nullptr;

        Frame& victim =
            frames[frameIndex];

        if (victim.dirty)
        {
            pager.writePage(
                victim.pageNumber,
                victim.data
            );

            victim.dirty = false;
        }
    }

    pager.readPage(
        pageNumber,
        frames[frameIndex].data
    );

    Frame& frame =
        frames[frameIndex];

    frame.pageNumber =
        pageNumber;

    frame.occupied = true;
    frame.dirty = false;

    ++usageCounter;

    frame.lastUsed =
        usageCounter;

    return frame.data;
}

void BufferPool::markDirty(int pageNumber)
{
    for (auto& frame : frames)
    {
        if (
            frame.occupied &&
            frame.pageNumber == pageNumber)
        {
            frame.dirty = true;
            return;
        }
    }
}

void BufferPool::flushPage(int pageNumber)
{
    for (auto& frame : frames)
    {
        if (
            frame.occupied &&
            frame.pageNumber == pageNumber)
        {
            if (frame.dirty)
            {
                pager.writePage(
                    frame.pageNumber,
                    frame.data
                );

                frame.dirty = false;
            }

            return;
        }
    }
}

void BufferPool::flushAll()
{
    for (auto& frame : frames)
    {
        if (
            frame.occupied &&
            frame.dirty)
        {
            pager.writePage(
                frame.pageNumber,
                frame.data
            );

            frame.dirty = false;
        }
    }
}