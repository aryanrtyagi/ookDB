#include "schema.h"

void Schema::addColumn(
    const std::string& name,
    DataType type
)
{
    columns.emplace_back(name, type);
}

const Column& Schema::getColumn(int index) const
{
    return columns.at(index);
}

int Schema::size() const
{
    return static_cast<int>(columns.size());
}

int Schema::getColumnIndex(
    const std::string& name
) const
{
    for (int i = 0; i < columns.size(); i++)
    {
        if (columns[i].getName() == name)
        {
            return i;
        }
    }

    return -1;
}