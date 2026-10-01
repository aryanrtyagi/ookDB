#ifndef CATALOG_SERIALIZER_H
#define CATALOG_SERIALIZER_H

#include "catalog.h"

#include <vector>
#include <cstdint>

class CatalogSerializer
{
public:

    static std::vector<uint8_t>
    serialize(const Catalog& catalog);

    static void
    deserialize(
        const std::vector<uint8_t>& data,
        Catalog& catalog
    );
};

#endif