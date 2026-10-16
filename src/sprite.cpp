#include "sprite.hpp"
#include <iostream>
static void advance(std::uint8_t& position, std::int8_t& speed, int maximum) {
    int next = static_cast<int>(position) + speed;
    int velocity = speed;
    while (next < 0 || next > maximum) {
        if (next < 0)
            next = -next;
        else
            next = 2 * maximum - next;
        velocity = -velocity;
    }
    if ((next == 0 && velocity < 0) || (next == maximum && velocity > 0))
        velocity = -velocity;
    position = static_cast<std::uint8_t>(next);
    speed = static_cast<std::int8_t>(velocity);
}
void tick(Memory& mem, Sprite* sprites, std::size_t count) {
    clear(mem);
    for (std::size_t i = 0; i < count; ++i) {
        if (!sprites[i].alive)
            continue;
        advance(sprites[i].x, sprites[i].vx, 63);
        advance(sprites[i].y, sprites[i].vy, 31);
        plot(mem, sprites[i].x, sprites[i].y);
    }
    show(mem);
}
void dump_sprites(const Sprite* sprites, std::size_t count) {
    for (std::size_t i = 0; i < count; ++i) {
        const Sprite& s = sprites[i];
        if (s.alive)
            std::cout << "sprite " << i << " x=" << unsigned(s.x) << " y=" << unsigned(s.y)
                      << " vx=" << int(s.vx) << " vy=" << int(s.vy) << '\n';
    }
}
