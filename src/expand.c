// expand.c: minimal expander

#include "minim.h"

static obj condense_body(obj es) {
    if (Mnullp(Mcdr(es))) {
        // single expression => just use expression
        return expand_expr(Mcar(es));
    } else {
        // multiple expressions => wrap in begin
        return expand_expr(Mcons(Mbegin_symbol, es));
    }
}

// (let ([<id> <expr>]) ...) <body>)
// => (let-values ([(<id>) <expr>]) ...) <body>)
//
// (let ([<id> <expr>]) ...) <body> ...)
// => (let-values ([(<id>) <expr>]) ...) (begin <body> ...)
static obj expand_let_expr(obj e) {
    obj it, hd, tl, bind;

    it = Mcadr(e);
    hd = tl = Mlist1(Mlist2(Mlist1(Mcaar(it)), expand_expr(Mcadar(it))));
    for (it = Mcdr(it); !Mnullp(it); it = Mcdr(it)) {
        bind = Mlist2(Mlist1(Mcaar(it)), expand_expr(Mcadar(it)));
        Mcdr(tl) = Mlist1(bind);
        tl = Mcdr(tl);
    }

    return Mlist3(Mlet_values_symbol, hd, condense_body(Mcddr(e)));
}

// (let <name> ([<id> <expr>]) ...)
//   <body>
//   ...)
// =>
// (letrec ([<name> (lambda (<id> ...) <body ...)])
//   (<name> <expr> ...))
static obj expand_let_loop(obj e) {
    obj it, formals, body, tl;

    it = Mcaddr(e);
    if (Mnullp(it)) {
        // special case: no arguments
        formals = Mnull;
        body = Mlist1(Mcadr(e));
    } else {
        // at least one argument =>
        // construct the lambda formals
        formals = tl = Mcons(Mcaar(it), Mnull);
        for (it = Mcdr(it); !Mnullp(it); it = Mcdr(it)) {
            Mcdr(tl) = Mcons(Mcaar(it), Mnull);
            tl = Mcdr(tl);
        }

        // construct the body
        body = tl = Mcons(Mcadr(e), Mnull);
        for (it = Mcaddr(e); !Mnullp(it); it = Mcdr(it)) {
            Mcdr(tl) = Mcons(Mcadar(it), Mnull);
            tl = Mcdr(tl);
        }
    }

    return Mlist3(
        Mletrec_symbol,
        Mlist1(Mlist2(Mcadr(e), Mcons(Mlambda_symbol, Mcons(formals, Mcdddr(e))))),
        body
    );
}

// (letrec ([<id> <expr>] ...)
//   <body>
//   ...)
// =>
// (let ([<id> #<unbound>] ...)
//   (set! <id> <expr>)
//   ...
//   <body>
//   ...)
static obj expand_letrec_expr(obj e) {
    obj it, binds, body, tl;

    // create the initial bindings
    it = Mcadr(e);
    binds = tl = Mcons(Mlist2(Mcaar(it), Mlist2(Mquote_symbol, Munbound)), Mnull);
    for (it = Mcdr(it); !Mnullp(it); it = Mcdr(it)) {
        Mcdr(tl) = Mcons(Mlist2(Mcaar(it), Mlist2(Mquote_symbol, Munbound)), Mnull);
        tl = Mcdr(tl);
    }

    // create the body with set! expressions
    it = Mcadr(e);
    body = tl = Mcons(Mlist3(Msetb_symbol, Mcaar(it), Mcadar(it)), Mnull);
    for (it = Mcdr(it); !Mnullp(it); it = Mcdr(it)) {
        Mcdr(tl) = Mcons(Mlist3(Msetb_symbol, Mcaar(it), Mcadar(it)), Mnull);
        tl = Mcdr(tl);
    }

    Mcdr(tl) = Mcddr(e);
    return Mcons(Mlet_symbol, Mcons(binds, body));
}


obj expand_expr(obj e) {
    obj hd, tl, it;

loop:

    if (Mconsp(e)) {
        hd = Mcar(e);
        if (hd == Mlet_symbol) {
            if (Msymbolp(Mcadr(e))) {
                // named let => convert to letrec
                e = expand_let_loop(e);
                goto loop;
            } else if (Mnullp(Mcadr(e))) {
                // empty bindings => just use body
                return condense_body(Mcddr(e));
            } else {
                // at least one binding
                return expand_let_expr(e);
            }
        } else if (hd == Mletrec_symbol) {
            // letrec => transforms to let
            if (Mnullp(Mcadr(e))) {
                // empty bindings => just use body
                return condense_body(Mcddr(e));
            } else {
                // at least one binding
                e = expand_letrec_expr(e);
                goto loop;
            }
        } else if (hd == Mbegin_symbol) {
            if (Mnullp(Mcdr(e))) {
                // no body expression
                return Mlist2(Mquote_symbol, Mvoid);
            } else if (Mnullp(Mcddr(e))) {
                // one body expression
                e = Mcadr(e);
                goto loop;
            } else {
                // multiple body expressions
                hd = tl = Mcons(Mbegin_symbol, Mnull);
                for (it = Mcdr(e); !Mnullp(it); it = Mcdr(it)) {
                    Mcdr(tl) = Mcons(expand_expr(Mcar(it)), Mnull);
                    tl = Mcdr(tl);
                }
        
                return hd;
            }
        } else if (hd == Mif_symbol) {
            return Mlist4(
                Mif_symbol,
                expand_expr(Mcadr(e)),
                expand_expr(Mcaddr(e)),
                expand_expr(Mcar(Mcdddr(e)))
            );
        } else if (hd == Mlambda_symbol) {
            return Mlist3(Mlambda_symbol, Mcadr(e), condense_body(Mcddr(e)));
        } else if (hd == Msetb_symbol) {
            return Mlist3(Msetb_symbol, Mcadr(e), expand_expr(Mcaddr(e)));
        } else if (hd == Mquote_symbol) {
            return e;
        } else if (hd == Mcallcc_symbol) {
            return Mlist2(Mcallcc_symbol, expand_expr(Mcadr(e)));
        } else if (hd == Mcallwv_symbol) {
            return Mlist3(Mcallwv_symbol, expand_expr(Mcadr(e)), expand_expr(Mcaddr(e)));
        } else {
            hd = tl = Mcons(expand_expr(Mcar(e)), Mnull);
            for (it = Mcdr(e); !Mnullp(it); it = Mcdr(it)) {
                Mcdr(tl) = Mcons(expand_expr(Mcar(it)), Mnull);
                tl = Mcdr(tl);
            }
        
            return hd;
        }
    } else if (Msymbolp(e) || Mimmediatep(e)) {
        // immediate or variable
        return e;
    } else {
        minim_error1("expand_expr", "unreachable", e);
    }
}
