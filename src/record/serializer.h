#ifndef SERIALIZER_H
#define SERIALIZER_H

#include "row.h"

#include <vector>
#include <cstdint>

class Serializer
{
public:

    static std::vector<uint8_t> serialize(const Row& row);

    static Row deserialize(
        const std::vector<uint8_t>& data
    );
};

#endif