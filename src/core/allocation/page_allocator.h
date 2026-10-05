#ifndef PAGE_ALLOCATOR_H
#define PAGE_ALLOCATOR_H

#include "../pager/pager.h"

class PageAllocator{
private:
    Pager& pager;
    int nextPageNumber;

public:
    explicit PageAllocator( Pager& pager );
    int getNextPageNumber() const;
    int allocatePage();
};

#endif