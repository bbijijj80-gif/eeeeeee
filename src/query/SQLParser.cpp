#include "SQLParser.h"

#include "../../include/macros.h"

namespace minidb {

SQLParser::SQLParser(const std::string& sql) {
    SQLLexer lexer(sql);
    tokens_ = lexer.Tokenize();
}

const Token& SQLParser::Peek() const { return tokens_[pos_]; }
const Token& SQLParser::Advance() { return tokens_[pos_++]; }
bool SQLParser::Check(TokenType type) const { return Peek().type == type; }
bool SQLParser::CheckKeyword(const std::string& kw) const {
    return Peek().type == TokenType::KEYWORD && Peek().text == kw;
}

const Token& SQLParser::Expect(TokenType type, const std::string& err_msg) {
    MINIDB_ASSERT(Check(type), "SQLParser: " + err_msg + " (получено: '" + Peek().text + "')");
    return Advance();
}

void SQLParser::ExpectKeyword(const std::string& kw) {
    MINIDB_ASSERT(CheckKeyword(kw), "SQLParser: ожидалось ключевое слово " + kw + ", получено '" + Peek().text + "'");
    Advance();
}

std::unique_ptr<Statement> SQLParser::Parse() {
    if (CheckKeyword("CREATE")) return ParseCreateTable();
    if (CheckKeyword("INSERT")) return ParseInsert();
    if (CheckKeyword("SELECT")) return ParseSelect();
    MINIDB_THROW("SQLParser: неизвестная команда SQL, начинается с '" + Peek().text + "'");
}

ColumnType SQLParser::ParseColumnType() {
    MINIDB_ASSERT(Check(TokenType::KEYWORD), "ожидался тип столбца");
    std::string t = Advance().text;
    if (t == "VARCHAR" || t == "TEXT") {
        if (Check(TokenType::LPAREN)) {  // необязательная длина: VARCHAR(255)
            Advance();
            Expect(TokenType::NUMBER, "ожидалось число в объявлении длины VARCHAR");
            Expect(TokenType::RPAREN, "ожидалась закрывающая скобка");
        }
        return ColumnType::VARCHAR;
    }
    return ColumnType::INTEGER;  // INT / INTEGER
}

std::unique_ptr<Statement> SQLParser::ParseCreateTable() {
    ExpectKeyword("CREATE");
    ExpectKeyword("TABLE");

    auto stmt = std::make_unique<CreateTableStatement>();
    stmt->table_name = Expect(TokenType::IDENTIFIER, "ожидалось имя таблицы").text;

    Expect(TokenType::LPAREN, "ожидалась '(' после имени таблицы");
    while (!Check(TokenType::RPAREN)) {
        ColumnDef col;
        col.name = Expect(TokenType::IDENTIFIER, "ожидалось имя столбца").text;
        col.type = ParseColumnType();
        stmt->columns.push_back(col);
        if (Check(TokenType::COMMA)) Advance();
    }
    Expect(TokenType::RPAREN, "ожидалась ')' в конце списка столбцов");
    return stmt;
}

Value SQLParser::ParseLiteral() {
    if (Check(TokenType::NUMBER)) {
        return Value::MakeInt(std::stoll(Advance().text));
    }
    if (Check(TokenType::STRING)) {
        return Value::MakeStr(Advance().text);
    }
    MINIDB_THROW("SQLParser: ожидался литерал (число или строка), получено '" + Peek().text + "'");
}

std::unique_ptr<Statement> SQLParser::ParseInsert() {
    ExpectKeyword("INSERT");
    ExpectKeyword("INTO");

    auto stmt = std::make_unique<InsertStatement>();
    stmt->table_name = Expect(TokenType::IDENTIFIER, "ожидалось имя таблицы").text;

    ExpectKeyword("VALUES");
    Expect(TokenType::LPAREN, "ожидалась '(' перед списком значений");
    while (!Check(TokenType::RPAREN)) {
        stmt->values.push_back(ParseLiteral());
        if (Check(TokenType::COMMA)) Advance();
    }
    Expect(TokenType::RPAREN, "ожидалась ')' в конце списка значений");
    return stmt;
}

std::unique_ptr<Statement> SQLParser::ParseSelect() {
    ExpectKeyword("SELECT");

    auto stmt = std::make_unique<SelectStatement>();
    if (Check(TokenType::STAR)) {
        Advance();
        stmt->columns.push_back("*");
    } else {
        stmt->columns.push_back(Expect(TokenType::IDENTIFIER, "ожидалось имя столбца").text);
        while (Check(TokenType::COMMA)) {
            Advance();
            stmt->columns.push_back(Expect(TokenType::IDENTIFIER, "ожидалось имя столбца").text);
        }
    }

    ExpectKeyword("FROM");
    stmt->table_name = Expect(TokenType::IDENTIFIER, "ожидалось имя таблицы").text;

    if (CheckKeyword("WHERE")) {
        Advance();
        stmt->has_where = true;
        stmt->where.column = Expect(TokenType::IDENTIFIER, "ожидалось имя столбца в WHERE").text;

        TokenType op_tok = Peek().type;
        CompareOp op;
        switch (op_tok) {
            case TokenType::OP_EQ: op = CompareOp::EQ; break;
            case TokenType::OP_LT: op = CompareOp::LT; break;
            case TokenType::OP_GT: op = CompareOp::GT; break;
            case TokenType::OP_LE: op = CompareOp::LE; break;
            case TokenType::OP_GE: op = CompareOp::GE; break;
            case TokenType::OP_NE: op = CompareOp::NE; break;
            default: MINIDB_THROW("SQLParser: ожидался оператор сравнения в WHERE");
        }
        Advance();
        stmt->where.op = op;
        stmt->where.value = ParseLiteral();
    }

    return stmt;
}

}  // namespace minidb
