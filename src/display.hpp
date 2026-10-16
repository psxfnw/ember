#pragma once
#include "memory.hpp"
void show(const Memory& mem);
void plot(Memory& mem, int x, int y);
void unplot(Memory& mem, int x, int y);
void clear(Memory& mem);
void frame(Memory& mem);
int rowmin(const Memory& mem, int y);
void show_bars(Memory& mem, std::uint16_t lo, std::uint16_t hi);
