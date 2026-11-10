#ifndef RECORD_PAGE_H
#define RECORD_PAGE_H

#include "../core/pager/pager.h"
#include "../record/row.h"

#include <vector>
#include <utility>
#include <cstdint>

class RecordPage
{
private:
    char *pageData;

    static constexpr uint32_t MAGIC = 0x41525953;
    static constexpr int HEADER_SIZE = 24;
    static constexpr int SLOT_SIZE = 8;

    // Raw integer access
    uint32_t readUInt32(uint32_t offset) const;
    void writeUInt32(uint32_t offset, uint32_t value);

    uint16_t readUInt16(uint32_t offset) const;
    void writeUInt16(uint32_t offset, uint16_t value);

    // Initialization
    void initializeIfNeeded();

    // Slot directory
    uint32_t getSlotOffset(uint32_t slotNumber) const;
    uint16_t getSlotDataOffset(uint32_t slotNumber) const;
    uint16_t getSlotLength(uint32_t slotNumber) const;
    uint32_t getSlotGeneration(uint32_t slotNumber) const;

    void setSlot(
        uint32_t slotNumber,
        uint16_t offset,
        uint16_t length,
        uint32_t generation);

    // Slot lookup and validation
    bool findLiveSlot(int logicalIndex, uint32_t &slotNumber) const;
    bool findReusableSlot(uint32_t &slotNumber) const;

    // Space management
    void compact(uint32_t excludedSlot = UINT32_MAX);

    bool updateSlot(
        uint32_t slotNumber,
        const Row &row);

public:
    explicit RecordPage(char *page);

    // Space information
    bool hasSpace(const Row &row) const;

    int size() const;
    int usedSpace() const;
    int freeSpace() const;

    bool isValidSlot(
        uint32_t slotNumber,
        uint32_t generation) const;
    // Return all active slots as (slotNumber, generation).
    std::vector<std::pair<uint32_t, uint32_t>>
    getLiveSlots() const;

    // Insert
    bool insert(const Row &row);

    bool insert(
        const Row &row,
        uint32_t &slotNumber,
        uint32_t &generation);

    // Stable slot-based operations
    Row getBySlot(
        uint32_t slotNumber,
        uint32_t generation) const;

    bool updateBySlot(
        uint32_t slotNumber,
        uint32_t generation,
        const Row &row);

    bool eraseBySlot(
        uint32_t slotNumber,
        uint32_t generation);

    // Temporary compatibility with the old logical-index API
    Row get(int index) const;
    bool update(int index, const Row &row);
    bool erase(int index);

    // Page chain
    int getNextPage() const;
    void setNextPage(int pageNumber);

    void clear();
};

#endif
