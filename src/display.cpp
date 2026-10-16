#include "display.hpp"
#include <iostream>
void show(const Memory& mem) {
    for (int y = 0; y < 32; ++y) {
        for (int x = 0; x < 64; ++x) {
            Byte cell = mem.data[0xA00 + y * 8 + (x / 8)];
            bool lit = (cell >> (7 - (x % 8))) & 1;
            std::cout << (lit ? '#' : '.');
        }
        std::cout << '\n';
    }
}
void plot(Memory& mem, int x, int y) {
    if (x < 0 || x >= 64 || y < 0 || y >= 32)
        return;
    const auto addr = VRAM_LO + static_cast<std::size_t>(y) * 8 + static_cast<std::size_t>(x) / 8;
    mem.data[addr] = static_cast<Byte>(mem.data[addr] | (1u << (7 - x % 8)));
}
void unplot(Memory& mem, int x, int y) {
    if (x < 0 || x >= 64 || y < 0 || y >= 32)
        return;
    const auto addr = VRAM_LO + static_cast<std::size_t>(y) * 8 + static_cast<std::size_t>(x) / 8;
    mem.data[addr] = static_cast<Byte>(mem.data[addr] & ~(1u << (7 - x % 8)));
}
void clear(Memory& mem) {
    for (std::size_t i = VRAM_LO; i <= VRAM_HI; ++i)
        mem.data[i] = 0;
}
void frame(Memory& mem) {
    clear(mem);
    for (int x = 0; x < 64; ++x) {
        plot(mem, x, 0);
        plot(mem, x, 31);
    }
    for (int y = 0; y < 32; ++y) {
        plot(mem, 0, y);
        plot(mem, 63, y);
    }
}
int rowmin(const Memory& mem, int y) {
    if (y < 0 || y >= 32)
        return -1;
    for (int x = 0; x < 64; ++x)
        if ((mem.data[VRAM_LO + static_cast<std::size_t>(y) * 8 +
                      static_cast<std::size_t>(x) / 8] >>
             (7 - x % 8)) &
            1)
            return x;
    return -1;
}
void show_bars(Memory& mem, std::uint16_t lo, std::uint16_t hi) {
    clear(mem);
    for (std::uint16_t i = lo; i < hi; ++i) {
        const int height = mem_get(mem, i) > 32 ? 32 : mem_get(mem, i);
        for (int y = 31; y >= 32 - height; --y)
            plot(mem, i - lo, y);
    }
    show(mem);
}
