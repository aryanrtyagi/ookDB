#ifndef PAGE_ALLOCATOR_H
#define PAGE_ALLOCATOR_H

#include "pager.h"

class PageAllocator
{
private:
    Pager& pager;

public:

    explicit PageAllocator(
        Pager& pager
    );

    int allocatePage();

    int getNextPageNumber() const;
};

#endif