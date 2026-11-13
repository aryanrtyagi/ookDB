#include "value.h"

#include <stdexcept>

// --------------------------------------------------
// Default constructor: SQL NULL
// --------------------------------------------------

Value::Value()
    : type(DataType::NULL_VALUE),
      intValue(0),
      doubleValue(0.0),
      boolValue(false),
      stringValue("")
{
}

// --------------------------------------------------
// Constructors for regular values
// --------------------------------------------------

Value::Value(int value)
    : type(DataType::INT),
      intValue(value),
      doubleValue(0.0),
      boolValue(false),
      stringValue("")
{
}

Value::Value(double value)
    : type(DataType::DOUBLE),
      intValue(0),
      doubleValue(value),
      boolValue(false),
      stringValue("")
{
}

Value::Value(bool value)
    : type(DataType::BOOLEAN),
      intValue(0),
      doubleValue(0.0),
      boolValue(value),
      stringValue("")
{
}

Value::Value(const std::string& value)
    : type(DataType::VARCHAR),
      intValue(0),
      doubleValue(0.0),
      boolValue(false),
      stringValue(value)
{
}

Value::Value(const char* value)
    : type(DataType::VARCHAR),
      intValue(0),
      doubleValue(0.0),
      boolValue(false),
      stringValue(value)
{
}

// --------------------------------------------------
// NULL factory
// --------------------------------------------------

Value Value::null()
{
    return Value();
}

// --------------------------------------------------
// Type information
// --------------------------------------------------

DataType Value::getType() const
{
    return type;
}

bool Value::isNull() const
{
    return type == DataType::NULL_VALUE;
}

// --------------------------------------------------
// Typed accessors
// --------------------------------------------------

int Value::asInt() const
{
    if (isNull())
    {
        throw std::runtime_error(
            "Cannot read NULL as INT"
        );
    }

    return intValue;
}

double Value::asDouble() const
{
    if (isNull())
    {
        throw std::runtime_error(
            "Cannot read NULL as DOUBLE"
        );
    }

    return doubleValue;
}

bool Value::asBool() const
{
    if (isNull())
    {
        throw std::runtime_error(
            "Cannot read NULL as BOOLEAN"
        );
    }

    return boolValue;
}

std::string Value::asString() const
{
    if (isNull())
    {
        throw std::runtime_error(
            "Cannot read NULL as VARCHAR"
        );
    }

    return stringValue;
}