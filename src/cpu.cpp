#include "cpu.hpp"
#include "alu.hpp"
#include "display.hpp"
#include <iomanip>
#include <iostream>
Decoded decode(Byte op) { return {static_cast<Byte>((op >> 4) & 15), static_cast<Byte>(op & 15)}; }
std::size_t instruction_size(Byte op) {
    switch (static_cast<Op>(op)) {
    case Op::LoadiA:
    case Op::LoadiB:
        return 2;
    case Op::LoadA:
    case Op::LoadB:
    case Op::StoreA:
    case Op::StoreB:
    case Op::OutS:
    case Op::Call:
    case Op::LoadH:
    case Op::Jmp:
    case Op::Jz:
    case Op::Jnz:
    case Op::Jc:
    case Op::Jnc:
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
static Byte guest_read(const CPU& cpu, std::size_t addr) {
    if (addr == TICKS)
        return static_cast<Byte>((cpu.ticks + 1) & 255);
    return mem_get(*cpu.mem, addr);
}
static void guest_write(CPU& cpu, std::size_t addr, Byte value) {
    mem_set(*cpu.mem, addr, value);
    if (addr == PORT_OUT)
        std::cout << static_cast<char>(value);
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
    const Byte raw = mem_get(*cpu.mem, cpu.pc);
    const Op op = static_cast<Op>(raw);
    const std::size_t size = instruction_size(raw);
    if (size > MEM_SIZE - cpu.pc) {
        fail(cpu, "truncated instruction");
        return;
    }
    const std::uint16_t operand =
        size == 3 ? get16(*cpu.mem, cpu.pc + 1) : (size == 2 ? mem_get(*cpu.mem, cpu.pc + 1) : 0);
    std::uint16_t next_pc = static_cast<std::uint16_t>(cpu.pc + size);
    const Decoded instruction = decode(raw);
    switch (instruction.group) {
    case 0:
        switch (op) {
        case Op::Halt:
            cpu.halted = true;
            break;
        case Op::OutS:
            if (!address_ok(cpu, operand))
                return;
            for (std::size_t i = 0; i < 256 && i < MEM_SIZE - operand; ++i) {
                const Byte value = guest_read(cpu, operand + i);
                if (value == 0)
                    break;
                std::cout << static_cast<char>(value);
            }
            break;
        case Op::Out:
            std::cout << static_cast<char>(cpu.a);
            break;
        case Op::OutN:
            std::cout << static_cast<unsigned>(cpu.a) << ' ';
            break;
        case Op::Nop:
            break;
        default:
            unknown(cpu, raw);
            return;
        }
        break;
    case 1:
        switch (op) {
        case Op::Mul:
            cpu.a = alu_mul(cpu.a, cpu.b, cpu.f);
            break;
        case Op::Cmp:
            alu_sub(cpu.a, cpu.b, cpu.f);
            break;
        case Op::Add:
            cpu.a = alu_add(cpu.a, cpu.b, cpu.f);
            break;
        case Op::Sub:
            cpu.a = alu_sub(cpu.a, cpu.b, cpu.f);
            break;
        case Op::And:
            cpu.a = alu_and(cpu.a, cpu.b, cpu.f);
            break;
        case Op::Or:
            cpu.a = alu_or(cpu.a, cpu.b, cpu.f);
            break;
        case Op::Xor:
            cpu.a = alu_xor(cpu.a, cpu.b, cpu.f);
            break;
        case Op::Not:
            cpu.a = alu_not(cpu.a, cpu.f);
            break;
        case Op::Shl:
            cpu.a = alu_shl(cpu.a, cpu.f);
            break;
        case Op::Shr:
            cpu.a = alu_shr(cpu.a, cpu.f);
            break;
        case Op::Inc:
            cpu.a = alu_inc(cpu.a, cpu.f);
            break;
        case Op::Dec:
            cpu.a = alu_dec(cpu.a, cpu.f);
            break;
        default:
            unknown(cpu, raw);
            return;
        }
        break;
    case 2:
        switch (op) {
        case Op::LoadiA:
            cpu.a = static_cast<Byte>(operand);
            break;
        case Op::LoadiB:
            cpu.b = static_cast<Byte>(operand);
            break;
        case Op::LoadA:
        case Op::LoadB:
            if (!address_ok(cpu, operand))
                return;
            if (op == Op::LoadA)
                cpu.a = guest_read(cpu, operand);
            else
                cpu.b = guest_read(cpu, operand);
            break;
        case Op::StoreA:
        case Op::StoreB:
            if (!address_ok(cpu, operand))
                return;
            guest_write(cpu, operand, op == Op::StoreA ? cpu.a : cpu.b);
            break;
        case Op::MovAB:
            cpu.a = cpu.b;
            break;
        case Op::MovBA:
            cpu.b = cpu.a;
            break;
        case Op::LoadH:
            cpu.h = operand;
            break;
        case Op::LoadIndirect:
            if (!address_ok(cpu, cpu.h))
                return;
            cpu.a = guest_read(cpu, cpu.h);
            break;
        case Op::StoreIndirect:
            if (!address_ok(cpu, cpu.h))
                return;
            guest_write(cpu, cpu.h, cpu.a);
            break;
        case Op::HLow:
            cpu.a = static_cast<Byte>(cpu.h & 255);
            break;
        case Op::IncH:
            cpu.h = static_cast<std::uint16_t>(cpu.h + 1);
            break;
        case Op::DecH:
            cpu.h = static_cast<std::uint16_t>(cpu.h - 1);
            break;
        default:
            unknown(cpu, raw);
            return;
        }
        break;
    case 3: {
        bool take = false;
        switch (op) {
        case Op::Jmp:
            take = true;
            break;
        case Op::Jz:
            take = cpu.f.z;
            break;
        case Op::Jnz:
            take = !cpu.f.z;
            break;
        case Op::Jc:
            take = cpu.f.c;
            break;
        case Op::Jnc:
            take = !cpu.f.c;
            break;
        default:
            unknown(cpu, raw);
            return;
        }
        if (take) {
            if (!address_ok(cpu, operand))
                return;
            next_pc = operand;
        }
        break;
    }
    case 4:
        switch (op) {
        case Op::Cls:
            clear(*cpu.mem);
            break;
        case Op::Plot:
        case Op::Unplot:
            cpu.f.c = cpu.a >= 64 || cpu.b >= 32;
            if (!cpu.f.c) {
                if (op == Op::Plot)
                    plot(*cpu.mem, cpu.a, cpu.b);
                else
                    unplot(*cpu.mem, cpu.a, cpu.b);
            }
            break;
        case Op::Show:
            show(*cpu.mem);
            break;
        default:
            unknown(cpu, raw);
            return;
        }
        break;
    case 5: {
        Byte value = 0;
        std::uint16_t target = 0;
        switch (op) {
        case Op::PushA:
        case Op::PushB:
            if (!cpu.stack.push(*cpu.mem, op == Op::PushA ? cpu.a : cpu.b)) {
                fail(cpu, "stack overflow");
                return;
            }
            break;
        case Op::PopA:
        case Op::PopB:
            if (!cpu.stack.pop(*cpu.mem, value)) {
                fail(cpu, "stack underflow");
                return;
            }
            if (op == Op::PopA)
                cpu.a = value;
            else
                cpu.b = value;
            break;
        case Op::PushH:
            if (!cpu.stack.push16(*cpu.mem, cpu.h)) {
                fail(cpu, "stack overflow");
                return;
            }
            break;
        case Op::PopH:
            if (!cpu.stack.pop16(*cpu.mem, target)) {
                fail(cpu, "stack underflow");
                return;
            }
            cpu.h = target;
            break;
        case Op::Call:
            if (!address_ok(cpu, operand))
                return;
            if (!cpu.stack.push16(*cpu.mem, next_pc)) {
                fail(cpu, "stack overflow");
                return;
            }
            next_pc = operand;
            break;
        case Op::Ret:
            if (!cpu.stack.peek16(*cpu.mem, target)) {
                fail(cpu, "stack underflow");
                return;
            }
            if (!address_ok(cpu, target))
                return;
            cpu.stack.pop16(*cpu.mem, target);
            next_pc = target;
            break;
        default:
            unknown(cpu, raw);
            return;
        }
        break;
    }
    case 6:
        switch (op) {
        case Op::Alloc:
            if (cpu.heap_ptr < HEAP_LO || cpu.heap_ptr > HEAP_HI + 1 ||
                cpu.a > HEAP_HI + 1 - cpu.heap_ptr)
                cpu.f.c = true;
            else {
                cpu.h = cpu.heap_ptr;
                cpu.heap_ptr = static_cast<std::uint16_t>(cpu.heap_ptr + cpu.a);
                cpu.f.c = false;
            }
            break;
        default:
            unknown(cpu, raw);
            return;
        }
        break;
    default:
        unknown(cpu, raw);
        return;
    }
    cpu.pc = next_pc;
    ++cpu.ticks;
    mem_set(*cpu.mem, FLAG_STATUS,
            static_cast<Byte>((cpu.f.z ? 1 : 0) | (cpu.f.n ? 2 : 0) | (cpu.f.c ? 4 : 0)));
}
void dump_regs(const CPU& cpu) {
    std::cout << "PC=" << cpu.pc << " A=" << unsigned(cpu.a) << " B=" << unsigned(cpu.b)
              << " H=" << cpu.h << " Z=" << cpu.f.z << " N=" << cpu.f.n << " C=" << cpu.f.c
              << " halted=" << cpu.halted << " heap=" << cpu.heap_ptr << " SP=" << cpu.stack.sp()
              << '\n';
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
