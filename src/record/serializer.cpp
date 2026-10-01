#include "serializer.h"

#include <cstring>
#include <stdexcept>

void writeInt(
    std::vector<uint8_t>& buffer,
    int value
)
{
    uint8_t* bytes =
        reinterpret_cast<uint8_t*>(&value);

    for (int i = 0; i < sizeof(int); i++)
    {
        buffer.push_back(bytes[i]);
    }
}

int readInt(
    const std::vector<uint8_t>& buffer,
    size_t& offset
)
{
    int value;

    std::memcpy(
        &value,
        buffer.data() + offset,
        sizeof(int)
    );

    offset += sizeof(int);

    return value;
}


void writeDouble(
    std::vector<uint8_t>& buffer,
    double value
)
{
    uint8_t* bytes =
        reinterpret_cast<uint8_t*>(&value);

    for (int i = 0; i < sizeof(double); i++)
    {
        buffer.push_back(bytes[i]);
    }
}


double readDouble(
    const std::vector<uint8_t>& buffer,
    size_t& offset
)
{
    double value;

    std::memcpy(
        &value,
        buffer.data() + offset,
        sizeof(double)
    );

    offset += sizeof(double);

    return value;
}


std::vector<uint8_t> Serializer::serialize(
    const Row& row
)
{
    std::vector<uint8_t> buffer;

    // Number of values
    writeInt(buffer, row.size());

    for (int i = 0; i < row.size(); i++)
    {
        const Value& value = row.getValue(i);

        DataType type = value.getType();

        // Store type
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

                writeInt(
                    buffer,
                    static_cast<int>(str.size())
                );

                for (char c : str)
                {
                    buffer.push_back(
                        static_cast<uint8_t>(c)
                    );
                }

                break;
            }
        }
    }

    return buffer;
}


Row Serializer::deserialize(
    const std::vector<uint8_t>& data
)
{
    Row row;

    size_t offset = 0;

    int valueCount =
        readInt(data, offset);

    for (int i = 0; i < valueCount; i++)
    {
        DataType type =
            static_cast<DataType>(
                data[offset++]
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
                    data[offset++] != 0;

                row.addValue(
                    Value(value)
                );

                break;
            }

            case DataType::VARCHAR:
            {
                int length =
                    readInt(data, offset);

                std::string str;

                for (int j = 0; j < length; j++)
                {
                    str +=
                        static_cast<char>(
                            data[offset++]
                        );
                }

                row.addValue(
                    Value(str)
                );

                break;
            }

            default:
                throw std::runtime_error(
                    "Invalid DataType"
                );
        }
    }

    return row;
}