// global.c: global objects

#include "minim.h"

obj Mnull;
obj Mtrue;
obj Mfalse;
obj Mvoid;
obj Meof;
obj Mvalues;
obj Munbound;

obj Mbegin_symbol;
obj Mcallcc_symbol;
obj Mcallwv_symbol;
obj Mdynwind_symbol;
obj Mif_symbol;
obj Mlambda_symbol;
obj Mlet_symbol;
obj Mlet_values_symbol;
obj Mletrec_symbol;
obj Mletrec_values_symbol;
obj Mquote_symbol;
obj Msetb_symbol;

intern_table *itab;
obj *Mcurr_tc_box;

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
    Mvalues = GC_malloc_uncollectable(sizeof(byte));
    obj_type(Mvalues) = SPECIAL_OBJ_TYPE;
    Munbound = GC_malloc_uncollectable(sizeof(byte));
    obj_type(Munbound) = SPECIAL_OBJ_TYPE;

    // intern table
    itab = make_intern_table();
    GC_add_roots(itab, ptr_add(itab, sizeof(intern_table)));

    // intern symbols
    Mbegin_symbol = Mintern("begin");
    Mcallcc_symbol = Mintern("call/cc");
    Mcallwv_symbol = Mintern("call-with-values");
    Mdynwind_symbol = Mintern("dynamic-wind");
    Mif_symbol = Mintern("if");
    Mlambda_symbol = Mintern("lambda");
    Mlet_symbol = Mintern("let");
    Mlet_values_symbol = Mintern("let-values");
    Mletrec_symbol = Mintern("letrec");
    Mletrec_values_symbol = Mintern("letrec-values");
    Mquote_symbol = Mintern("quote");
    Msetb_symbol = Mintern("set!");

    // initialize thread context
    Mcurr_tc_box = GC_malloc_uncollectable(sizeof(obj));
    Mcurr_tc() = Mthread_context();

    // initialize primitives
    init_prims();
}


NORETURN void minim_shutdown(int code) {
    GC_deinit();
    exit(code);
}
