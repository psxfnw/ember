// memory.hpp — the box of bytes.
//
// GIVEN. You do not have to change this file in Lab 1, but read every line:
// four of the five ideas in Lab 1's theory are visible here.
#pragma once

#include <cstddef>
#include <cstdint>

// A cell of ember memory is exactly one byte, on every machine, forever.
// That is why it is std::uint8_t and not `int` (whose width is not promised).
using Byte = std::uint8_t;

// `const` means: this name will not be used to change these bits.
const std::size_t MEM_SIZE = 4096;

struct Memory {
    // The `{}` zero-initializes the whole array.
    // Experiment (Lab 1, M4): delete the `{}`, rebuild, `dump`. What appears?
    // Put the `{}` back afterwards — reading uninitialized memory is UB,
    // and this course does not ship UB.
    Byte data[MEM_SIZE]{};
};

// Read the byte at `addr`. If `addr` is outside the box, return 0.
Byte mem_get(const Memory& mem, std::size_t addr);

// Write `value` at `addr`. Return false (and change nothing) if `addr` is
// outside the box. Returning false is how main.cpp knows to print a message.
bool mem_set(Memory& mem, std::size_t addr, Byte value);

const std::size_t CODE_LO = 0x000, CODE_HI = 0x7FF;
const std::size_t DATA_LO = 0x800, DATA_HI = 0x9FF;
std::uint16_t get16(const Memory& mem, std::size_t addr);
bool set16(Memory& mem, std::size_t addr, std::uint16_t value);

std::uint16_t find(const Memory& mem, std::uint16_t lo, std::uint16_t hi, Byte needle);

const std::size_t VRAM_LO = 0xA00, VRAM_HI = 0xAFF;
const std::size_t PORT_OUT = 0xB00, TICKS = 0xB01;
bool sort_memory(Memory& mem, std::uint16_t lo, std::uint16_t hi, bool visualize = false);
