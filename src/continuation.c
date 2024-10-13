// continuation.c: support for continuations

#include "minim.h"

obj continuation_restore(obj cc) {
    obj k;

    switch (Mcontinuation_type(cc)) {
    // bottom of continuation chain
    // (immutable, by default)
    case NULL_CONT_TYPE:
        return cc;

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
        k = Mcallcc_continuation(Mcontinuation_prev(cc), Mcontinuation_env(cc));
        Mcontinuation_callcc_frozenp(k) = Mcontinuation_callcc_frozenp(cc);
        if (Mcontinuation_callcc_frozenp(k)) {
            // frozen => stop copying
            Mcontinuation_callcc_frozenp(k) = 1;
            return k;
        }
        break;

    default:
        minim_error1("restore_cc", "unimplemented", Mfixnum(Mcontinuation_type(cc)));
    }

    Mcontinuation_prev(k) = continuation_restore(Mcontinuation_prev(cc));
    return k;
}
