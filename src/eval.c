// eval.c: interpreter

#include "minim.h"

int Mimmediatep(obj x) {
    return Mtruep(x)
        || Mfalsep(x)
        || Mfixnump(x)
        || Mcharp(x)
        || Mstringp(x);
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

static obj do_closure(obj f, obj args) {
    obj env, it;
    iptr arity, argc;

    arity = Mclosure_arity(f);
    argc = list_length(args);
    if (arity < 0) {
        arity = -arity - 1;
        if (argc < arity) {
            minim_error2(
                Mclosure_name(f),
                "arity mismatch",
                Mlist2(Mintern("at-least"), Mfixnum(arity)),
                Mfixnum(argc)
            );
        }
    } else if (arity != argc) {
        minim_error2(
            Mclosure_name(f),
            "arity mismatch",
            Mfixnum(arity),
            Mfixnum(argc)
        );
    }

    env = env_extend(Mclosure_env(f));
    it = Mclosure_formals(f);
    while (Mconsp(it)) {
        env_insert(env, Mcar(it), Mcar(args));
        args = Mcdr(args);
        it = Mcdr(it);
    }

    if (!Mnullp(it)) {
        env_insert(env, it, args);
    }

    return env;
}

static obj do_arg(obj tc, obj x) {
    obj k = continuation_mutable(Mtc_cc(tc));
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

    return k;
}

static obj do_begin(obj tc) {
    obj x, k;

    k = continuation_mutable(Mtc_cc(tc));
    x = Mcontinuation_seq_value(k);
    Mtc_env(tc) = Mcontinuation_env(k);
    if (Mnullp(Mcdr(x))) {
        // evaluate last expression in tail position
        Mtc_cc(tc) = Mcontinuation_prev(k);
    } else {
        // still evaluating sequence
        Mcontinuation_seq_value(k) = Mcdr(x);
        Mtc_cc(tc) = k;
    }

    return Mcar(x);
}

static obj do_let(obj tc, obj v) {
    obj k, env, binds;

    k = continuation_mutable(Mtc_cc(tc));
    env = Mcontinuation_let_env(k);
    binds = Mcontinuation_let_bindings(k);
    
    // add result using first binding
    env_insert(env, Mcaar(binds), v);

    binds = Mcdr(binds);
    if (Mnullp(binds)) {
        // no more bindings => evaluate body in tail position
        Mtc_env(tc) = env;
        Mtc_cc(tc) = Mcontinuation_prev(k);
        return Mcontinuation_let_body(k);
    } else {
        // at least one more binding
        Mcontinuation_let_bindings(k) = binds;
        Mtc_env(tc) = Mcontinuation_env(k);
        Mtc_cc(tc) = k;
        return Mcadar(binds);
    }
}

static void do_setb(obj tc, obj x, obj v) {
    obj env, cell;

    env = Mcontinuation_env(Mtc_cc(tc));
    cell = env_find(env, x);
    if (Mfalsep(cell)) {
        minim_error1("set!", "unbound variable", x);
    } else {
        Mcdr(cell) = v;
    }
}

static void check_callcc(obj f) {
    if (Mprimp(f)) {
        iptr arity = Mprim_arity(f);
        if (arity < 0 ? arity == -1 : arity != 1)
            minim_error1("call/cc", "expected a procedure of at least 1 argument", f);
    } else if (Mclosurep(f)) {
        iptr arity = Mclosure_arity(f);
        if (arity < 0 ? arity == -1 : arity != 1)
            minim_error1("call/cc", "expected a procedure of at least 1 argument", f);
    } else if (Mcontinuationp(f)) {
        // do nothing
    } else {
        minim_error1("call/cc", "expected a procedure", f);
    }
}

static obj eval_k(obj e) {
    obj tc, x, hd, f, args;

    tc = Mcurr_tc();

loop:

    if (Mconsp(e)) {
        // cons => syntax or application
        hd = Mcar(e);
        if (hd == Mlet_symbol) {
            // let
            Mtc_cc(tc) = Mlet_continuation(Mtc_cc(tc), Mtc_env(tc), Mcadr(e), Mcaddr(e));
            e = Mcadar(Mcontinuation_let_bindings(Mtc_cc(tc)));
            goto loop;
        } else if (hd == Mbegin_symbol) {
            // begin
            Mtc_cc(tc) = Mseq_continuation(Mtc_cc(tc), Mtc_env(tc), Mcddr(e));
            e = Mcadr(e);
            goto loop;
        } else if (hd == Mif_symbol) {
            // if
            Mtc_cc(tc) = Mcond_continuation(Mtc_cc(tc), Mtc_env(tc), Mcaddr(e), Mcar(Mcdddr(e)));
            e = Mcadr(e);
            goto loop;
        } else if (hd == Mlambda_symbol) {
            // lambda
            x = Mclosure(Mtc_env(tc), Mcadr(e), Mcaddr(e));
            goto do_k;
        } else if (hd == Msetb_symbol) {
            // set!
            Mtc_cc(tc) = Msetb_continuation(Mtc_cc(tc), Mtc_env(tc), Mcadr(e));
            e = Mcaddr(e);
            goto loop;
        } else if (hd == Mquote_symbol) {
            // quote
            x = Mcadr(e);
            goto do_k;
        } else if (hd == Mcallcc_symbol) {
            // call/cc
            continuation_set_immutable(Mtc_cc(tc)); // freeze the continuation chain
            Mtc_cc(tc) = Mcallcc_continuation(Mtc_cc(tc), Mtc_env(tc));
            e = Mcadr(e);
            goto loop;
        } else {
            // application
            Mtc_cc(tc) = Mapp_continuation(Mtc_cc(tc), Mtc_env(tc), e);
            e = Mcar(e);
            goto loop;
        }
    } else if (Msymbolp(e)) {
        // variable => lookup
        x = env_find(Mtc_env(tc), e);
        if (Mconsp(x)) {
            x = Mcdr(x);
            goto do_k;
        } else if (Mfalsep(x)) {
            minim_error(Msymbol_value(e), "unbound identifier");
        } else if (Munboundp(x)) {
            minim_error(Msymbol_value(e), "uninitialized identifier");
        } else {
            minim_error1("eval_expr", "unreachable", e);
        }
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
    if (Mprimp(f)) {
        x = do_prim(f, args);
        Mtc_cc(tc) = Mcontinuation_prev(Mtc_cc(tc));
        goto do_k;
    } else if (Mclosurep(f)) {
        e = Mclosure_body(f);
        Mtc_env(tc) = do_closure(f, args);
        Mtc_cc(tc) = Mcontinuation_prev(Mtc_cc(tc));
        goto loop;
    } else if (Mcontinuationp(f)) {
        Mtc_cc(tc) = continuation_restore(Mtc_cc(tc), f);
        goto do_k;
    } else {
        minim_error1("eval_expr", "application: not a procedure", x);
    }

do_k:
    switch (Mcontinuation_type(Mtc_cc(tc))) {
    // bottom of continuation chain, so exit
    case NULL_CONT_TYPE:
        return x;

    // applications
    case APP_CONT_TYPE:
        Mtc_cc(tc) = do_arg(tc, x);
        Mtc_env(tc) = Mcontinuation_env(Mtc_cc(tc));
        if (Mnullp(Mcdr(Mcontinuation_app_tl(Mtc_cc(tc))))) {
            // evaluated last argument
            f = Mcar(Mcontinuation_app_hd(Mtc_cc(tc)));
            args = Mcdr(Mcontinuation_app_hd(Mtc_cc(tc)));
            goto do_app;
        } else {
            // still evaluating arguments
            e = Mcadr(Mcontinuation_app_tl(Mtc_cc(tc)));
            goto loop;
        }

    // if expressions
    case COND_CONT_TYPE:
        Mtc_cc(tc) = continuation_mutable(Mtc_cc(tc));
        e = Mfalsep(x) ? Mcontinuation_cond_iff(Mtc_cc(tc)) : Mcontinuation_cond_ift(Mtc_cc(tc));
        Mtc_env(tc) = Mcontinuation_env(Mtc_cc(tc));
        Mtc_cc(tc) = Mcontinuation_prev(Mtc_cc(tc));
        goto loop;

    // begin expressions
    case SEQ_CONT_TYPE:
        e = do_begin(tc);
        goto loop;
    
    // let expressions
    case LET_CONT_TYPE:
        e = do_let(tc, x);
        goto loop;

    // set! expressions
    case SETB_CONT_TYPE:
        // update binding, result is void
        do_setb(tc, Mcontinuation_setb_name(Mtc_cc(tc)), x);
        x = Mvoid;
        Mtc_cc(tc) = Mcontinuation_prev(Mtc_cc(tc));
        goto do_k;
    
    // call/cc expressions
    case CALLCC_CONT_TYPE:
        check_callcc(x);
        f = x;
        args = Mlist1(Mcontinuation_prev(Mtc_cc(tc)));
        Mtc_env(tc) = Mcontinuation_env(Mtc_cc(tc));
        goto do_app;

    // unknown
    default:
        minim_error1("eval_expr", "unimplemented", Mfixnum(Mcontinuation_type(Mtc_cc(tc))));
    }
}

obj eval_expr(obj e) {
    obj tc, k, env, v;

    // stash old continuation and environment
    tc = Mcurr_tc();
    k = Mtc_cc(tc);
    env = Mtc_env(tc);

    check_expr(e);
    e = expand_expr(e);
    Mtc_cc(tc) = Mnull_continuation(env);
    v = eval_k(e);

    // restore old continuation and environment
    Mtc_cc(tc) = k;
    Mtc_env(tc) = env;
    return v;
}
