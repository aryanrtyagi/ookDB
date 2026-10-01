#include "catalog_serializer.h"

#include <cstring>
#include <stdexcept>

// ============================================================
// Helpers
// ============================================================

static void writeInt(
    std::vector<uint8_t> &buffer,
    int value)
{
    uint8_t *bytes =
        reinterpret_cast<uint8_t *>(&value);

    buffer.insert(
        buffer.end(),
        bytes,
        bytes + sizeof(int));
}

static int readInt(
    const std::vector<uint8_t> &buffer,
    size_t &offset)
{
    if (offset + sizeof(int) > buffer.size())
    {
        throw std::runtime_error(
            "Invalid catalog data");
    }

    int value;

    std::memcpy(
        &value,
        buffer.data() + offset,
        sizeof(int));

    offset += sizeof(int);

    return value;
}

static void writeString(
    std::vector<uint8_t> &buffer,
    const std::string &value)
{
    writeInt(
        buffer,
        static_cast<int>(value.size()));

    buffer.insert(
        buffer.end(),
        value.begin(),
        value.end());
}

static std::string readString(
    const std::vector<uint8_t> &buffer,
    size_t &offset)
{
    int length =
        readInt(buffer, offset);

    if (
        length < 0 ||
        offset + static_cast<size_t>(length) > buffer.size())
    {
        throw std::runtime_error(
            "Invalid catalog string");
    }

    std::string result(
        reinterpret_cast<const char *>(
            buffer.data() + offset),
        length);

    offset += length;

    return result;
}

// ============================================================
// Serialize Catalog
// ============================================================

std::vector<uint8_t>
CatalogSerializer::serialize(
    const Catalog &catalog)
{
    std::vector<uint8_t> buffer;

    const auto &tables =
        catalog.getTables();

    // Number of tables
    writeInt(
        buffer,
        static_cast<int>(tables.size()));

    for (const auto &entry : tables)
    {
        const TableMetadata &metadata =
            entry.second;

        // --------------------------------------
        // Table name
        // --------------------------------------

        writeString(
            buffer,
            metadata.name);

        // --------------------------------------
        // Number of columns
        // --------------------------------------

        int columnCount =
            metadata.schema.size();

        writeInt(
            buffer,
            columnCount
        );

        for (int i = 0; i < columnCount; i++)
        {
            const Column &column =
                metadata.schema.getColumn(i);

            writeString(
                buffer,
                column.getName());

            writeInt(
                buffer,
                static_cast<int>(
                    column.getType()));
        }

        // --------------------------------------
        // First page
        // --------------------------------------

        writeInt(
            buffer,
            metadata.firstPage);
    }

    return buffer;
}

// ============================================================
// Deserialize Catalog
// ============================================================

void CatalogSerializer::deserialize(
    const std::vector<uint8_t> &data,
    Catalog &catalog)
{
    catalog.clear();

    size_t offset = 0;

    int tableCount =
        readInt(data, offset);

    for (int i = 0; i < tableCount; i++)
    {
        // --------------------------------------
        // Table name
        // --------------------------------------

        std::string tableName =
            readString(data, offset);

        // --------------------------------------
        // Schema
        // --------------------------------------

        Schema schema;

        int columnCount =
            readInt(data, offset);

        for (int j = 0; j < columnCount; j++)
        {
            std::string columnName =
                readString(data, offset);

            int type =
                readInt(data, offset);

            schema.addColumn(
                columnName,
                static_cast<DataType>(type));
        }

        // --------------------------------------
        // First page
        // --------------------------------------

        int firstPage =
            readInt(data, offset);

        catalog.createTable(
            tableName,
            schema,
            firstPage);
    }
}