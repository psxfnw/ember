#pragma once
#include "memory.hpp"

struct StackState;
class Stack {
    StackState* state;

  public:
    Stack();
    ~Stack();
    Stack(const Stack&) = delete;
    Stack& operator=(const Stack&) = delete;
    bool push(Memory& mem, Byte value);
    bool pop(const Memory& mem, Byte& out);
    bool push16(Memory& mem, std::uint16_t value);
    bool pop16(const Memory& mem, std::uint16_t& out);
    bool peek16(const Memory& mem, std::uint16_t& out) const;
    std::uint16_t sp() const;
    std::size_t size() const;
    bool empty() const;
    bool full() const;
};
void dump_stack(const Stack& stack, const Memory& mem);
