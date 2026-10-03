#ifndef VALUE_H
#define VALUE_H

#include <string>

enum class DataType
{
    INT,
    DOUBLE,
    BOOLEAN,
    VARCHAR
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
    Value(int value);
    Value(double value);
    Value(bool value);
    Value(const std::string& value);
    Value(const char* value);

    DataType getType() const;

    int asInt() const;
    double asDouble() const;
    bool asBool() const;
    std::string asString() const;
};

#endif