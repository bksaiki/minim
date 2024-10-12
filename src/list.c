// list.c: list and pair primitives 

#include "minim.h"

obj car_proc(obj x) {
    return Mcar(x);
}

obj cdr_proc(obj x) {
    return Mcdr(x);
}

iptr list_length(obj x) {
    iptr l;
    for (l = 0; !Mnullp(x); x = Mcdr(x), ++l);
    return l;
}

obj Mlength(obj x) {
    return Mfixnum(list_length(x));
}

