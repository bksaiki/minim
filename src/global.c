// global.c: global objects

#include "minim.h"

obj Mnull;
obj Mtrue;
obj Mfalse;
obj Mvoid;
obj Meof;

obj Mbegin_symbol;
obj Mcallcc_symbol;
obj Mif_symbol;
obj Mlambda_symbol;
obj Mlet_symbol;
obj Mquote_symbol;
obj Msetb_symbol;

intern_table *itab;

void minim_init(void) {
    // special values
    Mnull = GC_malloc_uncollectable(sizeof(byte));
    obj_type(Mnull) = SPECIAL_OBJ_TYPE;
    Mtrue = GC_malloc_uncollectable(sizeof(byte));
    obj_type(Mtrue) = SPECIAL_OBJ_TYPE;
    Mfalse = GC_malloc_uncollectable(sizeof(byte));
    obj_type(Mfalse) = SPECIAL_OBJ_TYPE;
    Mvoid = GC_malloc_uncollectable(sizeof(byte));
    obj_type(Mvoid) = SPECIAL_OBJ_TYPE;
    Meof = GC_malloc_uncollectable(sizeof(byte));
    obj_type(Meof) = SPECIAL_OBJ_TYPE;

    // intern table
    itab = make_intern_table();
    GC_add_roots(itab, ptr_add(itab, sizeof(intern_table)));

    // intern symbols
    Mbegin_symbol = Mintern("begin");
    Mcallcc_symbol = Mintern("call/cc");
    Mif_symbol = Mintern("if");
    Mlambda_symbol = Mintern("lambda");
    Mlet_symbol = Mintern("let");
    Mquote_symbol = Mintern("quote");
    Msetb_symbol = Mintern("set!");

    // initialize primitives
    init_prims();
}


NORETURN void minim_shutdown(int code) {
    exit(code);
}
