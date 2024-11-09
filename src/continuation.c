// continuation.c: support for continuations

#include "minim.h"

// Marks a continuation chain as immutable.
// Unwinding through an immutable continuation chain requires
// copying each continuation as needed.
void continuation_set_immutable(obj k) {
    while (Mcontinuationp(k) && !Mcontinuation_immutablep(k)) {
        Mcontinuation_immutablep(k) = 1;
        k = Mcontinuation_prev(k);
    }
}

// Safely returns a mutable version of the current continuation.
// If the continuation chain is immutable, a copy is made.
// Otherwise, the argument is returned.
obj continuation_mutable(obj k) {
    if (Mcontinuation_immutablep(k)) {
        // immutable => need to make a copy
        obj k2;

        switch (Mcontinuation_type(k)) {
        // bottom of continuation chain (immutable, by default)
        case NULL_CONT_TYPE:
            k2 = k;
            break;

        // application
        case APP_CONT_TYPE:
            k2 = Mapp_continuation(Mcontinuation_prev(k), Mcontinuation_env(k), NULL);
            Mcontinuation_app_hd(k2) = Mcontinuation_app_hd(k);
            Mcontinuation_app_tl(k2) = Mcontinuation_app_tl(k);
            break;

        // if expressions
        case COND_CONT_TYPE:
            k2 = Mcond_continuation(
                Mcontinuation_prev(k),
                Mcontinuation_env(k),
                Mcontinuation_cond_ift(k),
                Mcontinuation_cond_iff(k)
            );
            break;

        // begin expressions
        case SEQ_CONT_TYPE:
            k2 = Mseq_continuation(
                Mcontinuation_prev(k),
                Mcontinuation_env(k),
                Mcontinuation_seq_value(k)
            );
            break;

        // let expressions
        case LET_CONT_TYPE:
            k2 = Mlet_continuation(
                Mcontinuation_prev(k),
                Mcontinuation_env(k),
                Mcontinuation_let_bindings(k),
                Mcontinuation_let_body(k)
            );
            Mcontinuation_let_env(k2) = Mcontinuation_let_env(k);
            break;

        // set! expressions
        case SETB_CONT_TYPE:
            k2 = Msetb_continuation(
                Mcontinuation_prev(k),
                Mcontinuation_env(k),
                Mcontinuation_setb_name(k)
            );
            break;

        // call/cc expressions
        case CALLCC_CONT_TYPE:
            k2 = Mcallcc_continuation(
                Mcontinuation_prev(k),
                Mcontinuation_env(k),
                Mcontinuation_callcc_winders(k)
            );
            break;

        // dynamic-wind expressions
        case DYNWIND_CONT_TYPE:
            k2 = Mdynwind_continuation(
                Mcontinuation_prev(k),
                Mcontinuation_env(k),
                Mcontinuation_dynwind_pre(k),
                Mcontinuation_dynwind_val(k),
                Mcontinuation_dynwind_post(k)    
            );
            Mcontinuation_dynwind_pre(k2) = Mcontinuation_dynwind_pre(k);
            Mcontinuation_dynwind_state(k2) = Mcontinuation_dynwind_state(k);
            break;

        default:
            minim_error1("continuation_pop", "unimplemented", Mfixnum(Mcontinuation_type(k)));
        }

        return k2;
    } else {
        // mutable => return it
        return k;
    }
}

// Length of a continuation chain.
// static uptr continuation_length(obj k) {
//     uptr l = 0;
//     for (; Mcontinuationp(k); k = Mcontinuation_prev(k), ++l);
//     return l;
// }

// // Extracts the tail of a continuation chain.
// static obj continuation_tail(obj k, iptr l) {
//     for (uptr i = 0; i < l; ++i, k = Mcontinuation_prev(k));
//     return k;
// }

// Extracts the common tail of two winder lists
static obj common_tail(obj xs, obj ys) {
    iptr l1, l2;

    // eliminate excess winders
    l1 = list_length(xs);
    l2 = list_length(ys);
    if (l1 > l2) {
        xs = list_tail(xs, l1 - l2);
    } else if (l2 > l1) {
        ys = list_tail(ys, l2 - l1);
    }

    // walk back along tails until a common ancestor
    while (!Mnullp(xs)) {
        if (xs == ys)
            return xs;

        xs = Mcdr(xs);
        ys = Mcdr(ys);
    }

    return xs;
}

// Restores a continuation. The continuation must have been captured
// by `call/cc`. May add a continuation frame to handle any winders
// installed by `dynamic-wind`.
obj continuation_restore(obj tc, obj k) {
    obj cc_winders, k_winders, tl, it, unwind, wind, winders;
    
    // check that continuation was captured by call/cc
    if (!Mcontinuation_capturedp(k)) {
        minim_error1("continuation_restore()", "can only restored captured continuations", k);
    }

    // compute common tail of winders
    cc_winders = Mtc_wnd(tc);
    k_winders = Mcontinuation_callcc_winders(k);
    tl = common_tail(cc_winders, k_winders);

    // winders are the same, just restore the continuation
    if (tl == k_winders) {
        return k;
    }

    // find all winders in `cc` that need to be unwound
    unwind = Mnull;
    for (it = cc_winders; it != tl; it = Mcdr(it)) {
        // unwinding => need to execute post thunk
        unwind = Mcons(Mcdar(it), unwind);
    }

    wind = Mnull;
    for (it = k_winders; it != tl; it = Mcdr(it)) {
        // winding => need to execute pre thunk
        wind = Mcons(Mcaar(it), wind);
    }

    // unwind, in reverse order, than wind
    winders = Mappend(Mreverse(unwind), wind);
    if (!Mnullp(winders)) {
        // any winders => need to execute them before anything else
        k = Mwinders_continuation(k, Mcontinuation_env(k), winders);
    }

    return k;
}

// For debugging
void print_continuation(obj cc) {
    fprintf(stderr, "continuation (type=%d, ptr=%p):\n", Mcontinuation_type(cc), cc);
    fprintf(stderr, " immutable?: %d\n", Mcontinuation_immutablep(cc));
    fprintf(stderr, " prev: %p\n", Mcontinuation_prev(cc));
    fprintf(stderr, " env: ");
    writeln_object(stderr, Mcontinuation_env(cc));

    switch (Mcontinuation_type(cc)) {
    // bottom of continuation chain
    case NULL_CONT_TYPE:
        break;

    // application
    case APP_CONT_TYPE:
        fprintf(stderr, " hd: ");
        writeln_object(stderr, Mcontinuation_app_hd(cc));
        fprintf(stderr, " tl: ");
        writeln_object(stderr, Mcontinuation_app_tl(cc));
        break;

    // if expressions
    case COND_CONT_TYPE:
        fprintf(stderr, " ift: ");
        writeln_object(stderr, Mcontinuation_cond_ift(cc));
        fprintf(stderr, " iff: ");
        writeln_object(stderr, Mcontinuation_cond_iff(cc));
        break;

    // begin expressions
    case SEQ_CONT_TYPE:
        fprintf(stderr, " seq: ");
        writeln_object(stderr, Mcontinuation_seq_value(cc));
        break;

    // let expressions
    case LET_CONT_TYPE:
        fprintf(stderr, " env*: ");
        writeln_object(stderr, Mcontinuation_let_env(cc));
        fprintf(stderr, " bindings: ");
        writeln_object(stderr, Mcontinuation_let_bindings(cc));
        fprintf(stderr, " body: ");
        writeln_object(stderr, Mcontinuation_let_body(cc));
        break;

    // set! expressions
    case SETB_CONT_TYPE:
        fprintf(stderr, " name: ");
        writeln_object(stderr, Mcontinuation_setb_name(cc));
        break;

    // call/cc expressions
    case CALLCC_CONT_TYPE:
        break;

    default:
        minim_error1("print_continuation", "unimplemented", Mfixnum(Mcontinuation_type(cc)));
    }

    if (Mcontinuationp(Mcontinuation_prev(cc)))
        print_continuation(Mcontinuation_prev(cc));
}
