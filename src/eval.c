// eval.c: interpreter

#include "minim.h"

static inline int Mimmediatep(obj x) {
    return Mnullp(x)
        || Mtruep(x)
        || Mfalsep(x)
        || Mfixnump(x);
}

static obj eval_k(obj e, obj env, obj k) {
    obj x;

    if (Mimmediatep(e)) {
        x = e;
        goto do_k;
    } else if (Msymbolp(e)) {
        x = env_find(env, e);
        if (Mnullp(x))
            minim_error1("eval_expr", "unbound identifier", e);

        x = Mcdr(x);
        goto do_k;
    } else {
        minim_error1("eval_expr", "cannot evaluate", e);
    }

do_k:
    switch (Mcontinuation_type(k))
    {
    case NULL_CONT_TYPE:
        // bottom of continuation chain, so exit
        return x;
    
    default:
        minim_error1("eval_expr", "unimplemented continuation", k);
    }
}

obj eval_expr(obj e, obj env) {
    return eval_k(e, env, Mnull_continuation());
}
