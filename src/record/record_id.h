#ifndef RECORD_ID_H
#define RECORD_ID_H

#include <cstdint>

struct RecordId
{
    int pageNumber;
    uint32_t slotNumber;
    uint32_t generation;

    RecordId()
        : pageNumber(-1),
          slotNumber(0),
          generation(0)
    {
    }

    RecordId(
        int page,
        uint32_t slot,
        uint32_t gen
    )
        : pageNumber(page),
          slotNumber(slot),
          generation(gen)
    {
    }

    bool isValid() const
    {
        return pageNumber >= 0;
    }

    bool operator==(const RecordId& other) const
    {
        return pageNumber == other.pageNumber &&
               slotNumber == other.slotNumber &&
               generation == other.generation;
    }

    bool operator!=(const RecordId& other) const
    {
        return !(*this == other);
    }
};

#endif
