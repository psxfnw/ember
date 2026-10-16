#pragma once
#include "display.hpp"
struct Sprite {
    std::uint8_t x = 0, y = 0;
    std::int8_t vx = 0, vy = 0;
    bool alive = false;
};
void tick(Memory& mem, Sprite* sprites, std::size_t count);
void dump_sprites(const Sprite* sprites, std::size_t count);
