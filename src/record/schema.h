#ifndef SCHEMA_H
#define SCHEMA_H

#include "column.h"

#include <vector>

class Schema
{
private:
    std::vector<Column> columns;

public:
    void addColumn(
        const std::string& name,
        DataType type
    );

    const Column& getColumn(int index) const;

    int columnCount() const;
};

#endif