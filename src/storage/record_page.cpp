#include "record_page.h"

#include "../record/serializer.h"

#include <cstring>
#include <stdexcept>
#include <vector>


RecordPage::RecordPage(char* page)
    : pageData(page)
{
}


// ============================================================
// Find the byte offset of a particular record
// ============================================================

int RecordPage::getRecordOffset(int index) const
{
    if (pageData == nullptr)
    {
        throw std::runtime_error(
            "RecordPage has no page"
        );
    }

    if (index < 0 || index >= size())
    {
        throw std::out_of_range(
            "Invalid record index"
        );
    }

    int offset = HEADER_SIZE;

    for (int i = 0; i < index; i++)
    {
        int recordSize;

        std::memcpy(
            &recordSize,
            pageData + offset,
            sizeof(int)
        );

        offset += RECORD_SIZE_HEADER;
        offset += recordSize;
    }

    return offset;
}


// ============================================================
// Check whether a Row can fit in the page
// ============================================================

bool RecordPage::hasSpace(const Row& row) const
{
    if (pageData == nullptr)
    {
        return false;
    }

    std::vector<uint8_t> bytes =
        Serializer::serialize(row);

    int requiredSpace =
        RECORD_SIZE_HEADER +
        static_cast<int>(bytes.size());

    return usedSpace() + requiredSpace <= PAGE_SIZE;
}


// ============================================================
// Insert Row
// ============================================================

bool RecordPage::insert(const Row& row)
{
    if (!hasSpace(row))
    {
        return false;
    }

    std::vector<uint8_t> bytes =
        Serializer::serialize(row);

    int offset = usedSpace();

    int recordSize =
        static_cast<int>(bytes.size());


    // ------------------------------------------
    // Store record size
    // ------------------------------------------

    std::memcpy(
        pageData + offset,
        &recordSize,
        sizeof(int)
    );

    offset += RECORD_SIZE_HEADER;


    // ------------------------------------------
    // Store serialized Row
    // ------------------------------------------

    std::memcpy(
        pageData + offset,
        bytes.data(),
        bytes.size()
    );


    // ------------------------------------------
    // Increase record count
    // ------------------------------------------

    int recordCount = size();

    recordCount++;

    std::memcpy(
        pageData,
        &recordCount,
        sizeof(int)
    );


    return true;
}


// ============================================================
// Get Row
// ============================================================

Row RecordPage::get(int index) const
{
    int offset =
        getRecordOffset(index);


    int recordSize;

    std::memcpy(
        &recordSize,
        pageData + offset,
        sizeof(int)
    );

    offset += RECORD_SIZE_HEADER;


    std::vector<uint8_t> bytes(
        pageData + offset,
        pageData + offset + recordSize
    );


    return Serializer::deserialize(bytes);
}


// ============================================================
// Number of records
// ============================================================

int RecordPage::size() const
{
    if (pageData == nullptr)
    {
        return 0;
    }

    int recordCount;

    std::memcpy(
        &recordCount,
        pageData,
        sizeof(int)
    );

    return recordCount;
}


// ============================================================
// Bytes currently used by records
// ============================================================

int RecordPage::usedSpace() const
{
    if (pageData == nullptr)
    {
        return 0;
    }

    int offset = HEADER_SIZE;

    int recordCount = size();

    for (int i = 0; i < recordCount; i++)
    {
        int recordSize;

        std::memcpy(
            &recordSize,
            pageData + offset,
            sizeof(int)
        );

        offset += RECORD_SIZE_HEADER;
        offset += recordSize;
    }

    return offset;
}


// ============================================================
// Remaining free space
// ============================================================

int RecordPage::freeSpace() const
{
    return PAGE_SIZE - usedSpace();
}


// ============================================================
// Clear page
// ============================================================

void RecordPage::clear()
{
    if (pageData == nullptr)
    {
        return;
    }

    std::memset(
        pageData,
        0,
        PAGE_SIZE
    );
}