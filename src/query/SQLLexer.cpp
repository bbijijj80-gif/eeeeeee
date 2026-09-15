#include "SQLLexer.h"

#include <cctype>
#include <unordered_set>

#include "../../include/macros.h"

namespace minidb {

namespace {
const std::unordered_set<std::string> kKeywords = {
    "CREATE", "TABLE", "INSERT", "INTO", "VALUES", "SELECT", "FROM", "WHERE",
    "INT", "INTEGER", "VARCHAR", "TEXT"
};

std::string ToUpper(const std::string& s) {
    std::string out = s;
    for (auto& c : out) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    return out;
}
}  // namespace

SQLLexer::SQLLexer(const std::string& input) : input_(input) {}

bool SQLLexer::IsAtEnd() const { return pos_ >= input_.size(); }
char SQLLexer::Peek() const { return IsAtEnd() ? '\0' : input_[pos_]; }
char SQLLexer::Advance() { return input_[pos_++]; }

void SQLLexer::SkipWhitespace() {
    while (!IsAtEnd() && std::isspace(static_cast<unsigned char>(Peek()))) Advance();
}

std::vector<Token> SQLLexer::Tokenize() {
    std::vector<Token> tokens;

    while (true) {
        SkipWhitespace();
        if (IsAtEnd()) {
            tokens.push_back({TokenType::END_OF_INPUT, ""});
            break;
        }

        char c = Peek();

        if (c == '(') { Advance(); tokens.push_back({TokenType::LPAREN, "("}); continue; }
        if (c == ')') { Advance(); tokens.push_back({TokenType::RPAREN, ")"}); continue; }
        if (c == ',') { Advance(); tokens.push_back({TokenType::COMMA, ","}); continue; }
        if (c == ';') { Advance(); tokens.push_back({TokenType::SEMICOLON, ";"}); continue; }
        if (c == '*') { Advance(); tokens.push_back({TokenType::STAR, "*"}); continue; }

        if (c == '=') { Advance(); tokens.push_back({TokenType::OP_EQ, "="}); continue; }
        if (c == '<') {
            Advance();
            if (Peek() == '=') { Advance(); tokens.push_back({TokenType::OP_LE, "<="}); }
            else if (Peek() == '>') { Advance(); tokens.push_back({TokenType::OP_NE, "<>"}); }
            else { tokens.push_back({TokenType::OP_LT, "<"}); }
            continue;
        }
        if (c == '>') {
            Advance();
            if (Peek() == '=') { Advance(); tokens.push_back({TokenType::OP_GE, ">="}); }
            else { tokens.push_back({TokenType::OP_GT, ">"}); }
            continue;
        }

        if (c == '\'') {
            Advance();
            std::string value;
            while (!IsAtEnd() && Peek() != '\'') value += Advance();
            MINIDB_ASSERT(!IsAtEnd(), "SQLLexer: незакрытая строковая константа");
            Advance();  // закрывающая кавычка
            tokens.push_back({TokenType::STRING, value});
            continue;
        }

        if (std::isdigit(static_cast<unsigned char>(c)) ||
            (c == '-' && pos_ + 1 < input_.size() && std::isdigit(static_cast<unsigned char>(input_[pos_ + 1])))) {
            std::string number;
            number += Advance();
            while (!IsAtEnd() && std::isdigit(static_cast<unsigned char>(Peek()))) number += Advance();
            tokens.push_back({TokenType::NUMBER, number});
            continue;
        }

        if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
            std::string ident;
            while (!IsAtEnd() && (std::isalnum(static_cast<unsigned char>(Peek())) || Peek() == '_')) ident += Advance();
            std::string upper = ToUpper(ident);
            if (kKeywords.count(upper)) {
                tokens.push_back({TokenType::KEYWORD, upper});
            } else {
                tokens.push_back({TokenType::IDENTIFIER, ident});
            }
            continue;
        }

        MINIDB_THROW(std::string("SQLLexer: неожиданный символ '") + c + "'");
    }
    return tokens;
}

}  // namespace minidb
