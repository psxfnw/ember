#include "assembler.hpp"
#include "lexer.hpp"
#include <cctype>
#include <map>
#include <sstream>

// # — єдине числове поле; решта символів точно задає форму операндів ISA.
const std::vector<Instruction>& instruction_set() {
    static const std::vector<Instruction> table = {{Op::Halt, "HALT", ""},
                                                   {Op::Nop, "NOP", ""},
                                                   {Op::Out, "OUT", ""},
                                                   {Op::OutN, "OUTN", ""},
                                                   {Op::OutS, "OUTS", "#"},
                                                   {Op::Add, "ADD", "A,B"},
                                                   {Op::Sub, "SUB", "A,B"},
                                                   {Op::And, "AND", "A,B"},
                                                   {Op::Or, "OR", "A,B"},
                                                   {Op::Xor, "XOR", "A,B"},
                                                   {Op::Not, "NOT", "A"},
                                                   {Op::Shl, "SHL", "A"},
                                                   {Op::Shr, "SHR", "A"},
                                                   {Op::Inc, "INC", "A"},
                                                   {Op::Dec, "DEC", "A"},
                                                   {Op::Cmp, "CMP", "A,B"},
                                                   {Op::Mul, "MUL", "A,B"},
                                                   {Op::LoadiA, "LOADI", "A,#"},
                                                   {Op::LoadiB, "LOADI", "B,#"},
                                                   {Op::LoadIndirect, "LOAD", "A,[H]"},
                                                   {Op::LoadA, "LOAD", "A,[#]"},
                                                   {Op::LoadB, "LOAD", "B,[#]"},
                                                   {Op::StoreIndirect, "STORE", "[H],A"},
                                                   {Op::StoreA, "STORE", "[#],A"},
                                                   {Op::StoreB, "STORE", "[#],B"},
                                                   {Op::MovAB, "MOV", "A,B"},
                                                   {Op::MovBA, "MOV", "B,A"},
                                                   {Op::LoadH, "LOADH", "H,#"},

                                                   {Op::IncH, "INCH", ""},
                                                   {Op::DecH, "DECH", ""},
                                                   {Op::HLow, "HLOW", ""},
                                                   {Op::Jmp, "JMP", "#"},
                                                   {Op::Jz, "JZ", "#"},
                                                   {Op::Jnz, "JNZ", "#"},
                                                   {Op::Jc, "JC", "#"},
                                                   {Op::Jnc, "JNC", "#"},
                                                   {Op::Cls, "CLS", ""},
                                                   {Op::Plot, "PLOT", ""},
                                                   {Op::Unplot, "UNPLOT", ""},
                                                   {Op::Show, "SHOW", ""},
                                                   {Op::PushA, "PUSH", "A"},
                                                   {Op::PopA, "POP", "A"},
                                                   {Op::PushB, "PUSH", "B"},
                                                   {Op::PopB, "POP", "B"},
                                                   {Op::Call, "CALL", "#"},
                                                   {Op::Ret, "RET", ""},
                                                   {Op::PushH, "PUSH", "H"},
                                                   {Op::PopH, "POP", "H"},
                                                   {Op::Alloc, "ALLOC", ""}};
    return table;
}
static std::string upper(std::string text) {
    for (char& c : text)
        c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    return text;
}
static bool end_line(const Token* t) {
    return t->kind == Token::Kind::Newline || t->kind == Token::Kind::Eof;
}
static bool error_at(std::string& error, int line, const std::string& message) {
    error = "line " + std::to_string(line) + ": " + message;
    return false;
}
static bool literal(const std::string& text, unsigned& out) {
    unsigned base = 10;
    std::size_t i = 0;
    out = 0;
    if (text.size() > 2 && text[0] == '0' && (text[1] == 'x' || text[1] == 'X')) {
        base = 16;
        i = 2;
    } else if (text.size() > 2 && text[0] == '0' && (text[1] == 'b' || text[1] == 'B')) {
        base = 2;
        i = 2;
    }
    for (; i < text.size(); ++i) {
        const char c = text[i];
        const unsigned digit = c >= '0' && c <= '9'   ? c - '0'
                               : c >= 'a' && c <= 'f' ? c - 'a' + 10
                                                      : c - 'A' + 10;
        if (out > (65535 - digit) / base)
            return false;
        out = out * base + digit;
    }
    return true;
}
static bool matches(const char* pattern, const Token* token, const Token*& operand) {
    operand = nullptr;
    for (const char* p = pattern; *p; ++p) {
        if (end_line(token))
            return false;
        if (*p == '#') {
            if (token->kind != Token::Kind::Number && token->kind != Token::Kind::Ident)
                return false;
            operand = token;
        } else if (*p == 'A' || *p == 'B' || *p == 'H') {
            if (token->kind != Token::Kind::Ident || upper(token->text) != std::string(1, *p))
                return false;
        } else if (token->text != std::string(1, *p))
            return false;
        token = token->next;
    }
    return end_line(token);
}
struct TokenOwner {
    Token* head;
    ~TokenOwner() { destroy(head); }
};
bool assemble(const std::string& source, std::vector<Byte>& bytes, std::string& error) {
    bytes.clear();
    error.clear();
    TokenOwner tokens{lex(source, error)};
    if (!tokens.head)
        return false;
    std::map<std::string, unsigned> symbols;
    std::vector<Byte> result;
    for (int pass = 1; pass <= 2; ++pass) {
        std::size_t pc = 0;
        for (const Token* t = tokens.head; t->kind != Token::Kind::Eof;) {
            if (t->kind == Token::Kind::Newline) {
                t = t->next;
                continue;
            }
            const int line = t->line;
            if (t->kind == Token::Kind::Ident && t->next->kind == Token::Kind::Colon) {
                if (pass == 1 && !symbols.emplace(t->text, static_cast<unsigned>(pc)).second)
                    return error_at(error, line, "duplicate symbol: " + t->text);
                t = t->next->next;
                if (end_line(t))
                    continue;
            }
            if (t->kind == Token::Kind::Dot) {
                t = t->next;
                if (t->kind != Token::Kind::Ident || t->text != "define")
                    return error_at(error, line, "expected .define");
                t = t->next;
                if (t->kind != Token::Kind::Ident)
                    return error_at(error, line, "expected macro name");
                const std::string name = t->text;
                t = t->next;
                if (t->kind != Token::Kind::Number)
                    return error_at(error, line, ".define needs a numeric literal");
                unsigned value = 0;
                if (!literal(t->text, value))
                    return error_at(error, line, "number outside 0..65535");
                t = t->next;
                if (!end_line(t))
                    return error_at(error, line, "extra tokens after .define");
                if (pass == 1 && !symbols.emplace(name, value).second)
                    return error_at(error, line, "duplicate symbol: " + name);
                continue;
            }
            if (t->kind != Token::Kind::Ident)
                return error_at(error, line, "expected mnemonic");
            const std::string mnemonic = upper(t->text);
            t = t->next;
            const Instruction* instruction = nullptr;
            const Token* operand = nullptr;
            bool known = false;
            for (const auto& entry : instruction_set()) {
                if (mnemonic != entry.name)
                    continue;
                known = true;
                if (matches(entry.pattern, t, operand)) {
                    instruction = &entry;
                    break;
                }
            }
            if (!instruction)
                return error_at(error, line,
                                known ? "bad operands for " + mnemonic
                                      : "unknown mnemonic: " + mnemonic);
            const std::size_t size = instruction_size(static_cast<Byte>(instruction->op));
            if (pc + size > 0x800)
                return error_at(error, line, "code exceeds 2048-byte region");
            if (pass == 2) {
                unsigned value = 0;
                if (operand) {
                    if (operand->kind == Token::Kind::Number) {
                        if (!literal(operand->text, value))
                            return error_at(error, line, "number outside 0..65535");
                    } else {
                        const auto found = symbols.find(operand->text);
                        if (found == symbols.end())
                            return error_at(error, line, "unknown symbol: " + operand->text);
                        value = found->second;
                    }
                    if (size == 2 && value > 255)
                        return error_at(error, line, "byte operand outside 0..255");
                }
                result.push_back(static_cast<Byte>(instruction->op));
                if (size >= 2)
                    result.push_back(static_cast<Byte>(value & 255));
                if (size == 3)
                    result.push_back(static_cast<Byte>(value >> 8));
            }
            pc += size;
            while (!end_line(t))
                t = t->next;
        }
    }
    bytes = std::move(result);
    return true;
}
bool disassemble(const std::vector<Byte>& bytes, std::string& source, std::string& error) {
    source.clear();
    error.clear();
    std::ostringstream out;
    for (std::size_t pc = 0; pc < bytes.size();) {
        const Instruction* instruction = nullptr;
        for (const auto& entry : instruction_set())
            if (static_cast<Byte>(entry.op) == bytes[pc]) {
                instruction = &entry;
                break;
            }
        if (!instruction) {
            error = "byte " + std::to_string(pc) + ": unknown opcode";
            return false;
        }
        const auto size = instruction_size(bytes[pc]);
        if (size > bytes.size() - pc) {
            error = "byte " + std::to_string(pc) + ": truncated instruction";
            return false;
        }
        unsigned value = 0;
        if (size >= 2)
            value = bytes[pc + 1];
        if (size == 3)
            value |= static_cast<unsigned>(bytes[pc + 2]) << 8;
        out << instruction->name;
        if (*instruction->pattern)
            out << ' ';
        for (const char* p = instruction->pattern; *p; ++p) {
            if (*p == '#')
                out << value;
            else if (*p == ',')
                out << ", ";
            else
                out << *p;
        }
        out << '\n';
        pc += size;
    }
    source = out.str();
    return true;
}
