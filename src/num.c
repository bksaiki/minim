// num.c: fixnum operations

#include "minim.h"

obj Mfx_neg(obj x) {
    return Mfixnum(-Mfixnum_value(x));
}

obj Mfx_inc(obj x) {
    return Mfixnum(Mfixnum_value(x) + 1);
}

obj Mfx_dec(obj x) {
    return Mfixnum(Mfixnum_value(x) - 1);
}

obj Mfx_add(obj x, obj y) {
    return Mfixnum(Mfixnum_value(x) + Mfixnum_value(y));
}

obj Mfx_sub(obj x, obj y) {
    return Mfixnum(Mfixnum_value(x) - Mfixnum_value(y));
}

obj Mfx_mul(obj x, obj y) {
    return Mfixnum(Mfixnum_value(x) * Mfixnum_value(y));
}

obj Mfx_div(obj x, obj y) {
    return Mfixnum(Mfixnum_value(x) / Mfixnum_value(y));
}

obj Mfx_eq(obj x, obj y) {
    return Mbool(Mfixnum_value(x) == Mfixnum_value(y));
}

obj Mfx_ge(obj x, obj y) {
    return Mbool(Mfixnum_value(x) >= Mfixnum_value(y));
}

obj Mfx_le(obj x, obj y) {
    return Mbool(Mfixnum_value(x) <= Mfixnum_value(y));
}

obj Mfx_gt(obj x, obj y) {
    return Mbool(Mfixnum_value(x) > Mfixnum_value(y));
}

obj Mfx_lt(obj x, obj y) {
    return Mbool(Mfixnum_value(x) < Mfixnum_value(y));
}
