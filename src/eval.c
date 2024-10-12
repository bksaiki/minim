// eval.c: interpreter

#include "minim.h"

static inline int Mimmediatep(obj x) {
    return Mtruep(x)
        || Mfalsep(x)
        || Mfixnump(x);
}

NORETURN void raise_arity_exn(obj prim, obj args) {
    minim_error2(Mprim_name(prim), "arity mismatch", Mfixnum(Mprim_arity(prim)), Mlength(args));
}

static obj do_prim(obj f, obj args) {
    obj (*fn)() = Mprim_value(f);

    switch (Mprim_arity(f))
    {
    case 0:
        if (!Mnullp(args))
            raise_arity_exn(f, args);
        return fn();

    case 1:
        if (Mnullp(args) || !Mnullp(Mcdr(args)))
            raise_arity_exn(f, args);
        return fn(Mcar(args));

    case 2:
        if (Mnullp(args) || Mnullp(Mcdr(args)) || !Mnullp(Mcddr(args)))
            raise_arity_exn(f, args);
        return fn(Mcar(args), Mcadr(args));

    case 3:
        if (Mnullp(args) || Mnullp(Mcdr(args)) || !Mnullp(Mcddr(args)))
            raise_arity_exn(f, args);
        return fn(Mcar(args), Mcadr(args));

    case 4:
        if (Mnullp(args) || Mnullp(Mcdr(args)) || Mnullp(Mcddr(args)) || !Mnullp(Mcdddr(args)))
            raise_arity_exn(f, args);
        return fn(Mcar(args), Mcadr(args));
    
    default:
        if (Mprim_arity(f) < 0) {
            minim_error1("eval_expr", "primitive operations should have fixed arity", f);
        } else {
            minim_error1("eval_expr", "primitive arity unsupported", Mfixnum(Mprim_arity(f)));
        }
    }
}

static obj eval_k(obj e, obj env, obj k) {
    obj x;

loop:
    if (Mconsp(e)) {
        // cons => syntax or application
        obj head = Mcar(e);
        if (head == Mbegin_symbol) {
            // begin
            if (Mnullp(Mcdr(e))) {
                // no expressions => void
                x = Mvoid;
                goto do_k;
            } else if (Mnullp(Mcddr(e))) {
                // one expressions => evaluate in tail position
                e = Mcadr(e);
                goto loop;
            } else {
                // multiple expressions => last expression in tail position
                k = Mseq_continuation(k, Mcddr(e));
                e = Mcadr(e);
                goto loop;
            }
        } else if (head == Mif_symbol) {
            // if
            k = Mcond_continuation(k, Mcaddr(e), Mcar(Mcdddr(e)));
            e = Mcadr(e);
            goto loop;
        } else {
            // application
            k = Mapp_continuation(k, e);
            e = Mcar(e);
            goto loop;
        }
    } else if (Msymbolp(e)) {
        // variable => lookup
        x = env_find(env, e);
        if (Mnullp(x))
            minim_error1("eval_expr", "unbound identifier", e);

        x = Mcdr(x);
        goto do_k;
    } else if (Mimmediatep(e)) {
        // immediate => fully evaluated
        x = e;
        goto do_k;
    } else if (Mnullp(e)) {
        // empty application
        minim_error1("eval_expr", "empty application", e);
    } else {
        minim_error1("eval_expr", "cannot evaluate", e);
    }

    minim_error1("eval_expr", "unreachable", e);

do_app:
    x = Mcar(Mcontinuation_app_hd(k));
    if (Mprimp(x)) {
        x = do_prim(x, Mcdr(Mcontinuation_app_hd(k)));
        k = Mcontinuation_prev(k);
        goto do_k;
    } else {
        minim_error1("eval_expr", "application: not a procedure", x);
    }

do_k:
    switch (Mcontinuation_type(k))
    {
    case NULL_CONT_TYPE:
        // bottom of continuation chain, so exit
        return x;

    case APP_CONT_TYPE:
        // applications
        if (Mcontinuation_app_tl(k)) {
            // evaluated at least the head
            //  `hd` the top of the val/arg list
            //  `tl` car is the last val, cdr is the remaining arguments
            Mcdr(Mcontinuation_app_tl(k)) = Mcons(x, Mcddr(Mcontinuation_app_tl(k)));
            Mcontinuation_app_tl(k) = Mcdr(Mcontinuation_app_tl(k));
        } else {
            // have not evaluated before
            //  `hd` is the top of the argument list
            //  `tl` is NULL
            Mcontinuation_app_hd(k) = Mcons(x, Mcdr(Mcontinuation_app_hd(k)));
            Mcontinuation_app_tl(k) = Mcontinuation_app_hd(k);
        }

        if (Mnullp(Mcdr(Mcontinuation_app_tl(k)))) {
            // evaluated last argument
            goto do_app;
        } else {
            // still evaluating arguments
            e = Mcadr(Mcontinuation_app_tl(k));
            goto loop;
        }

    case COND_CONT_TYPE:
        // if expressions
        e = Mfalsep(x) ? Mcontinuation_cond_iff(k) : Mcontinuation_cond_ift(k);
        k = Mcontinuation_prev(k);
        goto loop;

    case SEQ_CONT_TYPE:
        // begin expressions
        x = Mcontinuation_seq_value(k);
        if (Mnullp(Mcdr(x))) {
            // evaluate last expression in tail position
            k = Mcontinuation_prev(k);
        } else {
            // still evaluating sequence
            Mcontinuation_seq_value(k) = Mcdr(x);
        }

        e = Mcar(x);
        goto loop;

    default:
        minim_error1(
            "eval_expr",
            "unimplemented continuation",
            Mfixnum(Mcontinuation_type(k))
        );
    }
}

obj eval_expr(obj e, obj env) {
    return eval_k(e, env, Mnull_continuation());
}
