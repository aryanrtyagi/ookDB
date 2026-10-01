#ifndef COLUMN_H
#define COLUMN_H

#include "value.h"

#include <string>

class Column
{
private:
    std::string name;
    DataType type;

public:
    Column(
        const std::string& name,
        DataType type
    );

    const std::string& getName() const;

    DataType getType() const;
};

#endif