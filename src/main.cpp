// main.cpp — the prompt.
//
// GIVEN, in full. This file is scaffolding: it reads a line, splits it into
// words, and calls one of your functions. It uses `while`, `if` and functions,
// which the course only explains properly in Labs 4 and 7. That is on purpose —
// in Lab 1 you read this file, you do not write it.
//
// What you add in later labs is one more `else if` branch per command.
#include <iostream>
#include <sstream>
#include <string>

#include "cpu.hpp"
#include "dump.hpp"
#include "memory.hpp"

// Turn a word into a number. Accepts decimal (65) and hex (0x41).
// Returns false if the word is not a number at all.
static bool parse_number(const std::string& word, long& out) {
    try {
        std::size_t used = 0;
        // base 0 means: look at the prefix. "0x41" is hex, "65" is decimal.
        out = std::stol(word, &used, 0);
        return used == word.size(); // reject things like "12abc"
    } catch (...) {
        return false;
    }
}

static void print_help() {
    std::cout << "commands:\n"
              << "  dump              print all " << MEM_SIZE << " bytes\n"
              << "  get <addr>        show one byte four ways\n"
              << "  set <addr> <val>  write one byte (dec or 0x hex)\n"
              << "  inc <addr>        increment a byte modulo 256\n"
              << "  set16 <addr> <n>  write a little-endian 16-bit number\n"
              << "  reg <a|b> <n>     set an 8-bit register\n"
              << "  regs              show CPU state\n"
              << "  step              execute one instruction\n"
              << "  get16 <addr>      read a little-endian 16-bit value\n"
              << "  run               execute until HALT (100000 step limit)\n"
              << "  dumpcpu           inspect CPU object bytes on the host\n"
              << "  find <lo> <hi> <byte>  search [lo,hi)\n"
              << "  b <addr|off>      set/clear a breakpoint\n"
              << "  help              this list\n"
              << "  quit              leave\n";
}

int main() {
    CPU cpu;
    Memory mem; // 4096 bytes, on the stack, zeroed by the {} in memory.hpp

    cpu.mem = &mem;
    std::cout << "ember 0.4 - 4096 bytes of memory you can see. Type `help`.\n";

    std::string line;
    while (true) {
        std::cout << "ember> ";

        // getline reads one whole line. It returns false at end of input
        // (Ctrl-D), which is how the loop ends if you never type `quit`.
        if (!std::getline(std::cin, line)) {
            std::cout << '\n';
            break;
        }

        // Split the line into words: "set 0 65" -> cmd="set", args "0" and "65".
        std::istringstream words(line);
        std::string cmd;
        words >> cmd;

        if (cmd.empty()) {
            continue; // the user just pressed Enter
        } else if (cmd == "quit" || cmd == "exit") {
            break;
        } else if (cmd == "help") {
            print_help();
        } else if (cmd == "dump") {
            dump(mem);
        } else if (cmd == "get") {
            std::string a;
            long addr = 0;
            if (!(words >> a) || !parse_number(a, addr)) {
                std::cout << "usage: get <addr>\n";
            } else if (addr < 0) {
                std::cout << "address must not be negative\n";
            } else if (static_cast<std::size_t>(addr) >= MEM_SIZE) {
                std::cout << "address " << addr << " is outside 0.." << MEM_SIZE - 1 << '\n';
            } else {
                show_byte(mem_get(mem, (std::size_t)addr));
            }
        } else if (cmd == "set") {
            std::string a, v;
            long addr = 0, value = 0;
            if (!(words >> a) || !(words >> v) || !parse_number(a, addr) ||
                !parse_number(v, value)) {
                std::cout << "usage: set <addr> <value>\n";
            } else if (addr < 0) {
                std::cout << "address must not be negative\n";
            } else if (value < 0 || value > 255) {
                // A cell holds ONE byte. 256 does not fit. Lab 1, theory 3.
                std::cout << "a byte is 0..255, got " << value << '\n';
            } else if (!mem_set(mem, (std::size_t)addr, (Byte)value)) {
                std::cout << "address " << addr << " is outside 0.." << MEM_SIZE - 1 << '\n';
            }
        } else if (cmd == "reg") {
            std::string name, v;
            long value = 0;
            if (!(words >> name >> v) || !parse_number(v, value) || value < 0 || value > 65535)
                std::cout << "usage: reg <a|b|h|pc> <value>\n";
            else if ((name == "a" || name == "b") && value > 255)
                std::cout << "a byte is 0..255\n";
            else if (name == "a")
                cpu.a = static_cast<Byte>(value);
            else if (name == "b")
                cpu.b = static_cast<Byte>(value);
            else if (name == "h")
                cpu.h = static_cast<std::uint16_t>(value);
            else if (name == "pc")
                cpu.pc = static_cast<std::uint16_t>(value);
            else
                std::cout << "unknown register: " << name << '\n';
        } else if (cmd == "regs") {
            dump_regs(cpu);
        } else if (cmd == "get16") {
            std::string a;
            long addr = 0;
            if (!(words >> a) || !parse_number(a, addr) || addr < 0 ||
                static_cast<std::size_t>(addr) >= MEM_SIZE - 1)
                std::cout << "usage: get16 <0..4094>\n";
            else
                std::cout << get16(mem, static_cast<std::size_t>(addr)) << '\n';
        } else if (cmd == "b") {
            std::string value;
            long addr = 0;
            if (!(words >> value))
                std::cout << "usage: b <addr|off>\n";
            else if (value == "off")
                cpu.breakpoint = -1;
            else if (!parse_number(value, addr) || addr < 0 || addr >= static_cast<long>(MEM_SIZE))
                std::cout << "invalid breakpoint\n";
            else
                cpu.breakpoint = static_cast<int>(addr);
        } else if (cmd == "find") {
            std::string l, h, v;
            long lo = 0, hi = 0, value = 0;
            if (!(words >> l >> h >> v) || !parse_number(l, lo) || !parse_number(h, hi) ||
                !parse_number(v, value) || lo < 0 || hi < lo || hi > static_cast<long>(MEM_SIZE) ||
                value < 0 || value > 255)
                std::cout << "usage: find <0<=lo<=hi<=4096> <byte 0..255>\n";
            else {
                const auto found = find(mem, static_cast<std::uint16_t>(lo),
                                        static_cast<std::uint16_t>(hi), static_cast<Byte>(value));
                if (found == hi)
                    std::cout << "not found\n";
                else
                    std::cout << "found " << found << " index=" << (found - lo) << '\n';
            }
        } else if (cmd == "run") {
            std::string token;
            long limit = 100000;
            if (words >> token) {
                if (!parse_number(token, limit) || limit < 1 || limit > 10000000) {
                    std::cout << "usage: run [1..10000000]\n";
                    continue;
                }
            }
            run(cpu, static_cast<std::size_t>(limit));
        } else if (cmd == "dumpcpu") {
            dump(&cpu, sizeof(cpu));
        } else if (cmd == "step") {
            step(cpu);
        } else if (cmd == "inc") {
            std::string a;
            long addr = 0;
            if (!(words >> a) || !parse_number(a, addr)) {
                std::cout << "usage: inc <addr>\n";
            } else if (addr < 0 || static_cast<std::size_t>(addr) >= MEM_SIZE) {
                std::cout << "address " << addr << " is outside 0.." << MEM_SIZE - 1 << '\n';
            } else {
                const auto index = static_cast<std::size_t>(addr);
                mem_set(mem, index, static_cast<Byte>(mem_get(mem, index) + 1));
            }
        } else if (cmd == "set16") {
            std::string a, v;
            long addr = 0, value = 0;
            if (!(words >> a >> v) || !parse_number(a, addr) || !parse_number(v, value)) {
                std::cout << "usage: set16 <addr> <value>\n";
            } else if (addr < 0 || static_cast<std::size_t>(addr) >= MEM_SIZE - 1) {
                std::cout << "two-byte address must be 0.." << MEM_SIZE - 2 << '\n';
            } else if (value < 0 || value > 65535) {
                std::cout << "a 16-bit value is 0..65535\n";
            } else {
                const auto index = static_cast<std::size_t>(addr);
                set16(mem, index, static_cast<std::uint16_t>(value));
            }
        } else {
            std::cout << "unknown command: " << cmd << " (try `help`)\n";
        }
    }

    return 0; // 0 means "success" to the operating system
}
