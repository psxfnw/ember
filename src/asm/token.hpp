#pragma once
#include <string>
struct Token {
    enum class Kind { Ident, Number, Comma, Colon, LBracket, RBracket, Newline, Eof, Dot };
    Kind kind;
    std::string text;
    int line;
    Token* next = nullptr;
};
void push_back(Token*& head, Token*& tail, Token::Kind kind, const std::string& text, int line);
void destroy(Token* head);
const char* kind_name(Token::Kind kind);
