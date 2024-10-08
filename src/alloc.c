// alloc.c: object allocation

#include "minim.h"

obj Msymbol(const char *s) {
    obj x;
    size_t l;

    x = GC_malloc(Msymbol_size);
    obj_type(x) = SYMBOL_OBJ_TYPE;

    l = strlen(s);
    Msymbol_value(x) = GC_malloc_atomic(l + 1);
    strncpy(Msymbol_value(x), s, l + 1);

    return x;
}

obj Mcons(obj car, obj cdr) {
    obj x = GC_malloc(Mcons_size);
    obj_type(x) = CONS_OBJ_TYPE;
    Mcar(x) = car;
    Mcdr(x) = cdr;
    return x;
}

obj Mfixnum(iptr v) {
    obj x = GC_malloc(Mfixnum_size);
    obj_type(x) = FIXNUM_OBJ_TYPE;
    Mfixnum_value(x) = v;
    return x;
}
