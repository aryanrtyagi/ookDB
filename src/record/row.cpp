#include "row.h"

void Row::addValue(const Value& value)
{
    values.push_back(value);
}

const Value& Row::getValue(int index) const
{
    return values.at(index);
}

int Row::size() const
{
    return static_cast<int>(values.size());
}