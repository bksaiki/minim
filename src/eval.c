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

static void clear_values_buffer(obj tc) {
    Mtc_vc(tc) = 0;
}

static void assert_single_value(obj tc, obj x) {
    if (Mvaluesp(x)) {
        minim_error2(NULL, "values mismatch", Mfixnum(1), Mfixnum(Mtc_vc(tc)));
    }
}

// performs `values` primitive
// if there is only 1 argument, the argument is returned
// otherwise, the arguments are written to the values buffer
static obj do_values(obj args) {
    obj tc;
    iptr vc, i;

    // check if safe to write to buffer
    tc = Mcurr_tc();
    if (Mtc_vc(tc) != 0) {
        minim_error1("do_values()", "values buffer is not empty", args);
    }

    // check if can just return the values
    vc = list_length(args);
    if (vc == 1) {
        return Mcar(args);
    } else { 
        // check if we need to reallocate
        if (vc >= Mtc_va(tc)) {
            Mtc_va(tc) = 2 * vc;
            Mtc_vb(tc) = GC_malloc(Mtc_va(tc) * sizeof(obj));
        }

        // fill the buffer
        Mtc_vc(tc) = vc;
        for (i = 0; i < vc; i++) {
            Mtc_vb(tc)[i] = Mcar(args);
            args = Mcdr(args);
        }

        return Mvalues;
    }
}

// converts the values buffer to a list
static obj values_to_list(void) {
    obj tc, hd, tl;
    uptr vc, i;

    tc = Mcurr_tc();
    vc = Mtc_vc(tc);
    switch (vc)
    {
    case 0:
        // null values
        return Mnull;
    
    case 1:
        // single values
        return Mlist1(Mtc_vb(tc)[0]);

    default:
        // multiple values
        hd = tl = Mlist1(Mtc_vb(tc)[0]);
        for (i = 1; i < vc; ++i) {
            Mcdr(tl) = Mlist1(Mtc_vb(tc)[i]);
            tl = Mcdr(tl);
        }

        return hd;
    }
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

static void check_let_arity(obj tc, obj ids, obj v) {
    iptr len, expect;

    expect = Mvaluesp(v) ? Mtc_vc(tc) : 1;
    len = list_length(ids);
    if (len != expect) {
        minim_error2(NULL, "result arity mismatch", Mfixnum(len), Mfixnum(expect));
    }
}

static obj do_let(obj tc, obj v) {
    obj k, env, binds, ids;

    k = Mtc_cc(tc);
    env = Mcontinuation_let_env(k);
    binds = Mcontinuation_let_bindings(k);

    // check that we have enough ids
    ids = Mcaar(binds);
    check_let_arity(tc, ids, v);

    // bind values
    if (Mvaluesp(v)) {
        v = values_to_list();
        clear_values_buffer(tc);
        for (; !Mnullp(ids); ids = Mcdr(ids), v = Mcdr(v)) {
            if (Mclosurep(Mcar(v)) && Mfalsep(Mclosure_name(Mcar(v))))
                Mclosure_name(Mcar(v)) = Mcar(ids);
            env_insert(env, Mcar(ids), Mcar(v));
        }
    } else {
        if (Mclosurep(v) && Mfalsep(Mclosure_name(v)))
            Mclosure_name(v) = Mcar(ids);
        env_insert(env, Mcar(ids), v);
    }

    binds = Mcdr(binds);
    if (Mnullp(binds)) {
        // no more bindings => evaluate body in tail position
        Mtc_env(tc) = env;
        Mtc_cc(tc) = Mcontinuation_prev(k);
        return Mcontinuation_let_body(k);
    } else {
        // at least one more binding
        k = continuation_mutable(k);
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
        if (Mclosurep(v) && Mfalsep(Mclosure_name(v)))
            Mclosure_name(v) = x;
        Mcdr(cell) = v;
    }
}

static void check_callcc(obj f) {
    if (Mprimp(f)) {
        iptr arity = Mprim_arity(f);
        if (arity < 0 ? arity < -2 : arity != 1)
            minim_error1("call/cc", "expected a procedure of at least 1 argument", f);
    } else if (Mclosurep(f)) {
        iptr arity = Mclosure_arity(f);
        if (arity < 0 ? arity < -2 : arity != 1)
            minim_error1("call/cc", "expected a procedure of at least 1 argument", f);
    } else if (Mcontinuationp(f)) {
        // do nothing
    } else {
        minim_error1("call/cc", "expected a procedure", f);
    }
}

static void assert_thunk(const char *name, obj f) {
    if (Mprimp(f)) {
        iptr arity = Mprim_arity(f);
        if (arity < 0 ? arity != -1 : arity != 0)
            minim_error1(name, "expected a procedure of at least 0 argument", f);
    } else if (Mclosurep(f)) {
        iptr arity = Mclosure_arity(f);
        if (arity < 0 ? arity < -1 : arity != 0)
            minim_error1(name, "expected a procedure of at least 0 argument", f);
    } else if (Mcontinuationp(f)) {
        // do nothing
    } else {
        minim_error1(name, "expected a procedure", f);
    }
}

static obj eval_k(obj e) {
    obj tc, x, hd, f, args;

    tc = Mcurr_tc();

loop:

    if (Mconsp(e)) {
        // cons => syntax or application
        hd = Mcar(e);
        if (hd == Mlet_values_symbol) {
            // let-values
            // TODO: optimize for unbound
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
            Mtc_cc(tc) = Mcallcc_continuation(Mtc_cc(tc), Mtc_env(tc), Mtc_wnd(tc));
            e = Mcadr(e);
            goto loop;
        } else if (hd == Mcallwv_symbol) {
            // call-with-values
            Mtc_cc(tc) = Mcallwv_continuation(Mtc_cc(tc), Mtc_env(tc), Mcaddr(e));
            e = Mcadr(e);
            goto loop;
        } else if (hd == Mdynwind_symbol) {
            // dynamic-wind
            Mtc_cc(tc) = Mdynwind_continuation(Mtc_cc(tc), Mtc_env(tc), Mcaddr(e), Mcar(Mcdddr(e)));
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
        if (f == values_prim) {
            x = do_values(args);
        } else {
            x = do_prim(f, args);
        }

        goto do_k;
    } else if (Mclosurep(f)) {
        e = Mclosure_body(f);
        Mtc_env(tc) = do_closure(f, args);
        goto loop;
    } else if (Mcontinuationp(f)) {
        Mtc_cc(tc) = continuation_restore(tc, f);
        x = do_values(args);
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
        assert_single_value(Mtc_cc(tc), x);
        Mtc_cc(tc) = do_arg(tc, x);
        Mtc_env(tc) = Mcontinuation_env(Mtc_cc(tc));
        if (Mnullp(Mcdr(Mcontinuation_app_tl(Mtc_cc(tc))))) {
            // evaluated last argument
            f = Mcar(Mcontinuation_app_hd(Mtc_cc(tc)));
            args = Mcdr(Mcontinuation_app_hd(Mtc_cc(tc)));
            Mtc_cc(tc) = Mcontinuation_prev(Mtc_cc(tc));
            goto do_app;
        } else {
            // still evaluating arguments
            e = Mcadr(Mcontinuation_app_tl(Mtc_cc(tc)));
            goto loop;
        }

    // if expressions
    case COND_CONT_TYPE:
        assert_single_value(Mtc_cc(tc), x);
        Mtc_cc(tc) = continuation_mutable(Mtc_cc(tc));
        e = Mfalsep(x) ? Mcontinuation_cond_iff(Mtc_cc(tc)) : Mcontinuation_cond_ift(Mtc_cc(tc));
        Mtc_env(tc) = Mcontinuation_env(Mtc_cc(tc));
        Mtc_cc(tc) = Mcontinuation_prev(Mtc_cc(tc));
        goto loop;

    // begin expressions
    case SEQ_CONT_TYPE:
        clear_values_buffer(tc);
        e = do_begin(tc);
        goto loop;
    
    // let expressions
    case LET_CONT_TYPE:
        e = do_let(tc, x);
        goto loop;

    // set! expressions
    case SETB_CONT_TYPE:
        // update binding, result is void
        assert_single_value(Mtc_cc(tc), x);
        do_setb(tc, Mcontinuation_setb_name(Mtc_cc(tc)), x);
        x = Mvoid;
        Mtc_cc(tc) = Mcontinuation_prev(Mtc_cc(tc));
        goto do_k;
    
    // call/cc expressions
    case CALLCC_CONT_TYPE:
        if (Mcontinuation_capturedp(Mtc_cc(tc))) {
            // restoring captured continuation
            Mtc_wnd(tc) = Mcontinuation_callcc_winders(Mtc_cc(tc));
            Mtc_cc(tc) = Mcontinuation_prev(Mtc_cc(tc));
            goto do_k;
        } else {
            // capturing current continuation
            assert_single_value(Mtc_cc(tc), x);
            check_callcc(x);

            Mtc_cc(tc) = continuation_mutable(Mtc_cc(tc));
            Mcontinuation_capturedp(Mtc_cc(tc)) = 1;

            f = x;
            args = Mlist1(Mtc_cc(tc));
            Mtc_env(tc) = Mcontinuation_env(Mtc_cc(tc));
            Mtc_cc(tc) = Mcontinuation_prev(Mtc_cc(tc));
            goto do_app;
        }
    
    // call-with-values expressions
    case CALLWV_CONT_TYPE:
        Mtc_cc(tc) = continuation_mutable(Mtc_cc(tc));
        if (Mfalsep(Mcontinuation_callwv_producer(Mtc_cc(tc)))) {
            // evaluated producer syntax
            assert_single_value(Mtc_cc(tc), x);
            assert_thunk("call-with-values", x);

            Mcontinuation_callwv_producer(Mtc_cc(tc)) = x;
            e = Mcontinuation_callwv_consumer(Mtc_cc(tc));
            Mtc_env(tc) = Mcontinuation_env(Mtc_cc(tc));
            goto loop;
        } else if (!Mprocp(Mcontinuation_callwv_consumer(Mtc_cc(tc)))) {
            // evaluated consumer syntax
            assert_single_value(Mtc_cc(tc), x);
            if (!Mprocp(x)) {
                minim_error1("call-with-values", "expected a procedure", x);
            }

            Mcontinuation_callwv_consumer(Mtc_cc(tc)) = x;
            f = Mcontinuation_callwv_producer(Mtc_cc(tc));
            args = Mnull;
    
            Mtc_env(tc) = Mcontinuation_env(Mtc_cc(tc));
            goto do_app;
        } else {
            // evaluated producer procedure
            f = Mcontinuation_callwv_consumer(Mtc_cc(tc));
            if (Mvaluesp(x)) {
                args = values_to_list();
                clear_values_buffer(tc);
            } else {
                args = Mlist1(x);
            }

            Mtc_env(tc) = Mcontinuation_env(Mtc_cc(tc));
            Mtc_cc(tc) = Mcontinuation_prev(Mtc_cc(tc));
            goto do_app;
        }

    // dynamic-wind expressions
    case DYNWIND_CONT_TYPE:
        Mtc_cc(tc) = continuation_mutable(Mtc_cc(tc));
        switch (Mcontinuation_dynwind_state(Mtc_cc(tc))) {
        // unevaluated dynamic-wind
        case DYNWIND_NEW:
            if (!Mprocp(Mcontinuation_dynwind_pre(Mtc_cc(tc)))) {
                // evaluating pre thunk expression
                assert_single_value(Mtc_cc(tc), x);
                assert_thunk("dynamic-wind", x);
                Mcontinuation_dynwind_pre(Mtc_cc(tc)) = x;

                Mtc_env(tc) = Mcontinuation_env(Mtc_cc(tc));
                e = Mcontinuation_dynwind_val(Mtc_cc(tc));
                goto loop;
            } else if (!Mprocp(Mcontinuation_dynwind_val(Mtc_cc(tc)))) {
                // evaluating value thunk expression
                assert_single_value(Mtc_cc(tc), x);
                assert_thunk("dynamic-wind", x);
                Mcontinuation_dynwind_val(Mtc_cc(tc)) = x;

                Mtc_env(tc) = Mcontinuation_env(Mtc_cc(tc));
                e = Mcontinuation_dynwind_post(Mtc_cc(tc));
                goto loop;
            } else {
                // evaluating post thunk expression
                assert_single_value(Mtc_cc(tc), x);
                assert_thunk("dynamic-wind", x);
                Mcontinuation_dynwind_post(Mtc_cc(tc)) = x;
                Mcontinuation_dynwind_state(Mtc_cc(tc)) = DYNWIND_PRE;

                Mtc_env(tc) = Mcontinuation_env(Mtc_cc(tc));
                f = Mcontinuation_dynwind_pre(Mtc_cc(tc));
                args = Mnull;
                goto do_app;
            }

        // evaluated pre thunk
        case DYNWIND_PRE:
            clear_values_buffer(tc);
            Mcontinuation_dynwind_state(Mtc_cc(tc)) = DYNWIND_VAL;
            Mtc_wnd(tc) = Mcons(
                Mcons(
                    Mcontinuation_dynwind_pre(Mtc_cc(tc)),
                    Mcontinuation_dynwind_post(Mtc_cc(tc))
                ),
                Mtc_wnd(tc)
            );
    
            Mtc_env(tc) = Mcontinuation_env(Mtc_cc(tc));
            f = Mcontinuation_dynwind_val(Mtc_cc(tc));
            args = Mnull;
            goto do_app;

        // evaluated val thunk
        case DYNWIND_VAL:
            if (Mvaluesp(x)) {
                Mcontinuation_dynwind_val(Mtc_cc(tc)) = values_to_list();
                clear_values_buffer(tc);
            } else {
                Mcontinuation_dynwind_val(Mtc_cc(tc)) = Mlist1(x);
            }
            
            Mtc_wnd(tc) = Mcdr(Mtc_wnd(tc));
            Mcontinuation_dynwind_state(Mtc_cc(tc)) = DYNWIND_POST;
            Mtc_env(tc) = Mcontinuation_env(Mtc_cc(tc));
            f = Mcontinuation_dynwind_post(Mtc_cc(tc));
            args = Mnull;
            goto do_app;

        // evaluated post thunk
        case DYNWIND_POST:
            clear_values_buffer(tc);
            x = do_values(Mcontinuation_dynwind_val(Mtc_cc(tc)));
            Mtc_env(tc) = Mcontinuation_env(Mtc_cc(tc));
            Mtc_cc(tc) = Mcontinuation_prev(Mtc_cc(tc));
            goto do_k;
        
        default:
            minim_error("dynamic-wind", "unimplemented");
        }

    // reinstalling winders
    case WINDERS_CONT_TYPE:
        if (Mnullp(Mcontinuation_winders_it(Mtc_cc(tc)))) {
            // no more winders to execute
            x = do_values(Mcontinuation_winders_values(Mtc_cc(tc)));
            Mtc_env(tc) = Mcontinuation_env(Mtc_cc(tc));
            Mtc_cc(tc) = Mcontinuation_prev(Mtc_cc(tc));
            goto do_k;
        } else {
            // need to execute a winder
            if (Mfalsep(Mcontinuation_winders_values(Mtc_cc(tc)))) {
                // first time processing winders
                Mtc_cc(tc) = continuation_mutable(Mtc_cc(tc));
                if (Mvaluesp(x)) {
                    Mcontinuation_winders_values(Mtc_cc(tc)) = values_to_list();
                    clear_values_buffer(Mtc_cc(tc));
                } else {
                    Mcontinuation_winders_values(Mtc_cc(tc)) = Mlist1(x);
                }
            }

            // Mtc_env(tc) = Mcontinuation_env(Mtc_cc(tc));
            f = Mcar(Mcontinuation_winders_it(Mtc_cc(tc)));
            args = Mnull;

            Mcontinuation_winders_it(Mtc_cc(tc)) = Mcdr(Mcontinuation_winders_it(Mtc_cc(tc)));
            goto do_app;
        }

    // unknown
    default:
        minim_error1(
            "eval_expr",
            "unimplemented continuation handler",
            Mfixnum(Mcontinuation_type(Mtc_cc(tc)))
        );
    }
}

obj eval_expr(obj e) {
    obj tc, k, env, v;

    // stash old continuation and environment
    tc = Mcurr_tc();
    k = Mtc_cc(tc);
    env = Mtc_env(tc);

    // evaluate
    check_expr(e);
    e = expand_expr(e);
    Mtc_cc(tc) = Mnull_continuation(env);
    v = eval_k(e);

    // restore old continuation and environment
    Mtc_cc(tc) = k;
    Mtc_env(tc) = env;
    return v;
}
