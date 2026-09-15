// SQLLexer — разбивает исходную строку SQL на последовательность токенов.
#pragma once

#include <string>
#include <vector>

namespace minidb {

enum class TokenType {
    KEYWORD, IDENTIFIER, NUMBER, STRING,
    LPAREN, RPAREN, COMMA, SEMICOLON,
    OP_EQ, OP_LT, OP_GT, OP_LE, OP_GE, OP_NE,
    STAR, END_OF_INPUT
};

struct Token {
    TokenType type;
    std::string text;
};

class SQLLexer {
public:
    explicit SQLLexer(const std::string& input);

    // Возвращает весь список токенов из исходной строки (включая завершающий END_OF_INPUT)
    std::vector<Token> Tokenize();

private:
    char Peek() const;
    char Advance();
    bool IsAtEnd() const;
    void SkipWhitespace();

    std::string input_;
    size_t pos_ = 0;
};

}  // namespace minidb
