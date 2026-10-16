#include "alu.hpp"
#include <cassert>
#include <iostream>
#include <limits>
static void check(Byte v, unsigned expected, const Flags& f, bool carry) {
    assert(v == static_cast<Byte>(expected));
    assert(f.z == (v == 0));
    assert(f.n == ((v & 128) != 0));
    assert(f.c == carry);
}
int main() {
    Flags f;
    for (unsigned a = 0; a < 256; ++a) {
        for (unsigned b = 0; b < 256; ++b) {
            const Byte x = static_cast<Byte>(a), y = static_cast<Byte>(b);
            Byte v;
            v = alu_add(x, y, f);
            check(v, a + b, f, a + b > 255);
            v = alu_sub(x, y, f);
            check(v, a - b, f, a < b);
            v = alu_and(x, y, f);
            check(v, a & b, f, false);
            v = alu_or(x, y, f);
            check(v, a | b, f, false);
            v = alu_xor(x, y, f);
            check(v, a ^ b, f, false);
        }
        const Byte x = static_cast<Byte>(a);
        Byte v;
        v = alu_not(x, f);
        check(v, (~a) & 255, f, false);
        v = alu_shl(x, f);
        check(v, a << 1, f, (a & 128) != 0);
        v = alu_shr(x, f);
        check(v, a >> 1, f, (a & 1) != 0);
        for (bool carry : {false, true}) {
            f.c = carry;
            v = alu_inc(x, f);
            check(v, a + 1, f, carry);
            f.c = carry;
            v = alu_dec(x, f);
            check(v, a - 1, f, carry);
        }
    }
    Memory mem;
    assert(!mem_set(mem, MEM_SIZE, 99));
    assert(!mem_set(mem, std::numeric_limits<std::size_t>::max(), 99));
    assert(mem_get(mem, MEM_SIZE) == 0);
    CPU cpu;
    cpu.mem = &mem;
    cpu.a = 200;
    cpu.b = 100;
    mem_set(mem, 0, OP_ADD);
    mem_set(mem, 1, OP_INC);
    mem_set(mem, 2, OP_HALT);
    step(cpu);
    assert(cpu.a == 44 && cpu.f.c && cpu.pc == 1 && mem_get(mem, FLAG_STATUS) == 4);
    step(cpu);
    assert(cpu.a == 45 && cpu.f.c && cpu.pc == 2);
    step(cpu);
    assert(cpu.halted && cpu.pc == 3);
    step(cpu);
    assert(cpu.pc == 3);
    std::cout << "PASS: 65536 operand pairs, all unary values, preserved carry, CPU/HALT/bounds\n";
    Byte v = alu_add(200, 100, f);
    std::cout << "ADD 200 100: result=" << unsigned(v) << " Z=" << f.z << " N=" << f.n
              << " C=" << f.c << '\n';
    v = alu_shl(128, f);
    std::cout << "SHL 128: result=" << unsigned(v) << " Z=" << f.z << " N=" << f.n << " C=" << f.c
              << '\n';
    v = alu_and(7, 1, f);
    std::cout << "AND 7 1: result=" << unsigned(v) << " Z=" << f.z << " N=" << f.n << " C=" << f.c
              << '\n';
}
