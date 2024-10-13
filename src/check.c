// check.c: syntax checker

#include "minim.h"

static void assert_identifier(obj form, obj id) {
    if (!Msymbolp(id)) {
        minim_error2(Msymbol_value(Mcar(form)), "expected identifier", form, id);
    }
}

NORETURN static void bad_syntax_exn(obj form) {
    minim_error1(Msymbol_value(Mcar(form)), "bad syntax", form);
}

// Already assumes `expr` is `(<name> . <???>)`
// Check: `expr` must be `(<name> <datum>)
static void check_1ary_syntax(obj e) {
    obj rib = Mcdr(e);
    if (!Mconsp(rib) || !Mnullp(Mcdr(rib)))
        bad_syntax_exn(e);
}

// Already assumes `expr` is `(<name> . <???>)`
// Check: `expr` must be `(<name> <datum> <datum> <datum>)
static void check_3ary_syntax(obj e) {
    obj rib;
    
    rib = Mcdr(e);
    if (!Mconsp(rib))
        bad_syntax_exn(e);

    rib = Mcdr(rib);
    if (!Mconsp(rib))
        bad_syntax_exn(e);

    rib = Mcdr(rib);
    if (!Mconsp(rib) || !Mnullp(Mcdr(rib)))
        bad_syntax_exn(e);
}

// Already assumes `expr` is `(let . <???>)`
// Check: `expr` must be `(let-values ([(<var> ...) <val>] ...) <body> ...)`
// Does not check if each `<body>` is an expression.
// Does not check if `<body> ...` forms a list.
static void check_let(obj e) {
    obj bindings, bind, id;
    
    bindings = Mcdr(e);
    if (!Mconsp(bindings) || !Mconsp(Mcdr(bindings)))
        bad_syntax_exn(e);
    
    bindings = Mcar(bindings);
    while (Mconsp(bindings)) {
        bind = Mcar(bindings);
        if (!Mconsp(bind))
            bad_syntax_exn(e);

        id = Mcar(bind);
        assert_identifier(e, id);

        bind = Mcdr(bind);
        if (!Mconsp(bind) || !Mnullp(Mcdr(bind)))
            bad_syntax_exn(e);

        bindings = Mcdr(bindings);
    }

    if (!Mnullp(bindings))
        bad_syntax_exn(e);
}

// Already assumes `expr` is `(<name> . <???>)`
// Check: `expr` must be `(<name> <datum> ...)`
static void check_begin(obj e) {
    obj rib;
    
    rib = Mcdr(e);
    while (Mconsp(rib))
        rib = Mcdr(rib);

    if (!Mnullp(rib))
        bad_syntax_exn(e);
}

// Already assumes `expr` is `(<name> . <???>)`
// Check: `expr` must be `(<name> <datum> ...)`
static void check_lambda(obj e) {
    obj args = Mcadr(e);
    for (; Mconsp(args); args = Mcdr(args)) {
        assert_identifier(e, Mcar(args));
    }

    if (!Mnullp(args)) {
        assert_identifier(e, args);
    }
}

void check_expr(obj e) {
    obj hd, it;

    if (Mconsp(e)) {
        hd = Mcar(e);
        if (hd == Mlet_symbol) {
            check_let(e);
            for (it = Mcadr(e); !Mnullp(it); it = Mcdr(it))
                check_expr(Mcadar(it));
            for (it = Mcddr(e); !Mnullp(it); it = Mcdr(it))
                check_expr(Mcar(it));
        } else if (hd == Mbegin_symbol) {
            check_begin(e);
            for (it = Mcdr(e); !Mnullp(it); it = Mcdr(it))
                check_expr(Mcar(it));
        } else if (hd == Mif_symbol) {
            check_3ary_syntax(e);
            check_expr(Mcadr(e));
            check_expr(Mcaddr(e));
            check_expr(Mcar(Mcdddr(e)));
        } else if (hd == Mlambda_symbol) {
            check_lambda(e);
            for (it = Mcddr(e); !Mnullp(it); it = Mcdr(it))
                check_expr(Mcar(it));
        } else if (hd == Mquote_symbol) {
            check_1ary_syntax(e);
        } else if (Mlistp(e)) {
            for (it = e; !Mnullp(it); it = Mcdr(it))
                check_expr(Mcar(it));
        } else {
            minim_error1(NULL, "malformed application", e);
        }
    } else if (Msymbolp(e) || Mimmediatep(e)) {
        // immediate or variable
        return;
    } else if (Mnullp(e)) {
        // empty application
        minim_error1(NULL, "empty application", e);
    } else {
        minim_error1(NULL, "bad syntax", e);
    }
}
