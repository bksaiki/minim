// continuation.c: continuations

#include "minim.h"

obj Mnull_continuation(void) {
    obj x = GC_malloc(Mcontinuation_size(0));
    obj_type(x) = CONTINUATON_OBJ_TYPE;
    Mcontinuation_type(x) = NULL_CONT_TYPE;
    Mcontinuation_prev(x) = Mnull;
    return x;
}

obj Mapp_continuation(obj prev, obj args) {
    obj x = GC_malloc(Mcontinuation_size(3));
    obj_type(x) = CONTINUATON_OBJ_TYPE;
    Mcontinuation_type(x) = APP_CONT_TYPE;
    Mcontinuation_prev(x) = prev;
    Mcontinuation_app_hd(x) = args;
    Mcontinuation_app_tl(x) = NULL;
    return x;
}

obj Mcond_continuation(obj prev, obj ift, obj iff) {
    obj x = GC_malloc(Mcontinuation_size(3));
    obj_type(x) = CONTINUATON_OBJ_TYPE;
    Mcontinuation_type(x) = COND_CONT_TYPE;
    Mcontinuation_prev(x) = prev;
    Mcontinuation_cond_ift(x) = ift;
    Mcontinuation_cond_iff(x) = iff;
    return x;
}
