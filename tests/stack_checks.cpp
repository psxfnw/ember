#include "cpu.hpp"
#include <cassert>
#include <iostream>
int main() {
    Memory mem;
    Stack stack;
    assert(stack.empty() && stack.sp() == 0xFFF);
    Byte out = 99;
    std::uint16_t word = 99;
    assert(!stack.pop(mem, out) && out == 99);
    assert(!stack.pop16(mem, word) && word == 99);
    for (unsigned i = 0; i < 255; ++i)
        assert(stack.push(mem, static_cast<Byte>(i)));
    assert(stack.sp() == 0xF00 && !stack.full());
    assert(!stack.push16(mem, 0x1234) && stack.sp() == 0xF00);
    assert(stack.push(mem, 255) && stack.full() && stack.sp() == 0xEFF);
    assert(!stack.push(mem, 7));
    for (int i = 255; i >= 0; --i) {
        assert(stack.pop(mem, out));
        assert(out == i);
    }
    assert(stack.empty());
    assert(stack.push16(mem, 0x1234));
    assert(mem_get(mem, 0xFFF) == 0x34 && mem_get(mem, 0xFFE) == 0x12);
    mem_set(mem, 0xFFE, 0x56);
    assert(stack.pop16(mem, word) && word == 0x5634 && stack.empty());
    assert(stack.push(mem, 17));
    assert(!stack.pop16(mem, word) && stack.sp() == 0xFFE);
    assert(stack.pop(mem, out) && out == 17);
    CPU call;
    call.mem = &mem;
    call.pc = 0x100;
    call.f = {true, true, true};
    mem_set(mem, 0x100, 0x54);
    set16(mem, 0x101, 0x200);
    mem_set(mem, 0x200, 0x55);
    step(call);
    assert(call.pc == 0x200 && call.stack.sp() == 0xFFD);
    assert(mem_get(mem, 0xFFF) == 3 && mem_get(mem, 0xFFE) == 1);
    step(call);
    assert(call.pc == 0x103 && call.stack.empty() && call.f.z && call.f.n && call.f.c);
    CPU bad;
    bad.mem = &mem;
    assert(bad.stack.push16(mem, 0x1000));
    mem_set(mem, 0, 0x55);
    step(bad);
    assert(bad.faulted && bad.pc == 0 && bad.stack.sp() == 0xFFD);
    CPU h;
    h.mem = &mem;
    h.h = 0xABCD;
    h.pc = 0x300;
    h.f = {true, true, true};
    mem_set(mem, 0x300, 0x56);
    mem_set(mem, 0x301, 0x57);
    step(h);
    h.h = 0;
    step(h);
    assert(h.h == 0xABCD && h.stack.empty() && h.f.z && h.f.n && h.f.c);
    CPU truncated;
    truncated.mem = &mem;
    truncated.pc = 4094;
    mem_set(mem, 4094, 0x54);
    step(truncated);
    assert(truncated.faulted && truncated.stack.empty());
    CPU capacity;
    capacity.mem = &mem;
    mem_set(mem, 0, 0x54);
    set16(mem, 1, 10);
    for (int i = 0; i < 255; ++i)
        assert(capacity.stack.push(mem, 0));
    step(capacity);
    assert(capacity.faulted && capacity.stack.sp() == 0xF00);
    std::cout << "PASS: stack 256-byte LIFO, atomic failures, LE16, guest memory, CALL/RET, H, "
                 "flags, bounds\n";
}
