// print.c: basic printing

#include "minim.h"

static void write_pair(FILE *out, obj o) {
    fputc('(', out);
loop:
    write_obj(out, Mcar(o));
    if (Mconsp(Mcdr(o))) {
        // middle of a (proper/improper) list
        fputc(' ', out);
        o = Mcdr(o);
        goto loop;
    } else if (!Mnullp(Mcdr(o))) {
        // end of an improper list
        fputs(" . ", out);
        write_obj(out, Mcdr(o));
    }

    fputc(')', out);
}

void write_obj(FILE *out, obj o) {
    if (Mnullp(o)) {
        fputs("()", out);
    } else if (Mtruep(o)) {
        fputs("#t", out);
    } else if (Mfalsep(o)) {
        fputs("#f", out);
    } else if (Mvoidp(o)) {
        fputs("#<void>", out);
    } else if (Msymbolp(o)) {
        fputs(Msymbol_value(o), out);
    } else if (Mfixnump(o)) {
        fprintf(out, "%ld", Mfixnum_value(o));
    } else if (Mconsp(o)) {
        write_pair(out, o);
    } else {
        fputs("#<garbage", out);
    }
}
