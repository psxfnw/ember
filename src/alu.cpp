#include "alu.hpp"
#include <cassert>
static Byte result(Byte value, Flags& f) {
    f.z = value == 0;
    f.n = (value & 128) != 0;
    return value;
}
Byte alu_add(Byte a, Byte b, Flags& f) {
    Byte sum = 0;
    bool carry = false;
    for (int bit = 0; bit < 8; ++bit) {
        const bool ab = ((a >> bit) & 1) != 0, bb = ((b >> bit) & 1) != 0;
        const bool sum_bit = ab ^ bb ^ carry;
        const bool next = (ab && bb) || (ab && carry) || (bb && carry);
        if (sum_bit)
            sum = static_cast<Byte>(sum | (1u << bit));
        carry = next;
    }
    assert(sum == static_cast<Byte>(a + b));
    assert(carry == (static_cast<unsigned>(a) + b > 255));
    f.c = carry;
    return result(sum, f);
}
Byte alu_sub(Byte a, Byte b, Flags& f) {
    f.c = a < b;
    return result(static_cast<Byte>(a - b), f);
}
Byte alu_and(Byte a, Byte b, Flags& f) {
    f.c = false;
    return result(static_cast<Byte>(a & b), f);
}
Byte alu_or(Byte a, Byte b, Flags& f) {
    f.c = false;
    return result(static_cast<Byte>(a | b), f);
}
Byte alu_xor(Byte a, Byte b, Flags& f) {
    f.c = false;
    return result(static_cast<Byte>(a ^ b), f);
}
Byte alu_not(Byte a, Flags& f) {
    f.c = false;
    return result(static_cast<Byte>(~a), f);
}
Byte alu_shl(Byte a, Flags& f) {
    f.c = (a & 128) != 0;
    return result(static_cast<Byte>(a << 1), f);
}
Byte alu_shr(Byte a, Flags& f) {
    f.c = (a & 1) != 0;
    return result(static_cast<Byte>(a >> 1), f);
}
Byte alu_inc(Byte a, Flags& f) { return result(static_cast<Byte>(a + 1), f); }
Byte alu_dec(Byte a, Flags& f) { return result(static_cast<Byte>(a - 1), f); }
