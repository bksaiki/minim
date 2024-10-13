// prim.c: primitives

#include "minim.h"

obj cons_prim;
obj car_prim;
obj cdr_prim;

void init_prims(void) {
    cons_prim = Mprim(Mcons, 2, "cons");
    car_prim = Mprim(car_proc, 1, "car");
    cdr_prim = Mprim(cdr_proc, 1, "cdr");
}

#define env_add_prim(e, p)  \
    env_insert(e, Mprim_name(p), p)

obj prim_env(obj env) {
    env = env_extend(env);
    env_add_prim(env, car_prim);
    env_add_prim(env, cdr_prim);
    env_add_prim(env, cons_prim);
    return env;
}

// Wrapped primitives

obj car_proc(obj x) {
    return Mcar(x);
}

obj cdr_proc(obj x) {
    return Mcdr(x);
}

