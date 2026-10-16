#include "stack.hpp"
#include <iomanip>
#include <iostream>
struct StackState {
    std::uint16_t sp = STACK_HI;
};
Stack::Stack() : state(new StackState{}) {}
Stack::~Stack() { delete state; }
bool Stack::push(Memory& mem, Byte value) {
    if (full())
        return false;
    mem_set(mem, state->sp, value);
    --state->sp;
    return true;
}
bool Stack::pop(const Memory& mem, Byte& out) {
    if (empty())
        return false;
    ++state->sp;
    out = mem_get(mem, state->sp);
    return true;
}
bool Stack::push16(Memory& mem, std::uint16_t value) {
    if (256 - size() < 2)
        return false;
    push(mem, static_cast<Byte>(value & 255));
    push(mem, static_cast<Byte>(value >> 8));
    return true;
}

std::uint16_t Stack::sp() const { return state->sp; }
std::size_t Stack::size() const { return STACK_HI - state->sp; }
bool Stack::empty() const { return size() == 0; }
bool Stack::full() const { return size() == 256; }
bool Stack::peek16(const Memory& mem, std::uint16_t& out) const {
    if (size() < 2)
        return false;
    const Byte hi = mem_get(mem, state->sp + 1), lo = mem_get(mem, state->sp + 2);
    out = static_cast<std::uint16_t>((hi << 8) | lo);
    return true;
}
bool Stack::pop16(const Memory& mem, std::uint16_t& out) {
    if (!peek16(mem, out))
        return false;
    Byte hi = 0, lo = 0;
    pop(mem, hi);
    pop(mem, lo);
    return true;
}
void dump_stack(const Stack& stack, const Memory& mem) {
    std::cout << "SP=" << stack.sp() << " used=" << stack.size() << '\n';
    for (std::size_t addr = stack.sp() + 1; addr <= STACK_HI && addr < stack.sp() + 9u; ++addr)
        std::cout << "  " << std::hex << std::setfill('0') << std::setw(4) << addr << ": "
                  << std::setw(2) << unsigned(mem_get(mem, addr)) << std::dec << std::setfill(' ')
                  << '\n';
}
