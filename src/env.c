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

    return Mnull;
}

void env_insert(obj env, obj k, obj v) {
    Mcar(env) = Mcons(Mcons(k, v), Mcar(env));
}
