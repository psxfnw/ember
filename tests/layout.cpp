#include "cpu.hpp"
#include "sprite.hpp"
#include "value.hpp"
#include <cassert>
#include <cstddef>
#include <iostream>
struct P {
    char c;
    int n;
};
int main() {
    std::cout << "CPU size=" << sizeof(CPU) << " align=" << alignof(CPU) << '\n';
    std::cout << "mem offset=" << offsetof(CPU, mem) << " size=" << sizeof(CPU::mem) << '\n';
    std::cout << "pc offset=" << offsetof(CPU, pc) << " size=" << sizeof(CPU::pc) << '\n';
    std::cout << "ticks offset=" << offsetof(CPU, ticks) << " size=" << sizeof(CPU::ticks) << '\n';
    std::cout << "heap_ptr offset=" << offsetof(CPU, heap_ptr) << " size=" << sizeof(CPU::heap_ptr)
              << '\n';
    std::cout << "h offset=" << offsetof(CPU, h) << " size=" << sizeof(CPU::h) << '\n';
    std::cout << "a offset=" << offsetof(CPU, a) << " size=" << sizeof(CPU::a) << '\n';
    std::cout << "b offset=" << offsetof(CPU, b) << " size=" << sizeof(CPU::b) << '\n';
    std::cout << "f offset=" << offsetof(CPU, f) << " size=" << sizeof(CPU::f) << '\n';
    std::cout << "faulted offset=" << offsetof(CPU, faulted) << " size=" << sizeof(CPU::faulted)
              << '\n';
    std::cout << "breakpoint offset=" << offsetof(CPU, breakpoint)
              << " size=" << sizeof(CPU::breakpoint) << '\n';
    std::cout << "halted offset=" << offsetof(CPU, halted) << " size=" << sizeof(CPU::halted)
              << '\n';
    std::cout << "Flags size=" << sizeof(Flags) << '\n';
    std::cout << "Sprite size=" << sizeof(Sprite) << " align=" << alignof(Sprite) << '\n';
    std::cout << "Sprite offsets " << offsetof(Sprite, x) << ' ' << offsetof(Sprite, y) << ' '
              << offsetof(Sprite, vx) << ' ' << offsetof(Sprite, vy) << ' '
              << offsetof(Sprite, alive) << '\n';
    std::cout << "P size=" << sizeof(P) << " fields=" << sizeof(char) + sizeof(int)
              << " n-offset=" << offsetof(P, n) << '\n';
    Value value{Value::Kind::Addr, 0xC00};
    assert(value.kind == Value::Kind::Addr && value.bits == 3072);
    std::cout << "Value size=" << sizeof(Value) << " kind-offset=" << offsetof(Value, kind)
              << " bits-offset=" << offsetof(Value, bits) << " Addr=" << value.bits << '\n';
    std::cout << "Op Add=" << unsigned(static_cast<Byte>(Op::Add)) << '\n';
}
