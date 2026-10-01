#ifndef ROW_H
#define ROW_H

#include "value.h"

#include <vector>

class Row
{
private:
    std::vector<Value> values;

public:
    void addValue(const Value& value);

    const Value& getValue(int index) const;

    int size() const;
};

#endif