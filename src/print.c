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
    } else if (Mstringp(o)) {
        fprintf(out, "\"%s\"", Mstring_value(o));
    } else if (Mconsp(o)) {
        write_pair(out, o);
    } else if (Mprimp(o)) {
        fputs("#<procedure:", out);
        write_obj(out, Mprim_name(o));
        fputc('>', out);
    } else if (Minput_portp(o)) {
        fputs("#<input-port>", out);
    } else if (Moutput_portp(o)) {
        fputs("#<output-port>", out);
    } else if (Mcontinuationp(o)) {
        fputs("#<procedure>", out);
    } else {
        fputs("#<garbage>", out);
    }
}
