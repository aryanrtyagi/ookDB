#ifndef RECORD_PAGE_H
#define RECORD_PAGE_H

#include "../core/pager/pager.h"
#include "../record/row.h"

#include <cstdint>
#include <limits>

class RecordPage
{
private:
    char* pageData;

    // Page header layout:
    // 0  : Magic number       (4 bytes)
    // 4  : Record count       (4 bytes)
    // 8  : Next page          (4 bytes)
    // 12 : Slot count         (4 bytes)
    // 16 : Free start         (4 bytes)
    // 20 : Free end           (4 bytes)

    static constexpr uint32_t MAGIC = 0x41525953; // ARYS

    static constexpr uint32_t HEADER_SIZE = 24;
    static constexpr uint32_t SLOT_SIZE = 8;

    static constexpr uint32_t INVALID_SLOT =
        std::numeric_limits<uint32_t>::max();

    uint32_t readUInt32(uint32_t offset) const;
    void writeUInt32(uint32_t offset, uint32_t value);

    uint16_t readUInt16(uint32_t offset) const;
    void writeUInt16(uint32_t offset, uint16_t value);

    uint32_t getSlotOffset(uint32_t slotNumber) const;

    uint16_t getSlotDataOffset(uint32_t slotNumber) const;
    uint16_t getSlotLength(uint32_t slotNumber) const;
    uint32_t getSlotGeneration(uint32_t slotNumber) const;

    void setSlot(
        uint32_t slotNumber,
        uint16_t offset,
        uint16_t length,
        uint32_t generation
    );

    bool findLiveSlot(
        int logicalIndex,
        uint32_t& slotNumber
    ) const;

    bool findReusableSlot(uint32_t& slotNumber) const;

    void compact(uint32_t excludedSlot = INVALID_SLOT);

    void initializeIfNeeded();

    bool updateSlot(
        uint32_t slotNumber,
        const Row& row
    );

public:

    explicit RecordPage(char* page);

    bool hasSpace(const Row& row) const;

    // Existing API: retained for compatibility.
    bool insert(const Row& row);

    // New API: returns the allocated slot and generation.
    bool insert(
        const Row& row,
        uint32_t& slotNumber,
        uint32_t& generation
    );

    // Existing logical-index API.
    Row get(int index) const;

    // New stable-slot API.
    Row getBySlot(
        uint32_t slotNumber,
        uint32_t generation
    ) const;

    bool isValidSlot(
        uint32_t slotNumber,
        uint32_t generation
    ) const;

    int size() const;

    int usedSpace() const;

    int freeSpace() const;

    int getNextPage() const;

    void setNextPage(int pageNumber);

    // Existing logical-index API.
    bool update(int index, const Row& row);

    bool erase(int index);

    // New stable-slot APIs.
    bool updateBySlot(
        uint32_t slotNumber,
        uint32_t generation,
        const Row& row
    );

    bool eraseBySlot(
        uint32_t slotNumber,
        uint32_t generation
    );

    void clear();
};

#endif
