#include "serializer.h"

#include <cstring>
#include <stdexcept>
#include <limits>

// ============================================================
// Integer serialization
// ============================================================

static void writeInt(
    std::vector<uint8_t>& buffer,
    int value
)
{
    uint8_t* bytes =
        reinterpret_cast<uint8_t*>(&value);

    for (int i = 0; i < static_cast<int>(sizeof(int)); i++)
    {
        buffer.push_back(bytes[i]);
    }
}

static int readInt(
    const std::vector<uint8_t>& buffer,
    size_t& offset
)
{
    if (offset > buffer.size() ||
        buffer.size() - offset < sizeof(int))
    {
        throw std::runtime_error(
            "Corrupted serialized data: incomplete integer"
        );
    }

    int value;

    std::memcpy(
        &value,
        buffer.data() + offset,
        sizeof(int)
    );

    offset += sizeof(int);

    return value;
}

// ============================================================
// Double serialization
// ============================================================

static void writeDouble(
    std::vector<uint8_t>& buffer,
    double value
)
{
    uint8_t* bytes =
        reinterpret_cast<uint8_t*>(&value);

    for (int i = 0; i < static_cast<int>(sizeof(double)); i++)
    {
        buffer.push_back(bytes[i]);
    }
}

static double readDouble(
    const std::vector<uint8_t>& buffer,
    size_t& offset
)
{
    if (offset > buffer.size() ||
        buffer.size() - offset < sizeof(double))
    {
        throw std::runtime_error(
            "Corrupted serialized data: incomplete double"
        );
    }

    double value;

    std::memcpy(
        &value,
        buffer.data() + offset,
        sizeof(double)
    );

    offset += sizeof(double);

    return value;
}

// ============================================================
// Byte serialization
// ============================================================

static uint8_t readByte(
    const std::vector<uint8_t>& buffer,
    size_t& offset
)
{
    if (offset >= buffer.size())
    {
        throw std::runtime_error(
            "Corrupted serialized data: missing byte"
        );
    }

    return buffer[offset++];
}

// ============================================================
// Serialize Row
// ============================================================

std::vector<uint8_t> Serializer::serialize(
    const Row& row
)
{
    std::vector<uint8_t> buffer;

    // Number of values in the row.
    writeInt(buffer, row.size());

    for (int i = 0; i < row.size(); i++)
    {
        const Value& value = row.getValue(i);

        DataType type = value.getType();

        // Store the value type.
        buffer.push_back(
            static_cast<uint8_t>(type)
        );

        switch (type)
        {
            case DataType::INT:
            {
                writeInt(
                    buffer,
                    value.asInt()
                );

                break;
            }

            case DataType::DOUBLE:
            {
                writeDouble(
                    buffer,
                    value.asDouble()
                );

                break;
            }

            case DataType::BOOLEAN:
            {
                buffer.push_back(
                    value.asBool() ? 1 : 0
                );

                break;
            }

            case DataType::VARCHAR:
            {
                std::string str =
                    value.asString();

                if (str.size() >
                    static_cast<size_t>(
                        std::numeric_limits<int>::max()
                    ))
                {
                    throw std::runtime_error(
                        "String is too large to serialize"
                    );
                }

                writeInt(
                    buffer,
                    static_cast<int>(str.size())
                );

                for (unsigned char c : str)
                {
                    buffer.push_back(c);
                }

                break;
            }

            case DataType::NULL_VALUE:
            {
                // NULL has no payload.
                // Its type byte is sufficient.
                break;
            }

            default:
            {
                throw std::runtime_error(
                    "Invalid DataType during serialization"
                );
            }
        }
    }

    return buffer;
}

// ============================================================
// Deserialize Row
// ============================================================

Row Serializer::deserialize(
    const std::vector<uint8_t>& data
)
{
    Row row;

    size_t offset = 0;

    int valueCount =
        readInt(data, offset);

    if (valueCount < 0)
    {
        throw std::runtime_error(
            "Corrupted serialized data: invalid value count"
        );
    }

    // Every value needs at least one type byte.
    if (static_cast<size_t>(valueCount) >
        data.size() - offset)
    {
        throw std::runtime_error(
            "Corrupted serialized data: invalid row size"
        );
    }

    for (int i = 0; i < valueCount; i++)
    {
        DataType type =
            static_cast<DataType>(
                readByte(data, offset)
            );

        switch (type)
        {
            case DataType::INT:
            {
                int value =
                    readInt(data, offset);

                row.addValue(
                    Value(value)
                );

                break;
            }

            case DataType::DOUBLE:
            {
                double value =
                    readDouble(data, offset);

                row.addValue(
                    Value(value)
                );

                break;
            }

            case DataType::BOOLEAN:
            {
                bool value =
                    readByte(data, offset) != 0;

                row.addValue(
                    Value(value)
                );

                break;
            }

            case DataType::VARCHAR:
            {
                int length =
                    readInt(data, offset);

                if (length < 0 ||
                    static_cast<size_t>(length) >
                        data.size() - offset)
                {
                    throw std::runtime_error(
                        "Corrupted serialized data: invalid string length"
                    );
                }

                std::string str;

                str.reserve(
                    static_cast<size_t>(length)
                );

                for (int j = 0; j < length; j++)
                {
                    str += static_cast<char>(
                        data[offset++]
                    );
                }

                row.addValue(
                    Value(str)
                );

                break;
            }

            case DataType::NULL_VALUE:
            {
                row.addValue(
                    Value::null()
                );

                break;
            }

            default:
            {
                throw std::runtime_error(
                    "Invalid DataType during deserialization"
                );
            }
        }
    }

    return row;
}