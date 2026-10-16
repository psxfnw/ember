#include "frontend.hpp"
#include "assembler.hpp"
#include "lexer.hpp"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
static void load_image(CPU& cpu, Memory& mem, const std::vector<Byte>& bytes) {
    Byte discarded = 0;
    while (cpu.stack.pop(mem, discarded)) {
    }
    mem = Memory{};
    cpu.mem = &mem;
    cpu.pc = 0;
    cpu.ticks = 0;
    cpu.heap_ptr = HEAP_LO;
    cpu.h = 0;
    cpu.a = 0;
    cpu.b = 0;
    cpu.f = Flags{};
    cpu.faulted = false;
    cpu.breakpoint = -1;
    cpu.halted = false;
    for (std::size_t i = 0; i < bytes.size(); ++i)
        mem_set(mem, i, bytes[i]);
}
int file_command(const std::string& action, const std::string& path, CPU& cpu, Memory& mem,
                 bool interactive) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        std::cerr << "cannot open file: " << path << '\n';
        return 1;
    }
    const std::string source((std::istreambuf_iterator<char>(file)),
                             std::istreambuf_iterator<char>());
    std::string error;
    if (action == "lex") {
        Token* head = lex(source, error);
        if (!head) {
            std::cerr << error << '\n';
            return 1;
        }
        for (const Token* t = head; t; t = t->next)
            std::cout << t->line << ' ' << kind_name(t->kind) << " \"" << t->text << "\"\n";
        destroy(head);
        return 0;
    }
    std::vector<Byte> bytes;
    const bool raw =
        action == "disasm" && (path.size() < 4 || path.substr(path.size() - 4) != ".asm");
    if (raw)
        bytes.assign(source.begin(), source.end());
    else if (!assemble(source, bytes, error)) {
        std::cerr << error << '\n';
        return 1;
    }
    if (action == "disasm") {
        std::string listing;
        if (!disassemble(bytes, listing, error)) {
            std::cerr << error << '\n';
            return 1;
        }
        std::cout << listing;
        return 0;
    }
    if (action == "asm") {
        for (std::size_t i = 0; i < bytes.size(); ++i)
            std::cout << std::hex << std::setfill('0') << std::setw(2) << unsigned(bytes[i])
                      << ((i + 1) % 16 == 0 || i + 1 == bytes.size() ? '\n' : ' ');
        std::cout << std::dec << std::setfill(' ');
        if (interactive) {
            load_image(cpu, mem, bytes);
            std::cout << "loaded " << bytes.size() << " bytes at 0\n";
        }
        return 0;
    }
    load_image(cpu, mem, bytes);
    run(cpu);
    return cpu.faulted || !cpu.halted ? 1 : 0;
}
