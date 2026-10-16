#pragma once
#include "memory.hpp"
#include <cstdint>

enum class Op : std::uint8_t {
    Halt = 0x00,
    Nop = 0x01,
    Out = 0x02,
    OutN = 0x03,
    OutS = 0x04,
    Add = 0x10,
    Sub = 0x11,
    And = 0x12,
    Or = 0x13,
    Xor = 0x14,
    Not = 0x15,
    Shl = 0x16,
    Shr = 0x17,
    Inc = 0x18,
    Dec = 0x19,
    Cmp = 0x1A,
    LoadiA = 0x20,
    LoadiB = 0x21,
    LoadA = 0x22,
    LoadB = 0x23,
    StoreA = 0x24,
    StoreB = 0x25,
    MovAB = 0x26,
    MovBA = 0x27,
    LoadH = 0x28,
    LoadIndirect = 0x29,
    StoreIndirect = 0x2A,
    IncH = 0x2B,
    DecH = 0x2C,
    HLow = 0x2D,
    Jmp = 0x30,
    Jz = 0x31,
    Jnz = 0x32,
    Jc = 0x33,
    Jnc = 0x34,
    Cls = 0x40,
    Plot = 0x41,
    Unplot = 0x42,
    Show = 0x43,
    Alloc = 0x60
};

struct Flags {
    bool z = false; // результат нульовий
    bool n = false; // старший біт результату — одиниця
    bool c = false; // біт, який виїхав за межу восьми
};

struct CPU {
    Memory* mem = nullptr; // де лежить коробка. Що таке `*`, пояснює Lab 3;
                           // поки читайте як «процесор знає, де його пам'ять»
    std::uint16_t pc = 0;  // адреса НАСТУПНОЇ інструкції
    std::uint64_t ticks = 0;
    std::uint16_t heap_ptr = HEAP_LO;
    std::uint16_t h = 0;
    Byte a = 0;
    Byte b = 0;
    Flags f;
    bool faulted = false;
    int breakpoint = -1;
    bool halted = false; // після HALT наступні step відмовляють
};

void step(CPU& cpu);
void dump_regs(const CPU& cpu);
const std::size_t FLAG_STATUS = 0xB02;
struct Decoded {
    Byte group;
    Byte index;
};
Decoded decode(Byte op);

std::size_t instruction_size(Byte op);
void run(CPU& cpu, std::size_t limit = 100000);
