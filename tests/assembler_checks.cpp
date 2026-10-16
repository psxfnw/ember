#include "asm/assembler.hpp"
#include "asm/lexer.hpp"
#include <cassert>
#include <iostream>
#include <set>
static std::vector<Byte> compiled(const std::string& source) {
    std::vector<Byte> bytes;
    std::string error;
    const bool ok = assemble(source, bytes, error);
    if (!ok)
        std::cerr << error << '\n';
    assert(ok);
    return bytes;
}
int main() {
    assert(compiled("HALT") == std::vector<Byte>{0});
    assert((compiled("JMP 0x0123\n") == std::vector<Byte>{0x30, 0x23, 1}));
    assert((compiled("JMP done\nNOP\ndone: HALT") == std::vector<Byte>{0x30, 4, 0, 1, 0}));
    assert((compiled("LOAD A, [H]\nSTORE [H], A\n") == std::vector<Byte>{0x29, 0x2A}));
    assert((compiled(".define N 0b1000001\nLOADI A,N\nOUT\n") == std::vector<Byte>{0x20, 65, 2}));
    assert((compiled("LOADI A,N\n.define N 65\n") == std::vector<Byte>{0x20, 65}));
    std::set<unsigned> opcodes;
    for (const auto& ins : instruction_set()) {
        const auto opcode = static_cast<Byte>(ins.op);
        assert(opcodes.insert(opcode).second);
        const auto size = instruction_size(opcode);
        std::string line = std::string(ins.name) + " ";
        for (const char* p = ins.pattern; *p; ++p)
            line += *p == '#' ? (size == 2 ? "0xA5" : "0x1234") : std::string(1, *p);
        line += '\n';
        auto bytes = compiled(line);
        assert(bytes.size() == size && bytes[0] == opcode);
        if (size >= 2)
            assert(bytes[1] == (size == 2 ? 0xA5 : 0x34));
        if (size == 3)
            assert(bytes[2] == 0x12);
        std::string listing, error;
        assert(disassemble(bytes, listing, error));
        assert(compiled(listing) == bytes);
    }
    assert(opcodes.size() == 49);
    for (const auto& bad : {"0xGG", "0x", "12ab", "0b102", "0b", "0x1_0", "123_", "999A"}) {
        std::string error;
        Token* tokens = lex(std::string("\nLOADI A,") + bad, error);
        assert(!tokens && error.find("line 2: bad number") != std::string::npos);
    }
    for (const auto& source :
         {"LOADI A 3", "LOADI A,256", "LOADH H,65536", "JMP nowhere", "abc: HALT\nabc: NOP", "CALL",
          "STORE [H], B", "LOAD B,[H]", "JMP 1 2", "HALT NOP", ".define X 1\n.define X 2",
          ".define X nope", ".define X 1 2", "12: HALT", "ADD B,A", "HALT\n@",
          "LOADI A,9999999999999999999999999999999999999999"}) {
        std::vector<Byte> bytes{99};
        std::string error;
        assert(!assemble(source, bytes, error) && bytes.empty() && error.find("line ") == 0);
    }
    std::string maximum;
    for (int i = 0; i < 2048; ++i)
        maximum += "NOP\n";
    assert(compiled(maximum).size() == 2048);
    std::vector<Byte> bytes;
    std::string error;
    assert(!assemble(maximum + "HALT", bytes, error));
    std::string listing;
    assert(!disassemble({0xFF}, listing, error));
    assert(!disassemble({0x20}, listing, error));
    // Many successful and failed lists: LeakSanitizer checks both ownership paths.
    for (int i = 0; i < 1000; ++i) {
        Token* head = lex("ADD A, B\n", error);
        assert(head);
        const Token::Kind expected[] = {Token::Kind::Ident,   Token::Kind::Ident,
                                        Token::Kind::Comma,   Token::Kind::Ident,
                                        Token::Kind::Newline, Token::Kind::Eof};
        const Token* t = head;
        for (auto kind : expected) {
            assert(t && t->kind == kind);
            t = t->next;
        }
        assert(!t);
        destroy(head);
        assert(!lex("ADD A, B\n0xGG", error));
        assert(!assemble("LOADI A,1\nJMP missing", bytes, error));
    }
    std::cout << "PASS: 49 ISA forms, shared sizes, LE16, forward labels/macros, all-opcode round "
                 "trips, bounds and token lifetime\n";
}
