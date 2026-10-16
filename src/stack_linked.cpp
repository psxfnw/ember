#include "stack.hpp"
#include <iomanip>
#include <iostream>
#include <new>
struct Node {
    std::uint16_t address;
    Node* next;
};
struct StackState {
    std::uint16_t sp = STACK_HI;
    Node* top = nullptr;
};
Stack::Stack() : state(new StackState{}) {}
Stack::~Stack() {
    while (state->top) {
        Node* next = state->top->next;
        delete state->top;
        state->top = next;
    }
    delete state;
}
bool Stack::push(Memory& mem, Byte value) {
    if (full())
        return false;
    Node* node = new (std::nothrow) Node{state->sp, state->top};
    if (node == nullptr)
        return false;
    mem_set(mem, state->sp, value);
    --state->sp;
    state->top = node;
    return true;
}
bool Stack::pop(const Memory& mem, Byte& out) {
    if (empty())
        return false;
    Node* node = state->top;
    out = mem_get(mem, node->address);
    state->top = node->next;
    delete node;
    ++state->sp;
    return true;
}
bool Stack::push16(Memory& mem, std::uint16_t value) {
    if (256 - size() < 2)
        return false;
    Node* lo = new (std::nothrow) Node{state->sp, state->top};
    if (lo == nullptr)
        return false;
    Node* hi = new (std::nothrow) Node{static_cast<std::uint16_t>(state->sp - 1), lo};
    if (hi == nullptr) {
        delete lo;
        return false;
    }
    mem_set(mem, state->sp, static_cast<Byte>(value & 255));
    mem_set(mem, state->sp - 1, static_cast<Byte>(value >> 8));
    state->sp = static_cast<std::uint16_t>(state->sp - 2);
    state->top = hi;
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
