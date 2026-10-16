#include "token.hpp"
void push_back(Token*& head, Token*& tail, Token::Kind kind, const std::string& text, int line) {
    auto* t = new Token{kind, text, line, nullptr};
    if (!head)
        head = tail = t;
    else {
        tail->next = t;
        tail = t;
    }
}
const char* kind_name(Token::Kind kind) {
    switch (kind) {
    case Token::Kind::Ident:
        return "Ident";
    case Token::Kind::Number:
        return "Number";
    case Token::Kind::Comma:
        return "Comma";
    case Token::Kind::Colon:
        return "Colon";
    case Token::Kind::LBracket:
        return "LBracket";
    case Token::Kind::RBracket:
        return "RBracket";
    case Token::Kind::Newline:
        return "Newline";
    case Token::Kind::Eof:
        return "Eof";
    case Token::Kind::Dot:
        return "Dot";
    }
    return "?";
}
