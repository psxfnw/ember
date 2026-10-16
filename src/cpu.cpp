#include "cpu.hpp"
#include "alu.hpp"
#include <iomanip>
#include <iostream>
Decoded decode(Byte op) { return {static_cast<Byte>((op >> 4) & 15), static_cast<Byte>(op & 15)}; }
std::size_t instruction_size(Byte op) {
    switch (op) {
    case OP_LOADI_A:
    case OP_LOADI_B:
        return 2;
    case OP_LOAD_A:
    case OP_LOAD_B:
    case OP_STORE_A:
    case OP_STORE_B:
    case OP_LOADH:
    case OP_JMP:
    case OP_JZ:
    case OP_JNZ:
    case OP_JC:
    case OP_JNC:
        return 3;
    default:
        return 1;
    }
}
static void fail(CPU& cpu, const char* message) {
    cpu.halted = true;
    cpu.faulted = true;
    std::cout << "CPU error: " << message << '\n';
}
static bool address_ok(CPU& cpu, std::size_t addr) {
    if (addr >= MEM_SIZE) {
        fail(cpu, "address outside memory");
        return false;
    }
    return true;
}
static void unknown(CPU& cpu, Byte op) {
    cpu.halted = true;
    cpu.faulted = true;
    std::cout << "unknown opcode 0x" << std::hex << std::setfill('0') << std::setw(2)
              << unsigned(op) << std::dec << std::setfill(' ') << '\n';
}
void step(CPU& cpu) {
    if (cpu.halted) {
        std::cout << "halted\n";
        return;
    }
    if (cpu.mem == nullptr || cpu.pc >= MEM_SIZE) {
        fail(cpu, "PC outside memory or memory not connected");
        return;
    }
    const Byte op = mem_get(*cpu.mem, cpu.pc);
    const std::size_t size = instruction_size(op);
    if (size > MEM_SIZE - cpu.pc) {
        fail(cpu, "truncated instruction");
        return;
    }
    const std::uint16_t operand =
        size == 3 ? get16(*cpu.mem, cpu.pc + 1) : (size == 2 ? mem_get(*cpu.mem, cpu.pc + 1) : 0);
    std::uint16_t next_pc = static_cast<std::uint16_t>(cpu.pc + size);
    const Decoded instruction = decode(op);
    switch (instruction.group) {
    case 0:
        switch (instruction.index) {
        case OP_HALT:
            cpu.halted = true;
            break;
        case OP_OUT:
            std::cout << static_cast<char>(cpu.a);
            break;
        case OP_OUTN:
            std::cout << static_cast<unsigned>(cpu.a) << ' ';
            break;
        case OP_NOP:
            break;
        default:
            unknown(cpu, op);
            return;
        }
        break;
    case 1:
        switch (op) {
        case OP_CMP:
            alu_sub(cpu.a, cpu.b, cpu.f);
            break;
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
            unknown(cpu, op);
            return;
        }
        break;
    case 2:
        switch (op) {
        case OP_LOADI_A:
            cpu.a = static_cast<Byte>(operand);
            break;
        case OP_LOADI_B:
            cpu.b = static_cast<Byte>(operand);
            break;
        case OP_LOAD_A:
        case OP_LOAD_B:
            if (!address_ok(cpu, operand))
                return;
            if (op == OP_LOAD_A)
                cpu.a = mem_get(*cpu.mem, operand);
            else
                cpu.b = mem_get(*cpu.mem, operand);
            break;
        case OP_STORE_A:
        case OP_STORE_B:
            if (!address_ok(cpu, operand))
                return;
            mem_set(*cpu.mem, operand, op == OP_STORE_A ? cpu.a : cpu.b);
            break;
        case OP_MOV_A_B:
            cpu.a = cpu.b;
            break;
        case OP_MOV_B_A:
            cpu.b = cpu.a;
            break;
        case OP_LOADH:
            cpu.h = operand;
            break;
        case OP_LOAD_H:
            if (!address_ok(cpu, cpu.h))
                return;
            cpu.a = mem_get(*cpu.mem, cpu.h);
            break;
        case OP_STORE_H:
            if (!address_ok(cpu, cpu.h))
                return;
            mem_set(*cpu.mem, cpu.h, cpu.a);
            break;
        case OP_HLOW:
            cpu.a = static_cast<Byte>(cpu.h & 255);
            break;
        case OP_INCH:
            cpu.h = static_cast<std::uint16_t>(cpu.h + 1);
            break;
        case OP_DECH:
            cpu.h = static_cast<std::uint16_t>(cpu.h - 1);
            break;
        default:
            unknown(cpu, op);
            return;
        }
        break;
    case 3: {
        bool take = false;
        switch (op) {
        case OP_JMP:
            take = true;
            break;
        case OP_JZ:
            take = cpu.f.z;
            break;
        case OP_JNZ:
            take = !cpu.f.z;
            break;
        case OP_JC:
            take = cpu.f.c;
            break;
        case OP_JNC:
            take = !cpu.f.c;
            break;
        default:
            unknown(cpu, op);
            return;
        }
        if (take) {
            if (!address_ok(cpu, operand))
                return;
            next_pc = operand;
        }
        break;
    }
    default:
        unknown(cpu, op);
        return;
    }
    cpu.pc = next_pc;
    mem_set(*cpu.mem, FLAG_STATUS,
            static_cast<Byte>((cpu.f.z ? 1 : 0) | (cpu.f.n ? 2 : 0) | (cpu.f.c ? 4 : 0)));
}
void dump_regs(const CPU& cpu) {
    std::cout << "PC=" << cpu.pc << " A=" << unsigned(cpu.a) << " B=" << unsigned(cpu.b)
              << " H=" << cpu.h << " Z=" << cpu.f.z << " N=" << cpu.f.n << " C=" << cpu.f.c
              << " halted=" << cpu.halted << '\n';
}

void run(CPU& cpu, std::size_t limit) {
    std::size_t steps = 0;
    while (!cpu.halted && steps < limit) {
        if (cpu.breakpoint >= 0 && cpu.pc == static_cast<std::uint16_t>(cpu.breakpoint)) {
            std::cout << "breakpoint at " << cpu.pc << '\n';
            break;
        }
        step(cpu);
        ++steps;
    }
    std::cout << "\nsteps=" << steps << '\n';
    if (!cpu.halted)
        std::cout << "step limit reached (" << limit << ")\n";
}
