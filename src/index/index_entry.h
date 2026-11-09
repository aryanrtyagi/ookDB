#ifndef INDEX_ENTRY_H
#define INDEX_ENTRY_H

#include "../record/record_id.h"

#include <cstdint>

struct IndexEntry
{
    int64_t key;
    RecordId rid;

    IndexEntry()
        : key(0), rid()
    {
    }

    IndexEntry(int64_t key, const RecordId& rid)
        : key(key), rid(rid)
    {
    }
};

#endif
