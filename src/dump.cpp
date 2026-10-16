#include "dump.hpp"
#include <iomanip>
#include <iostream>

static bool is_printable(Byte b) { return b >= 0x20 && b <= 0x7E; }
void dump(const void* ptr, std::size_t n) {
    if (ptr == nullptr && n != 0) {
        std::cout << "null buffer\n";
        return;
    }
    const auto* bytes = static_cast<const unsigned char*>(ptr);
    for (std::size_t row = 0; row < n; row += 16) {
        std::cout << std::hex << std::setfill('0') << std::setw(4) << row << "  ";
        for (std::size_t col = 0; col < 16; ++col) {
            if (col < n - row)
                std::cout << std::setw(2) << unsigned(bytes[row + col]) << ' ';
            else
                std::cout << "   ";
        }
        std::cout << " |";
        for (std::size_t col = 0; col < 16 && col < n - row; ++col) {
            const Byte b = bytes[row + col];
            std::cout << (is_printable(b) ? static_cast<char>(b) : '.');
        }
        std::cout << "|\n";
    }
    std::cout << std::dec << std::setfill(' ');
}
void dump(const Memory& mem) { dump(mem.data, MEM_SIZE); }
void show_byte(Byte b) {
    std::cout << unsigned(b) << "  0x" << std::hex << std::setfill('0') << std::setw(2)
              << unsigned(b) << std::dec << std::setfill(' ') << "  0b";
    for (int bit = 7; bit >= 0; --bit)
        std::cout << ((b >> bit) & 1);
    std::cout << "  '" << (is_printable(b) ? static_cast<char>(b) : '.') << "'\n";
}
