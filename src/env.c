// env.c: environments

#include "minim.h"

obj empty_env(void) {
    return Mcons(Mnull, Mnull); 
}

obj env_extend(obj env) {
    return Mcons(Mnull, env);
}

obj env_find(obj env, obj k) {
    for (; !Mnullp(env); env = Mcdr(env)) {
        for (obj r = Mcar(env); !Mnullp(r); r = Mcdr(r)) {
            if (Mcaar(r) == k)
                return Mcar(r);
        }
    }

    return Mfalse;
}

void env_insert(obj env, obj k, obj v) {
    Mcar(env) = Mcons(Mcons(k, v), Mcar(env));
}

void import_env(obj dst, obj src) {
    obj r, cell;

    for (; !Mnullp(src); src = Mcdr(src)) {
        for (r = Mcar(src); !Mnullp(r); r = Mcdr(r)) {
            cell = env_find(dst, Mcaar(r));
            if (Mfalsep(cell)) {
                // no current binding => insert new one
                env_insert(dst, Mcaar(r), Mcdar(r));
            } else {
                // current binding => overwrite it
                Mcdr(cell) = Mcdar(r);
            }
        }
    }
}
