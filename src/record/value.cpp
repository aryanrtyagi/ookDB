#include "value.h"

Value::Value(int value)
{
    type = DataType::INT;
    intValue = value;
}

Value::Value(double value)
{
    type = DataType::DOUBLE;
    doubleValue = value;
}

Value::Value(bool value)
{
    type = DataType::BOOLEAN;
    boolValue = value;
}

Value::Value(const std::string& value)
{
    type = DataType::VARCHAR;
    stringValue = value;
}

DataType Value::getType() const
{
    return type;
}

int Value::asInt() const
{
    return intValue;
}

double Value::asDouble() const
{
    return doubleValue;
}

bool Value::asBool() const
{
    return boolValue;
}

std::string Value::asString() const
{
    return stringValue;
}