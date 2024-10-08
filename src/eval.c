// eval.c: interpreter

#include "minim.h"

static obj eval_k(obj e, obj env, obj k) {
    obj x;

    if (Mnullp(e) || Mtruep(e) || Mfalsep(e)) {
        x = e;
        goto do_k;
    } else if (Mfixnump(e)) {
        x = e;
        goto do_k;
    } else {
        fprintf(stderr, "cannot evaluate expression: ");
        write_obj(stderr, e);
        fprintf(stderr, "\n");
        minim_shutdown(1);
    }

do_k:
    switch (Mcontinuation_type(k))
    {
    case NULL_CONT_TYPE:
        // bottom of continuation chain, so exit
        return x;
    
    default:
        fprintf(stderr, "unimplemented for continuation type %d\n", Mcontinuation_type(k));
        minim_shutdown(1);
        break;
    }
}

obj eval_expr(obj e, obj env) {
    return eval_k(e, env, Mnull_continuation());
}
