#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <string>

// ============================================================
// Comparison Operators
// ============================================================

enum class ComparisonOperator
{
    EQUAL,
    NOT_EQUAL,
    GREATER,
    LESS,
    GREATER_EQUAL,
    LESS_EQUAL
};


// ============================================================
// Literal Types
// ============================================================

enum class LiteralType
{
    NUMBER,
    STRING
};


// ============================================================
// Literal
// ============================================================

struct Literal
{
    LiteralType type;
    std::string value;
};


// ============================================================
// Condition
// ============================================================

struct Condition
{
    std::string column;

    ComparisonOperator op;

    Literal value;
};

#endif