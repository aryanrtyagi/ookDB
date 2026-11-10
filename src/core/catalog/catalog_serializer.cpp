
#include "catalog_serializer.h"

#include <cstring>
#include <stdexcept>
#include <limits>

// ============================================================
// Serialization helpers
// ============================================================

static void writeInt(
    std::vector<uint8_t>& buffer,
    int value
)
{
    const uint8_t* bytes =
        reinterpret_cast<const uint8_t*>(&value);

    buffer.insert(
        buffer.end(),
        bytes,
        bytes + sizeof(int)
    );
}

static int readInt(
    const std::vector<uint8_t>& buffer,
    size_t& offset
)
{
    if (offset + sizeof(int) > buffer.size())
    {
        throw std::runtime_error(
            "Invalid catalog data"
        );
    }

    int value = 0;

    std::memcpy(
        &value,
        buffer.data() + offset,
        sizeof(int)
    );

    offset += sizeof(int);

    return value;
}

static void writeString(
    std::vector<uint8_t>& buffer,
    const std::string& value
)
{
    if (value.size() >
        static_cast<size_t>(std::numeric_limits<int>::max()))
    {
        throw std::runtime_error(
            "Catalog string is too large"
        );
    }

    writeInt(
        buffer,
        static_cast<int>(value.size())
    );

    buffer.insert(
        buffer.end(),
        value.begin(),
        value.end()
    );
}

static std::string readString(
    const std::vector<uint8_t>& buffer,
    size_t& offset
)
{
    int length = readInt(buffer, offset);

    if (length < 0 ||
        static_cast<size_t>(length) > buffer.size() - offset)
    {
        throw std::runtime_error(
            "Invalid catalog string"
        );
    }

    std::string result(
        reinterpret_cast<const char*>(
            buffer.data() + offset
        ),
        static_cast<size_t>(length)
    );

    offset += static_cast<size_t>(length);

    return result;
}

// ============================================================
// SERIALIZE CATALOG
// ============================================================

std::vector<uint8_t>
CatalogSerializer::serialize(const Catalog& catalog)
{
    std::vector<uint8_t> buffer;

    const auto& tables = catalog.getTables();

    // --------------------------------------
    // Number of tables
    // --------------------------------------

    writeInt(
        buffer,
        static_cast<int>(tables.size())
    );

    // --------------------------------------
    // Table metadata
    // --------------------------------------

    for (const auto& metadata : tables)
    {
        writeString(
            buffer,
            metadata.name
        );

        int columnCount = metadata.schema.size();

        writeInt(
            buffer,
            columnCount
        );

        for (int i = 0; i < columnCount; i++)
        {
            const Column& column =
                metadata.schema.getColumn(i);

            writeString(
                buffer,
                column.getName()
            );

            writeInt(
                buffer,
                static_cast<int>(column.getType())
            );
        }

        writeInt(
            buffer,
            metadata.firstPage
        );
    }

    // --------------------------------------
    // Number of indexes
    // --------------------------------------

    const auto& indexes = catalog.getIndexes();

    writeInt(
        buffer,
        static_cast<int>(indexes.size())
    );

    // --------------------------------------
    // Index metadata
    // --------------------------------------

    for (const auto& index : indexes)
    {
        writeString(buffer, index.indexName);
        writeString(buffer, index.tableName);
        writeString(buffer, index.columnName);
        writeString(buffer, index.filePath);
    }

    return buffer;
}

// ============================================================
// DESERIALIZE CATALOG
// ============================================================

void CatalogSerializer::deserialize(
    const std::vector<uint8_t>& data,
    Catalog& catalog
)
{
    catalog.clear();

    size_t offset = 0;

    int tableCount = readInt(data, offset);

    if (tableCount < 0 || tableCount > 100000)
    {
        throw std::runtime_error(
            "Invalid catalog table count"
        );
    }

    // --------------------------------------
    // Restore tables
    // --------------------------------------

    for (int i = 0; i < tableCount; i++)
    {
        std::string tableName =
            readString(data, offset);

        Schema schema;

        int columnCount = readInt(data, offset);

        if (columnCount < 0 || columnCount > 10000)
        {
            throw std::runtime_error(
                "Invalid catalog column count"
            );
        }

        for (int j = 0; j < columnCount; j++)
        {
            std::string columnName =
                readString(data, offset);

            int type = readInt(data, offset);

            if (type < static_cast<int>(DataType::INT) ||
                type > static_cast<int>(DataType::VARCHAR))
            {
                throw std::runtime_error(
                    "Invalid column data type in catalog"
                );
            }

            schema.addColumn(
                columnName,
                static_cast<DataType>(type)
            );
        }

        int firstPage = readInt(data, offset);

        catalog.restoreTable(
            tableName,
            schema,
            firstPage
        );
    }

    // --------------------------------------
    // Backward compatibility
    // --------------------------------------
    // Older catalog files ended after table metadata.
    // If no bytes remain, they simply have zero indexes.

    if (offset == data.size())
        return;

    // --------------------------------------
    // Restore indexes
    // --------------------------------------

    int indexCount = readInt(data, offset);

    if (indexCount < 0 || indexCount > 100000)
    {
        throw std::runtime_error(
            "Invalid catalog index count"
        );
    }

    for (int i = 0; i < indexCount; i++)
    {
        std::string indexName =
            readString(data, offset);

        std::string tableName =
            readString(data, offset);

        std::string columnName =
            readString(data, offset);

        std::string filePath =
            readString(data, offset);

        catalog.restoreIndex(
            indexName,
            tableName,
            columnName,
            filePath
        );
    }

    if (offset != data.size())
    {
        throw std::runtime_error(
            "Unexpected trailing catalog data"
        );
    }
}
