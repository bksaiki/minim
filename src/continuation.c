// continuation.c: continuations

#include "minim.h"

obj Mnull_continuation(void) {
    obj x = GC_malloc(Mcontinuation_size(0));
    obj_type(x) = CONTINUATON_OBJ_TYPE;
    Mcontinuation_type(x) = NULL_CONT_TYPE;
    Mcontinuation_prev(x) = Mnull;
    return x;
}
