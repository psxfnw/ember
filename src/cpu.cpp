#include "cpu.hpp"
#include "alu.hpp"
#include <iostream>
Decoded decode(Byte op) { return {static_cast<Byte>((op >> 4) & 15), static_cast<Byte>(op & 15)}; }
void step(CPU& cpu) {
    if (cpu.halted) {
        std::cout << "halted\n";
        return;
    }
    if (cpu.mem == nullptr || cpu.pc >= MEM_SIZE) {
        cpu.halted = true;
        std::cout << "PC outside memory or memory not connected\n";
        return;
    }
    const Byte op = mem_get(*cpu.mem, cpu.pc);
    const Decoded instruction = decode(op);
    switch (instruction.group) {
    case 0:
        switch (instruction.index) {
        case OP_HALT:
            cpu.halted = true;
            break;
        case OP_NOP:
            break;
        default:
            break;
        }
        break;
    case 1:
        switch (op) {
        case OP_ADD:
            cpu.a = alu_add(cpu.a, cpu.b, cpu.f);
            break;
        case OP_SUB:
            cpu.a = alu_sub(cpu.a, cpu.b, cpu.f);
            break;
        case OP_AND:
            cpu.a = alu_and(cpu.a, cpu.b, cpu.f);
            break;
        case OP_OR:
            cpu.a = alu_or(cpu.a, cpu.b, cpu.f);
            break;
        case OP_XOR:
            cpu.a = alu_xor(cpu.a, cpu.b, cpu.f);
            break;
        case OP_NOT:
            cpu.a = alu_not(cpu.a, cpu.f);
            break;
        case OP_SHL:
            cpu.a = alu_shl(cpu.a, cpu.f);
            break;
        case OP_SHR:
            cpu.a = alu_shr(cpu.a, cpu.f);
            break;
        case OP_INC:
            cpu.a = alu_inc(cpu.a, cpu.f);
            break;
        case OP_DEC:
            cpu.a = alu_dec(cpu.a, cpu.f);
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
    ++cpu.pc;
    mem_set(*cpu.mem, FLAG_STATUS,
            static_cast<Byte>((cpu.f.z ? 1 : 0) | (cpu.f.n ? 2 : 0) | (cpu.f.c ? 4 : 0)));
}
void dump_regs(const CPU& cpu) {
    std::cout << "PC=" << cpu.pc << " A=" << unsigned(cpu.a) << " B=" << unsigned(cpu.b)
              << " Z=" << cpu.f.z << " N=" << cpu.f.n << " C=" << cpu.f.c
              << " halted=" << cpu.halted << '\n';
}
