#ifndef GENERIC_TABLE_H
#define GENERIC_TABLE_H

#include "../pager/pager.h"
#include "../buffer/buffer_pool.h"
#include "../allocation/page_allocator.h"

#include "../../record/row.h"
#include "../../record/schema.h"
#include "../../record/record_id.h"
#include "../../storage/record_page.h"

#include <vector>

class GenericTable
{
private:
    Pager& pager;
    BufferPool& bufferPool;
    PageAllocator& pageAllocator;

    Schema schema;
    int firstPage;

    // Check whether a page belongs to this table.
    bool containsPage(int targetPage);

public:

    GenericTable(
        Pager& pager,
        BufferPool& bufferPool,
        PageAllocator& pageAllocator,
        const Schema& schema,
        int firstPage
    );

    // Existing logical-index API.
    bool insert(const Row& row);

    Row get(int index);

    bool updateRow(int index, const Row& row);

    bool deleteRow(int index);

    int size();

    const Schema& getSchema() const;

    // Stable RecordId API.
    RecordId insertWithId(const Row& row);

    Row get(const RecordId& rid);

    bool updateRow(
        const RecordId& rid,
        const Row& row
    );

    bool deleteRow(const RecordId& rid);

    // Enumerate all active records using stable identifiers.
    std::vector<RecordId> getAllRecordIds();
};

#endif
