
#include "record_page.h"

#include "../record/serializer.h"

#include <cstring>
#include <stdexcept>
#include <vector>
#include <utility>
#include <limits>


RecordPage::RecordPage(char* page)
    : pageData(page)
{
    initializeIfNeeded();
}


// ============================================================
// Raw integer access
// ============================================================

uint32_t RecordPage::readUInt32(uint32_t offset) const
{
    uint32_t value = 0;

    std::memcpy(
        &value,
        pageData + offset,
        sizeof(uint32_t)
    );

    return value;
}


void RecordPage::writeUInt32(
    uint32_t offset,
    uint32_t value
)
{
    std::memcpy(
        pageData + offset,
        &value,
        sizeof(uint32_t)
    );
}


uint16_t RecordPage::readUInt16(uint32_t offset) const
{
    uint16_t value = 0;

    std::memcpy(
        &value,
        pageData + offset,
        sizeof(uint16_t)
    );

    return value;
}


void RecordPage::writeUInt16(
    uint32_t offset,
    uint16_t value
)
{
    std::memcpy(
        pageData + offset,
        &value,
        sizeof(uint16_t)
    );
}


// ============================================================
// Initialization
// ============================================================

void RecordPage::initializeIfNeeded()
{
    if (pageData == nullptr)
    {
        throw std::runtime_error(
            "RecordPage received a null page"
        );
    }

    if (readUInt32(0) != MAGIC)
    {
        clear();
    }
}


void RecordPage::clear()
{
    if (pageData == nullptr)
        return;

    std::memset(pageData, 0, PAGE_SIZE);

    writeUInt32(0, MAGIC);
    writeUInt32(4, 0);

    int32_t noNextPage = -1;

    std::memcpy(
        pageData + 8,
        &noNextPage,
        sizeof(int32_t)
    );

    writeUInt32(12, 0);
    writeUInt32(16, HEADER_SIZE);
    writeUInt32(20, PAGE_SIZE);
}


// ============================================================
// Slot directory
// ============================================================

uint32_t RecordPage::getSlotOffset(
    uint32_t slotNumber
) const
{
    return PAGE_SIZE - ((slotNumber + 1) * SLOT_SIZE);
}


uint16_t RecordPage::getSlotDataOffset(
    uint32_t slotNumber
) const
{
    return readUInt16(
        getSlotOffset(slotNumber)
    );
}


uint16_t RecordPage::getSlotLength(
    uint32_t slotNumber
) const
{
    return readUInt16(
        getSlotOffset(slotNumber) + 2
    );
}


uint32_t RecordPage::getSlotGeneration(
    uint32_t slotNumber
) const
{
    return readUInt32(
        getSlotOffset(slotNumber) + 4
    );
}


void RecordPage::setSlot(
    uint32_t slotNumber,
    uint16_t offset,
    uint16_t length,
    uint32_t generation
)
{
    uint32_t position =
        getSlotOffset(slotNumber);

    writeUInt16(position, offset);
    writeUInt16(position + 2, length);
    writeUInt32(position + 4, generation);
}


// ============================================================
// Record count
// ============================================================

int RecordPage::size() const
{
    if (pageData == nullptr ||
        readUInt32(0) != MAGIC)
    {
        return 0;
    }

    return static_cast<int>(
        readUInt32(4)
    );
}


// ============================================================
// Find logical record index
// ============================================================

bool RecordPage::findLiveSlot(
    int logicalIndex,
    uint32_t& slotNumber
) const
{
    if (logicalIndex < 0 ||
        logicalIndex >= size())
    {
        return false;
    }

    int currentIndex = 0;

    uint32_t slotCount = readUInt32(12);

    for (uint32_t i = 0; i < slotCount; i++)
    {
        if (getSlotLength(i) == 0)
            continue;

        if (currentIndex == logicalIndex)
        {
            slotNumber = i;
            return true;
        }

        currentIndex++;
    }

    return false;
}


// ============================================================
// Find reusable deleted slot
// ============================================================

bool RecordPage::findReusableSlot(
    uint32_t& slotNumber
) const
{
    uint32_t slotCount = readUInt32(12);

    for (uint32_t i = 0; i < slotCount; i++)
    {
        if (getSlotLength(i) == 0 &&
            getSlotGeneration(i) <
                std::numeric_limits<uint32_t>::max())
        {
            slotNumber = i;
            return true;
        }
    }

    return false;
}


// ============================================================
// Space management
// ============================================================

int RecordPage::usedSpace() const
{
    return PAGE_SIZE - freeSpace();
}


int RecordPage::freeSpace() const
{
    if (pageData == nullptr ||
        readUInt32(0) != MAGIC)
    {
        return PAGE_SIZE - HEADER_SIZE;
    }

    uint32_t liveBytes = 0;
    uint32_t slotCount = readUInt32(12);

    for (uint32_t i = 0; i < slotCount; i++)
    {
        liveBytes += getSlotLength(i);
    }

    uint32_t directoryBytes =
        slotCount * SLOT_SIZE;

    uint32_t occupied =
        HEADER_SIZE +
        liveBytes +
        directoryBytes;

    if (occupied >= PAGE_SIZE)
        return 0;

    return static_cast<int>(
        PAGE_SIZE - occupied
    );
}


bool RecordPage::hasSpace(const Row& row) const
{
    std::vector<uint8_t> bytes =
        Serializer::serialize(row);

    if (bytes.empty() ||
        bytes.size() >
            std::numeric_limits<uint16_t>::max())
    {
        return false;
    }

    uint32_t reusableSlot;

    uint32_t extraSlotSpace =
        findReusableSlot(reusableSlot)
            ? 0
            : SLOT_SIZE;

    return freeSpace() >=
        static_cast<int>(
            bytes.size() + extraSlotSpace
        );
}


// ============================================================
// Compact record data
// ============================================================

void RecordPage::compact(uint32_t excludedSlot)
{
    struct RecordData
    {
        uint32_t slotNumber;
        uint32_t generation;
        std::vector<char> bytes;
    };

    std::vector<RecordData> records;

    uint32_t slotCount = readUInt32(12);

    for (uint32_t i = 0; i < slotCount; i++)
    {
        if (i == excludedSlot)
            continue;

        uint16_t length = getSlotLength(i);

        if (length == 0)
            continue;

        uint16_t offset = getSlotDataOffset(i);

        RecordData record;

        record.slotNumber = i;
        record.generation = getSlotGeneration(i);

        record.bytes.assign(
            pageData + offset,
            pageData + offset + length
        );

        records.push_back(
            std::move(record)
        );
    }

    uint32_t newOffset = HEADER_SIZE;

    for (const RecordData& record : records)
    {
        std::memcpy(
            pageData + newOffset,
            record.bytes.data(),
            record.bytes.size()
        );

        setSlot(
            record.slotNumber,
            static_cast<uint16_t>(newOffset),
            static_cast<uint16_t>(record.bytes.size()),
            record.generation
        );

        newOffset +=
            static_cast<uint32_t>(
                record.bytes.size()
            );
    }

    writeUInt32(16, newOffset);

    writeUInt32(
        20,
        PAGE_SIZE - slotCount * SLOT_SIZE
    );
}


// ============================================================
// Insert
// ============================================================

bool RecordPage::insert(const Row& row)
{
    uint32_t slot;
    uint32_t generation;

    return insert(row, slot, generation);
}


bool RecordPage::insert(
    const Row& row,
    uint32_t& slotNumber,
    uint32_t& generation
)
{
    std::vector<uint8_t> bytes =
        Serializer::serialize(row);

    if (bytes.empty() ||
        bytes.size() >
            std::numeric_limits<uint16_t>::max())
    {
        return false;
    }

    uint32_t reusableSlot;

    bool reuseExisting =
        findReusableSlot(reusableSlot);

    uint32_t requiredDirectorySpace =
        reuseExisting ? 0 : SLOT_SIZE;

    uint32_t slotCount = readUInt32(12);

    if (!reuseExisting &&
        PAGE_SIZE - (slotCount + 1) * SLOT_SIZE
            < HEADER_SIZE)
    {
        return false;
    }

    uint32_t freeStart = readUInt32(16);
    uint32_t freeEnd = readUInt32(20);

    uint32_t required =
        static_cast<uint32_t>(bytes.size()) +
        requiredDirectorySpace;

    if (freeEnd - freeStart < required)
    {
        compact();

        freeStart = readUInt32(16);
        freeEnd = readUInt32(20);
    }

    if (freeEnd - freeStart < required)
        return false;

    if (reuseExisting)
    {
        slotNumber = reusableSlot;

        generation =
            getSlotGeneration(slotNumber) + 1;
    }
    else
    {
        slotNumber = slotCount;
        generation = 1;

        writeUInt32(12, slotCount + 1);

        freeEnd -= SLOT_SIZE;
        writeUInt32(20, freeEnd);
    }

    std::memcpy(
        pageData + freeStart,
        bytes.data(),
        bytes.size()
    );

    setSlot(
        slotNumber,
        static_cast<uint16_t>(freeStart),
        static_cast<uint16_t>(bytes.size()),
        generation
    );

    writeUInt32(
        16,
        freeStart + static_cast<uint32_t>(bytes.size())
    );

    writeUInt32(
        4,
        readUInt32(4) + 1
    );

    return true;
}


// ============================================================
// Validate RID components
// ============================================================

bool RecordPage::isValidSlot(
    uint32_t slotNumber,
    uint32_t generation
) const
{
    if (pageData == nullptr ||
        readUInt32(0) != MAGIC)
    {
        return false;
    }

    uint32_t slotCount = readUInt32(12);

    if (slotNumber >= slotCount)
        return false;

    if (getSlotLength(slotNumber) == 0)
        return false;

    return getSlotGeneration(slotNumber) ==
        generation;
}


// ============================================================
// Get by stable slot
// ============================================================

Row RecordPage::getBySlot(
    uint32_t slotNumber,
    uint32_t generation
) const
{
    if (!isValidSlot(slotNumber, generation))
    {
        throw std::out_of_range(
            "Invalid or stale RecordId"
        );
    }

    uint16_t offset =
        getSlotDataOffset(slotNumber);

    uint16_t length =
        getSlotLength(slotNumber);

    std::vector<uint8_t> bytes(
        pageData + offset,
        pageData + offset + length
    );

    return Serializer::deserialize(bytes);
}


// ============================================================
// Existing logical-index get
// ============================================================

Row RecordPage::get(int index) const
{
    uint32_t slotNumber;

    if (!findLiveSlot(index, slotNumber))
    {
        throw std::out_of_range(
            "Invalid record index"
        );
    }

    return getBySlot(
        slotNumber,
        getSlotGeneration(slotNumber)
    );
}


// ============================================================
// Update by stable slot
// ============================================================

bool RecordPage::updateSlot(
    uint32_t slotNumber,
    const Row& row
)
{
    std::vector<uint8_t> bytes =
        Serializer::serialize(row);

    if (bytes.empty() ||
        bytes.size() >
            std::numeric_limits<uint16_t>::max())
    {
        return false;
    }

    uint16_t oldLength =
        getSlotLength(slotNumber);

    if (bytes.size() <= oldLength)
    {
        uint16_t offset =
            getSlotDataOffset(slotNumber);

        std::memcpy(
            pageData + offset,
            bytes.data(),
            bytes.size()
        );

        setSlot(
            slotNumber,
            offset,
            static_cast<uint16_t>(bytes.size()),
            getSlotGeneration(slotNumber)
        );

        return true;
    }

    // Check capacity before changing page contents.
    if (freeSpace() + oldLength < bytes.size())
        return false;

    // Compact all other records.
    // The target slot keeps its identity.
    compact(slotNumber);

    uint32_t freeStart = readUInt32(16);
    uint32_t freeEnd = readUInt32(20);

    if (freeEnd - freeStart < bytes.size())
        return false;

    std::memcpy(
        pageData + freeStart,
        bytes.data(),
        bytes.size()
    );

    setSlot(
        slotNumber,
        static_cast<uint16_t>(freeStart),
        static_cast<uint16_t>(bytes.size()),
        getSlotGeneration(slotNumber)
    );

    writeUInt32(
        16,
        freeStart + static_cast<uint32_t>(bytes.size())
    );

    return true;
}


bool RecordPage::updateBySlot(
    uint32_t slotNumber,
    uint32_t generation,
    const Row& row
)
{
    if (!isValidSlot(slotNumber, generation))
        return false;

    return updateSlot(slotNumber, row);
}


// ============================================================
// Existing logical-index update
// ============================================================

bool RecordPage::update(
    int index,
    const Row& row
)
{
    uint32_t slotNumber;

    if (!findLiveSlot(index, slotNumber))
        return false;

    return updateSlot(slotNumber, row);
}


// ============================================================
// Delete by stable slot
// ============================================================

bool RecordPage::eraseBySlot(
    uint32_t slotNumber,
    uint32_t generation
)
{
    if (!isValidSlot(slotNumber, generation))
        return false;

    setSlot(
        slotNumber,
        0,
        0,
        generation
    );

    writeUInt32(
        4,
        readUInt32(4) - 1
    );

    return true;
}


// ============================================================
// Existing logical-index delete
// ============================================================

bool RecordPage::erase(int index)
{
    uint32_t slotNumber;

    if (!findLiveSlot(index, slotNumber))
        return false;

    return eraseBySlot(
        slotNumber,
        getSlotGeneration(slotNumber)
    );
}


// ============================================================
// Page chain
// ============================================================

int RecordPage::getNextPage() const
{
    if (pageData == nullptr ||
        readUInt32(0) != MAGIC)
    {
        return -1;
    }

    int32_t nextPage = -1;

    std::memcpy(
        &nextPage,
        pageData + 8,
        sizeof(int32_t)
    );

    return nextPage;
}


void RecordPage::setNextPage(int pageNumber)
{
    if (pageData == nullptr)
        return;

    if (readUInt32(0) != MAGIC)
        clear();

    int32_t value = pageNumber;

    std::memcpy(
        pageData + 8,
        &value,
        sizeof(int32_t)
    );
}
