#pragma once
#include <cstdint>
struct Value {
    enum class Kind : std::uint8_t { Byte, Addr, SpriteId };
    Kind kind = Kind::Byte;
    std::uint16_t bits = 0;
};
