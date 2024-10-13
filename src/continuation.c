// continuation.c: support for continuations

#include "minim.h"

// Marks a continuation chain as immutable.
// Unwinding through an immutable continuation chain requires
// copying each continuation as needed.
void continuation_set_immutable(obj cc) {
    while (Mcontinuationp(cc)) {
        Mcontinuation_immutablep(cc) = 1;
        cc = Mcontinuation_prev(cc);
    }
}

// Safely returns a mutable version of the current continuation.
// If the continuation chain is immutable, a copy is made.
// Otherwise, the argument is returned.
obj continuation_mutable(obj cc) {
    if (Mcontinuation_immutablep(cc)) {
        // immutable => need to make a copy
        obj k;

        switch (Mcontinuation_type(cc)) {
        // bottom of continuation chain (immutable, by default)
        case NULL_CONT_TYPE:
            k = cc;
            break;

        // application
        case APP_CONT_TYPE:
            k = Mapp_continuation(Mcontinuation_prev(cc), Mcontinuation_env(cc), NULL);
            Mcontinuation_app_hd(k) = Mcontinuation_app_hd(cc);
            Mcontinuation_app_tl(k) = Mcontinuation_app_tl(cc);
            break;

        // if expressions
        case COND_CONT_TYPE:
            k = Mcond_continuation(
                Mcontinuation_prev(cc),
                Mcontinuation_env(cc),
                Mcontinuation_cond_ift(cc),
                Mcontinuation_cond_iff(cc)
            );
            break;

        // begin expressions
        case SEQ_CONT_TYPE:
            k = Mseq_continuation(
                Mcontinuation_prev(cc),
                Mcontinuation_env(cc),
                Mcontinuation_seq_value(cc)
            );
            break;

        // let expressions
        case LET_CONT_TYPE:
            k = Mlet_continuation(
                Mcontinuation_prev(cc),
                Mcontinuation_env(cc),
                Mcontinuation_let_bindings(cc),
                Mcontinuation_let_body(cc)
            );
            Mcontinuation_let_env(k) = Mcontinuation_let_env(cc);
            break;

        // set! expressions
        case SETB_CONT_TYPE:
            k = Msetb_continuation(
                Mcontinuation_prev(cc),
                Mcontinuation_env(cc),
                Mcontinuation_setb_name(cc)
            );
            break;

        // call/cc expressions
        case CALLCC_CONT_TYPE:
            return Mcallcc_continuation(Mcontinuation_prev(cc), Mcontinuation_env(cc));

        default:
            minim_error1("continuation_pop", "unimplemented", Mfixnum(Mcontinuation_type(cc)));
        }

        return k;
    } else {
        // mutable => return it
        return cc;
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
