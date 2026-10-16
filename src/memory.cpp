#include "memory.hpp"
#include "display.hpp"

Byte mem_get(const Memory& mem, std::size_t addr) {
    if (addr >= MEM_SIZE) {
        return 0;
    }
    return mem.data[addr];
}

bool mem_set(Memory& mem, std::size_t addr, Byte value) {
    if (addr >= MEM_SIZE) {
        return false;
    }
    mem.data[addr] = value;
    return true;
}

std::uint16_t get16(const Memory& mem, std::size_t addr) {
    if (addr >= MEM_SIZE - 1)
        return 0;
    const Byte lo = mem_get(mem, addr), hi = mem_get(mem, addr + 1);
    return static_cast<std::uint16_t>(lo | (hi << 8));
}
bool set16(Memory& mem, std::size_t addr, std::uint16_t value) {
    if (addr >= MEM_SIZE - 1)
        return false;
    mem_set(mem, addr, static_cast<Byte>(value & 255));
    mem_set(mem, addr + 1, static_cast<Byte>(value >> 8));
    return true;
}

std::uint16_t find(const Memory& mem, std::uint16_t lo, std::uint16_t hi, Byte needle) {
    if (lo > hi || hi > MEM_SIZE)
        return hi;
    for (std::uint16_t i = lo; i < hi; ++i)
        if (mem_get(mem, i) == needle)
            return i;
    return hi;
}

bool sort_memory(Memory& mem, std::uint16_t lo, std::uint16_t hi, bool visualize) {
    if (lo > hi || hi > MEM_SIZE)
        return false;
    if (visualize && (lo < DATA_LO || hi > DATA_HI + 1 || hi - lo > 64))
        return false;
    for (std::uint16_t end = hi; end > lo + 1; --end) {
        bool changed = false;
        for (std::uint16_t i = lo; i + 1 < end; ++i) {
            const Byte a = mem_get(mem, i), b = mem_get(mem, i + 1);
            if (a > b) {
                mem_set(mem, i, b);
                mem_set(mem, i + 1, a);
                changed = true;
            }
        }
        if (visualize)
            show_bars(mem, lo, hi);
        if (!changed)
            break;
    }
    return true;
}
