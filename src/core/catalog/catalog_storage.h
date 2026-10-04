#ifndef CATALOG_STORAGE_H
#define CATALOG_STORAGE_H

#include "catalog.h"
#include "buffer_pool.h"

class CatalogStorage
{
private:
    BufferPool& bufferPool;

    static constexpr int CATALOG_PAGE = 0;

public:

    explicit CatalogStorage(
        BufferPool& bufferPool
    );

    void save(
        const Catalog& catalog
    );

    void load(
        Catalog& catalog
    );
};

#endif