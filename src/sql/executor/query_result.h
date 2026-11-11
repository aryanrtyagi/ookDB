#ifndef QUERY_RESULT_H
#define QUERY_RESULT_H

#include <string>
#include <vector>

#include "../../record/value.h"

struct QueryResult
{
    // Column names returned by SELECT.
    std::vector<std::string> columns;

    // Actual rows containing typed Values.
    std::vector<std::vector<Value>> rows;

    // Number of rows affected by INSERT, UPDATE or DELETE.
    int affectedRows = 0;

    // Human-readable execution message.
    std::string message;

    // Indicates whether execution succeeded.
    bool success = true;
};

#endif
