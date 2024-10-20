// prim.c: primitives

#include "minim.h"

obj nullp_prim;
obj cons_prim;
obj car_prim;
obj cdr_prim;

obj fx_neg_prim;
obj fx_inc_prim;
obj fx_dec_prim;
obj fx_add_prim;
obj fx_sub_prim;
obj fx_mul_prim;
obj fx_div_prim;
obj fx_eq_prim;
obj fx_ge_prim;
obj fx_le_prim;
obj fx_gt_prim;
obj fx_lt_prim;

obj values_prim;

// Wrapped procedures

#define proc0(name, e) static obj name(void) { return (e); }
#define proc1(name, x, e) static obj name(obj x) { return (e); }


proc1(nullp_proc, x, Mbool(Mnullp(x)))
proc1(car_proc, x, Mcar(x))
proc1(cdr_proc, x, Mcdr(x))

static obj values_proc() {
    minim_error("values_proc()", "should never call");
}

// Public API

void init_prims(void) {
    nullp_prim = Mprim(nullp_proc, 1, "null?");
    cons_prim = Mprim(Mcons, 2, "cons");
    car_prim = Mprim(car_proc, 1, "car");
    cdr_prim = Mprim(cdr_proc, 1, "cdr");

    fx_neg_prim = Mprim(Mfx_neg, 1, "fxneg");
    fx_inc_prim = Mprim(Mfx_inc, 1, "fx1+");
    fx_dec_prim = Mprim(Mfx_dec, 1, "fx1-");
    fx_add_prim = Mprim(Mfx_add, 2, "fx2+");
    fx_sub_prim = Mprim(Mfx_sub, 2, "fx2-");
    fx_mul_prim = Mprim(Mfx_mul, 2, "fx2*");
    fx_div_prim = Mprim(Mfx_div, 2, "fx2/");
    fx_eq_prim = Mprim(Mfx_eq, 2, "fx2=");
    fx_ge_prim = Mprim(Mfx_ge, 2, "fx2>=");
    fx_le_prim = Mprim(Mfx_le, 2, "fx2<=");
    fx_gt_prim = Mprim(Mfx_gt, 2, "fx2>");
    fx_lt_prim = Mprim(Mfx_lt, 2, "fx2<");

    values_prim = Mprim(values_proc, -1, "values");
}

#define env_add_prim(e, p)  \
    env_insert(e, Mprim_name(p), p)

obj prim_env(obj env) {
    env = env_extend(env);

    env_add_prim(env, nullp_prim);
    env_add_prim(env, car_prim);
    env_add_prim(env, cdr_prim);
    env_add_prim(env, cons_prim);

    env_add_prim(env, fx_neg_prim);
    env_add_prim(env, fx_inc_prim);
    env_add_prim(env, fx_dec_prim);
    env_add_prim(env, fx_add_prim);
    env_add_prim(env, fx_sub_prim);
    env_add_prim(env, fx_mul_prim);
    env_add_prim(env, fx_div_prim);
    env_add_prim(env, fx_eq_prim);
    env_add_prim(env, fx_ge_prim);
    env_add_prim(env, fx_le_prim);
    env_add_prim(env, fx_gt_prim);
    env_add_prim(env, fx_lt_prim);

    env_add_prim(env, values_prim);

    return env;
}
