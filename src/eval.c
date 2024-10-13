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

static obj eval_k(obj e, obj env, obj k) {
    obj x, hd, f, args;

loop:

    if (Mconsp(e)) {
        // cons => syntax or application
        hd = Mcar(e);
        if (hd == Mlet_symbol) {
            // let
            k = Mlet_continuation(k, env, Mcadr(e), Mcaddr(e));
            e = Mcadar(Mcontinuation_let_bindings(k));
            goto loop;
        } else if (hd == Mbegin_symbol) {
            // begin
            k = Mseq_continuation(k, env, Mcddr(e));
            e = Mcadr(e);
            goto loop;
        } else if (hd == Mif_symbol) {
            // if
            k = Mcond_continuation(k, env, Mcaddr(e), Mcar(Mcdddr(e)));
            e = Mcadr(e);
            goto loop;
        } else if (hd == Mlambda_symbol) {
            // lambda
            x = Mclosure(env, Mcadr(e), Mcaddr(e));
            goto do_k;
        } else if (hd == Msetb_symbol) {
            // set!
            k = Msetb_continuation(k, env, Mcadr(e));
            e = Mcaddr(e);
            goto loop;
        } else if (hd == Mquote_symbol) {
            // quote
            x = Mcadr(e);
            goto do_k;
        } else if (hd == Mcallcc_symbol) {
            // call/cc
            k = Mcallcc_continuation(k, env);
            e = Mcadr(e);
            goto loop;
        } else {
            // application
            k = Mapp_continuation(k, env, e);
            e = Mcar(e);
            goto loop;
        }
    } else if (Msymbolp(e)) {
        // variable => lookup
        x = env_find(env, e);
        if (Mfalsep(x)) {
            minim_error(Msymbol_value(e), "unbound identifier");
        } else if (Munboundp(x)) {
            minim_error(Msymbol_value(e), "uninitialized identifier");
        }

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
    if (Mprimp(f)) {
        x = do_prim(f, args);
        k = Mcontinuation_prev(k);
        goto do_k;
    } else if (Mclosurep(f)) {
        e = Mclosure_body(f);
        env = do_closure(f, args);
        k = Mcontinuation_prev(k);
        goto loop;
    } else if (Mcontinuationp(f)) {
        k = f;
        goto do_k;
    } else {
        minim_error1("eval_expr", "application: not a procedure", x);
    }

do_k:
    switch (Mcontinuation_type(k)) {
    // bottom of continuation chain, so exit
    case NULL_CONT_TYPE:
        return x;

    // applications
    case APP_CONT_TYPE:
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

        env = Mcontinuation_env(k);
        if (Mnullp(Mcdr(Mcontinuation_app_tl(k)))) {
            // evaluated last argument
            f = Mcar(Mcontinuation_app_hd(k));
            args = Mcdr(Mcontinuation_app_hd(k));
            goto do_app;
        } else {
            // still evaluating arguments
            e = Mcadr(Mcontinuation_app_tl(k));
            goto loop;
        }

    // if expressions
    case COND_CONT_TYPE:
        e = Mfalsep(x) ? Mcontinuation_cond_iff(k) : Mcontinuation_cond_ift(k);
        env = Mcontinuation_env(k);
        k = Mcontinuation_prev(k);
        goto loop;

    // begin expressions
    case SEQ_CONT_TYPE:
        x = Mcontinuation_seq_value(k);
        env = Mcontinuation_env(k);
        if (Mnullp(Mcdr(x))) {
            // evaluate last expression in tail position
            k = Mcontinuation_prev(k);
        } else {
            // still evaluating sequence
            Mcontinuation_seq_value(k) = Mcdr(x);
        }

        e = Mcar(x);
        goto loop;
    
    // let expressions
    case LET_CONT_TYPE:
        // add result using first binding
        env_insert(
            Mcontinuation_let_env(k),
            Mcaar(Mcontinuation_let_bindings(k)),
            x
        );

        Mcontinuation_let_bindings(k) = Mcdr(Mcontinuation_let_bindings(k));
        if (Mnullp(Mcontinuation_let_bindings(k))) {
            // no more bindings => evaluate body in tail position
            e = Mcontinuation_let_body(k);
            env = Mcontinuation_let_env(k);
            k = Mcontinuation_prev(k);  
        } else {
            // at least one more binding
            e = Mcadar(Mcontinuation_let_bindings(k));
            env = Mcontinuation_env(k);
        }

        goto loop;

    // set! expressions
    case SETB_CONT_TYPE:
        env = Mcontinuation_env(k);
        e = env_find(env, Mcontinuation_setb_name(k));
        if (Mfalsep(x)) {
            minim_error1("set!", "unbound variable", Mcontinuation_setb_name(k));
        } else if (Munboundp(x)) {
            minim_error1("set!", "uninitialized variable", Mcontinuation_setb_name(k));   
        }

        // update binding, result is void
        Mcdr(e) = x;
        x = Mvoid;
        k = Mcontinuation_prev(k);
        goto do_k;
    
    // call/cc expressions
    case CALLCC_CONT_TYPE:
        if (Mcontinuation_callcc_frozenp(k)) {
            // exiting through the site of a captured continuation
            k = continuation_restore(Mcontinuation_prev(k));
            goto do_k;
        } else {
            // entering a procedure for call/cc for the first time
            if (Mprimp(x)) {
                iptr arity = Mprim_arity(x);
                if (arity < 0 ? arity == -1 : arity != 1)
                    minim_error1("call/cc", "expected a procedure of at least 1 argument", x);

                x = do_prim(x, Mlist1(k));
                goto do_k;
            } else if (Mclosurep(x)) {
                iptr arity = Mclosure_arity(x);
                if (arity < 0 ? arity == -1 : arity != 1)
                    minim_error1("call/cc", "expected a procedure of at least 1 argument", x);
                
                Mcontinuation_callcc_frozenp(k) = 1;
                env = do_closure(x, Mlist1(k));
                e = Mclosure_body(x);
                goto loop;
            } else if (Mcontinuationp(x)) {
                goto do_k;
            } else {
                minim_error1("call/cc", "expected a procedure", x);
            }
        }

    // unknown
    default:
        minim_error1("eval_expr", "unimplemented", Mfixnum(Mcontinuation_type(k)));
    }
}

obj eval_expr(obj e, obj env) {
    check_expr(e);
    e = expand_expr(e);
    return eval_k(e, env, Mnull_continuation(env));
}
