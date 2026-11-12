
#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <string>

#include "../lexer/lexer.h"
#include "../ast/query.h"

class Parser
{
private:
    std::vector<Token> tokens;
    std::size_t position = 0;

    const Token& currentToken() const;
    bool check(TokenType type) const;
    Token consume();
    bool match(TokenType type);
    Token expect(TokenType type);

    std::string parseIdentifier();
    Literal parseLiteral();
    std::string parseQualifiedIdentifier();

    Condition parseComparisonCondition();
    Condition parsePrimaryExpression();
    Condition parseNotExpression();
    Condition parseAndExpression();
    Condition parseOrExpression();
    Condition parseCondition();

    std::vector<std::string> parseColumnList();
    std::vector<Literal> parseLiteralList();

    SelectQuery parseSelect();
    CreateTableQuery parseCreateTable();
    CreateIndexQuery parseCreateIndex();
    InsertQuery parseInsert();
    UpdateQuery parseUpdate();
    DeleteQuery parseDelete();

    DescribeQuery parseDescribe();
    DropTableQuery parseDropTable();
    AlterTableQuery parseAlterTable();

public:
    explicit Parser(const std::vector<Token>& tokens);
    Query parse();
};

#endif
