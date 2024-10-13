// expand.c: minimal expander

#include "minim.h"

static obj condense_body(obj es) {
    if (Mnullp(Mcdr(es))) {
        // single expression => just use expression
        return Mcar(es);
    } else {
        // multiple expressions => wrap in begin
        return Mcons(Mbegin_symbol, es);
    }
}

static obj expand_let_expr(obj e) {
    obj it, hd, tl;

    it = Mcadr(e);
    hd = tl = Mcons(Mlist2(Mcaar(it), expand_expr(Mcadar(it))), Mnull);
    for (it = Mcdr(it); !Mnullp(it); it = Mcdr(it)) {
        Mcdr(tl) = Mcons(Mlist2(Mcaar(it), expand_expr(Mcadar(it))), Mnull);
        tl = Mcdr(tl);
    }

    return Mlist3(Mlet_symbol, hd, condense_body(Mcddr(e)));
}

obj expand_expr(obj e) {
    obj hd, tl, it;

    if (Mconsp(e)) {
        hd = Mcar(e);
        if (hd == Mlet_symbol) {
            if (Mnullp(Mcadr(e))) {
                // empty bindings => just use body
                return condense_body(Mcddr(e));
            } else {
                // at least one binding
                return expand_let_expr(e);
            }
        } else if (hd == Mbegin_symbol) {
            if (Mnullp(Mcdr(e))) {
                // no body expression
                return Mlist2(Mquote_symbol, Mvoid);
            } else if (Mnullp(Mcddr(e))) {
                // one body expression
                return expand_expr(Mcadr(e));
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
        } else if (hd == Mquote_symbol) {
            return e;
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
        minim_error1(NULL, "unreachable", e);
    }
}
