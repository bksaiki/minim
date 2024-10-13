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
            k2 = Mcallcc_continuation(Mcontinuation_prev(k), Mcontinuation_env(k));
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
static uptr continuation_length(obj k) {
    uptr l = 0;
    for (; Mcontinuationp(k); k = Mcontinuation_prev(k), ++l);
    return l;
}

// Extracts the tail of a continuation chain.
static obj continuation_tail(obj k, iptr l) {
    for (uptr i = 0; i < l; ++i, k = Mcontinuation_prev(k));
    return k;
}

// Extracts the common tail of two continuation chains.
static obj common_tail(obj k1, obj k2) {
    uptr l1, l2;

    // eliminate excess frames
    l1 = continuation_length(k1);
    l2 = continuation_length(k2);
    if (l1 > l2) {
        k1 = continuation_tail(k1, l1 - l2);
    } else  if (l2 > l1) {
        k2 = continuation_tail(k2, l2 - l1);
    }

    // unwind both until a common ancestor is found
    while (Mcontinuationp(k1)) {
        if (k1 == k2)
            return k1;

        k1 = Mcontinuation_prev(k1);
        k2 = Mcontinuation_prev(k2);
    }

    return k1;
}

// Restores a continuation.
// The result is a new continuation chain formed by merging
// the common ancestors of the continuation and current continuation.
obj continuation_restore(obj cc, obj k) {
    obj tl, re;
    
    tl = common_tail(cc, k);
    if (!Mcontinuationp(tl)) {
        // fully unwound, just reinstate `k` fully
        return k;
    } else if (tl == k) {
        // `tl` is `k` so just return it
        return tl;
    } else {
        // found a common tail, reinstate `k` up to that point
        // we can assume that `tl` is immutable since `k` must be
        // captured by `call/cc` (and variants)
        if (!Mcontinuation_immutablep(tl))
            minim_error1("continuation_restore", "must be immutable", tl);

        // walk back to the common tail and track the frames (must be >=1)
        re = Mcons(continuation_mutable(k), Mnull);
        for (k = Mcontinuation_prev(k); k != tl; k = Mcontinuation_prev(k)) {
            re = Mcons(continuation_mutable(k), re);
        }

        // fprintf(stderr, "reinstating %lu frames (%lu saved)\n", list_length(re), continuation_length(tl));

        // reinstate the frames and link them
        k = tl;
        for (; !Mnullp(re); re = Mcdr(re)) {
            Mcontinuation_prev(Mcar(re)) = k;
            k = Mcar(re);
        }

        return k;
    }
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
