// vector.c: primitive for vectors

#include "minim.h"

obj list_to_vector(obj x) {
    obj v;
    uptr len, i;

    if (Mnullp(x)) {
        return Memptyvec;
    } else {
        len = list_length(x);
        v = Mvector(len, NULL);

        i = 0;
        for (; Mconsp(x); x = Mcdr(x)) {
            Mvector_ref(v, i) = Mcar(x);
            i++;
        }

        return v;
    }
}
