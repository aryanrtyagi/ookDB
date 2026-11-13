#ifndef VALUE_H
#define VALUE_H

#include <string>

enum class DataType
{
    INT,
    DOUBLE,
    BOOLEAN,
    VARCHAR,
    NULL_VALUE
};

class Value
{
private:
    DataType type;

    int intValue;
    double doubleValue;
    bool boolValue;
    std::string stringValue;

public:
    // A default-constructed Value represents SQL NULL.
    Value();

    Value(int value);
    Value(double value);
    Value(bool value);
    Value(const std::string& value);
    Value(const char* value);

    // Explicit NULL factory.
    static Value null();

    DataType getType() const;

    bool isNull() const;

    int asInt() const;
    double asDouble() const;
    bool asBool() const;
    std::string asString() const;
};

#endif