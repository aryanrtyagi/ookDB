#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <string>
#include <memory>
#include <vector>

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
    LESS_EQUAL,
    IN,
    NOT_IN,
    BETWEEN,
    NOT_BETWEEN,
    LIKE,
    NOT_LIKE
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
    LiteralType type = LiteralType::STRING;
    std::string value;
};

enum class ConditionType
{
    COMPARISON,
    AND,
    OR,
    NOT,
    IN,
    BETWEEN,
    LIKE
};

// ============================================================
// Condition
// ============================================================

struct Condition
{
    ConditionType type = ConditionType::COMPARISON;

    std::string column;
    ComparisonOperator op = ComparisonOperator::EQUAL;

    Literal value;
    Literal secondValue;

    std::vector<Literal> values;

    bool negated = false;

    std::shared_ptr<Condition> left;
    std::shared_ptr<Condition> right;
};

#endif