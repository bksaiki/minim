// list.c: list and pair primitives 

#include "minim.h"

int Mlistp(obj x) {
    for (; Mconsp(x); x = Mcdr(x));
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

obj Mreverse(obj x) {
    obj hd, tl;

    if (Mnullp(x)) {
        return Mnull;
    } else {
        hd = tl = Mlist1(Mcar(x));
        for (x = Mcdr(x); !Mnullp(x); x = Mcdr(x)) {
            Mcdr(tl) = Mlist1(Mcar(x));
            tl = Mcdr(tl);
        }

        return hd;
    }
}

obj Mappend(obj x, obj y) {
    obj hd, tl;

    if (Mnullp(x)) {
        return y;
    } else if (Mnullp(y)) {
        return x;
    } else {
        hd = tl = Mlist1(Mcar(x));
        for (x = Mcdr(x); !Mnullp(x); x = Mcdr(x)) {
            Mcdr(tl) = Mlist1(Mcar(x));
            tl = Mcdr(tl);
        }

        Mcdr(tl) = y;
        return hd;
    }
}
