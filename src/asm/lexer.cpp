// Основа циклу — офіційний каркас Lab 08; розпізнавання чисел доповнено.
#include "lexer.hpp"

#include <cctype>

// Стан лексера: текст, курсор, номер рядка і список, який ми будуємо.
struct Lexer {
    const std::string& src;
    std::size_t i = 0;
    int line = 1;
    Token* head = nullptr;
    Token* tail = nullptr;
    std::string error; // непорожній, щойно лексер щось відхилив
};

// Додати вузол у кінець списку (Notes 08 §3).
static void emit(Lexer& lx, Token::Kind kind, const std::string& text) {
    push_back(lx.head, lx.tail, kind, text, lx.line);
}

// Записати помилку з номером рядка. Повертає false, щоб зручно писати
// `return fail(lx, "...");`
static bool fail(Lexer& lx, const std::string& msg) {
    lx.error = "line " + std::to_string(lx.line) + ": " + msg;
    return false;
}

void destroy(Token* head) {
    while (head) {
        Token* next = head->next;
        delete head;
        head = next;
    }
}

// З'їдаємо всю можливу числову лексему, щоб 12ab не стало двома токенами.
static bool number(Lexer& lx) {
    const std::size_t start = lx.i;
    while (lx.i < lx.src.size() &&
           (std::isalnum(static_cast<unsigned char>(lx.src[lx.i])) || lx.src[lx.i] == '_'))
        ++lx.i;
    const std::string word = lx.src.substr(start, lx.i - start);
    unsigned base = 10;
    std::size_t pos = 0;
    if (word.size() >= 2 && word[0] == '0' && (word[1] == 'x' || word[1] == 'X')) {
        base = 16;
        pos = 2;
    } else if (word.size() >= 2 && word[0] == '0' && (word[1] == 'b' || word[1] == 'B')) {
        base = 2;
        pos = 2;
    }
    if (pos == word.size())
        return fail(lx, "bad number: " + word);
    for (; pos < word.size(); ++pos) {
        const char c = word[pos];
        const unsigned digit = c >= '0' && c <= '9'   ? c - '0'
                               : c >= 'a' && c <= 'f' ? c - 'a' + 10
                               : c >= 'A' && c <= 'F' ? c - 'A' + 10
                                                      : 99;
        if (digit >= base)
            return fail(lx, "bad number: " + word);
    }
    emit(lx, Token::Kind::Number, word);
    return true;
}

Token* lex(const std::string& src, std::string& error) {
    error.clear();
    Lexer lx{src, 0, 1, nullptr, nullptr, ""};
    while (lx.i < src.size()) {
        char c = src[lx.i];
        if (c == ' ' || c == '\t' || c == '\r') {
            ++lx.i; // пробіли не токени
        } else if (c == ';') {
            while (lx.i < src.size() && src[lx.i] != '\n')
                ++lx.i; // коментар
        } else if (c == '\n') {
            emit(lx, Token::Kind::Newline, "\\n");
            ++lx.line;
            ++lx.i;
        } else if (std::isalpha((unsigned char)c) || c == '_') {
            std::size_t start = lx.i;
            while (lx.i < src.size() &&
                   (std::isalnum((unsigned char)src[lx.i]) || src[lx.i] == '_')) {
                ++lx.i;
            }
            emit(lx, Token::Kind::Ident, src.substr(start, lx.i - start));
        } else if (std::isdigit((unsigned char)c)) {
            if (!number(lx))
                break;
        } else if (c == ',') {
            emit(lx, Token::Kind::Comma, ",");
            ++lx.i;
        } else if (c == ':') {
            emit(lx, Token::Kind::Colon, ":");
            ++lx.i;
        } else if (c == '[') {
            emit(lx, Token::Kind::LBracket, "[");
            ++lx.i;
        } else if (c == ']') {
            emit(lx, Token::Kind::RBracket, "]");
            ++lx.i;
        } else if (c == '.') {
            emit(lx, Token::Kind::Dot, ".");
            ++lx.i;
        } else {
            fail(lx, std::string("unexpected character '") + c + "'");
            break;
        }
    }
    if (!lx.error.empty()) {
        destroy(lx.head);
        error = lx.error;
        return nullptr;
    }
    emit(lx, Token::Kind::Eof, "");
    return lx.head;
}
