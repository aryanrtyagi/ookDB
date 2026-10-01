#include "catalog_storage.h"
#include "catalog_serializer.h"

#include <cstring>
#include <stdexcept>


CatalogStorage::CatalogStorage(
    BufferPool& bufferPool
)
    : bufferPool(bufferPool)
{
}


// ============================================================
// SAVE CATALOG
// ============================================================

void CatalogStorage::save(
    const Catalog& catalog
)
{
    std::vector<uint8_t> serialized =
        CatalogSerializer::serialize(catalog);


    // We need a little header inside Page 0.
    //
    // Bytes 0-3:
    //     magic number
    //
    // Bytes 4-7:
    //     serialized catalog size
    //
    // Bytes 8+:
    //     catalog data


    if (serialized.size() + 8 > PAGE_SIZE)
    {
        throw std::runtime_error(
            "Catalog is too large for one page"
        );
    }


    char* page =
        bufferPool.getPage(CATALOG_PAGE);


    if (page == nullptr)
    {
        throw std::runtime_error(
            "Could not load catalog page"
        );
    }


    // Clear the page first.
    std::memset(
        page,
        0,
        PAGE_SIZE
    );


    // Magic number: "ARYD"
    const int magic = 0x41525944;


    std::memcpy(
        page,
        &magic,
        sizeof(int)
    );


    int size =
        static_cast<int>(
            serialized.size()
        );


    std::memcpy(
        page + 4,
        &size,
        sizeof(int)
    );


    std::memcpy(
        page + 8,
        serialized.data(),
        serialized.size()
    );


    bufferPool.markDirty(
        CATALOG_PAGE
    );


    bufferPool.flushPage(
        CATALOG_PAGE
    );
}


// ============================================================
// LOAD CATALOG
// ============================================================

void CatalogStorage::load(
    Catalog& catalog
)
{
    char* page =
        bufferPool.getPage(CATALOG_PAGE);


    if (page == nullptr)
    {
        throw std::runtime_error(
            "Could not load catalog page"
        );
    }


    int magic = 0;


    std::memcpy(
        &magic,
        page,
        sizeof(int)
    );


    // Page 0 has never been initialized.
    if (magic != 0x41525944)
    {
        catalog.clear();

        return;
    }


    int size = 0;


    std::memcpy(
        &size,
        page + 4,
        sizeof(int)
    );


    if (
        size < 0 ||
        size > PAGE_SIZE - 8
    )
    {
        throw std::runtime_error(
            "Invalid catalog size"
        );
    }


    std::vector<uint8_t> serialized(
        size
    );


    std::memcpy(
        serialized.data(),
        page + 8,
        size
    );


    CatalogSerializer::deserialize(
        serialized,
        catalog
    );
}