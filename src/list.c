// list.c: list and pair primitives 

#include "minim.h"

int Mlistp(obj x) {
    while (Mconsp(x)) x = Mcdr(x);
    return Mnullp(x);
}

iptr list_length(obj x) {
    iptr l;
    for (l = 0; !Mnullp(x); x = Mcdr(x), ++l);
    return l;
}

obj Mlength(obj x) {
    return Mfixnum(list_length(x));
}

