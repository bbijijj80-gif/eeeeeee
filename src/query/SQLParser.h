// SQLParser — рекурсивный нисходящий парсер, строящий AST из потока токенов SQLLexer.
// Поддерживает подмножество SQL: CREATE TABLE, INSERT INTO ... VALUES, SELECT ... FROM ... [WHERE].
#pragma once

#include <memory>
#include <vector>

#include "ASTNodes.h"
#include "SQLLexer.h"

namespace minidb {

class SQLParser {
public:
    explicit SQLParser(const std::string& sql);

    // Разбирает один SQL-запрос и возвращает соответствующий узел Statement
    std::unique_ptr<Statement> Parse();

private:
    const Token& Peek() const;
    const Token& Advance();
    bool Check(TokenType type) const;
    bool CheckKeyword(const std::string& kw) const;
    const Token& Expect(TokenType type, const std::string& err_msg);
    void ExpectKeyword(const std::string& kw);

    std::unique_ptr<Statement> ParseCreateTable();
    std::unique_ptr<Statement> ParseInsert();
    std::unique_ptr<Statement> ParseSelect();
    Value ParseLiteral();
    ColumnType ParseColumnType();

    std::vector<Token> tokens_;
    size_t pos_ = 0;
};

}  // namespace minidb
