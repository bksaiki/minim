// prim.c: primitives

#include "minim.h"

obj nullp_prim;
obj symbolp_prim;
obj fixnump_prim;
obj charp_prim;
obj stringp_prim;
obj consp_prim;
obj procp_prim;

obj cons_prim;
obj car_prim;
obj cdr_prim;
obj list_prim;
obj length_prim;
obj reverse_prim;
obj append_prim;

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

obj error_prim;

obj apply_prim;
obj callcc_prim;
obj callwv_prim;
obj exit_prim;
obj dynwind_prim;
obj values_prim;
obj void_prim;

// Wrapped procedures

#define proc0(name, e) \
    static obj name(void) { return (e); }
#define proc1(name, x, e) \
    static obj name(obj x) { return (e); }
#define uncallable_proc(name) \
    static obj name(obj x) { minim_error("" #name "()", "should never call"); }

proc1(nullp_proc, x, Mbool(Mnullp(x)))
proc1(symbolp_proc, x, Mbool(Msymbolp(x)))
proc1(fixnump_proc, x, Mbool(Mfixnump(x)))
proc1(charp_proc, x, Mbool(Mcharp(x)))
proc1(stringp_proc, x, Mbool(Mstringp(x)))
proc1(consp_proc, x, Mbool(Mconsp(x)))
proc1(procp_proc, x, Mbool(Mprocp(x)))
proc1(car_proc, x, Mcar(x))
proc1(cdr_proc, x, Mcdr(x))

uncallable_proc(apply_proc);
uncallable_proc(callcc_proc);
uncallable_proc(callwv_proc);
uncallable_proc(dynwind_proc);
uncallable_proc(exit_proc);
uncallable_proc(list_proc);
uncallable_proc(values_proc);
uncallable_proc(void_proc);

// Public API

void init_prims(void) {
    nullp_prim = Mprim(nullp_proc, 1, "null?");
    symbolp_prim = Mprim(symbolp_proc, 1, "symbol?");
    fixnump_prim = Mprim(fixnump_proc, 1, "fixnum?");
    charp_prim = Mprim(charp_proc, 1, "char?");
    stringp_prim = Mprim(stringp_proc, 1, "string?");
    consp_prim = Mprim(consp_proc, 1, "pair?");
    procp_prim = Mprim(procp_proc, 1, "procedure?");

    cons_prim = Mprim(Mcons, 2, "cons");
    car_prim = Mprim(car_proc, 1, "car");
    cdr_prim = Mprim(cdr_proc, 1, "cdr");
    length_prim = Mprim(Mlength, 1, "length");
    reverse_prim = Mprim(Mreverse, 1, "reverse");
    append_prim = Mprim(Mappend, 2, "append");

    fx_neg_prim = Mprim(Mfx_neg, 1, "fxneg");
    fx_inc_prim = Mprim(Mfx_inc, 1, "fx1+");
    fx_dec_prim = Mprim(Mfx_dec, 1, "fx1-");
    fx_add_prim = Mprim(Mfx_add, 2, "fx+");
    fx_sub_prim = Mprim(Mfx_sub, 2, "fx-");
    fx_mul_prim = Mprim(Mfx_mul, 2, "fx*");
    fx_div_prim = Mprim(Mfx_div, 2, "fx/");
    fx_eq_prim = Mprim(Mfx_eq, 2, "fx=");
    fx_ge_prim = Mprim(Mfx_ge, 2, "fx>=");
    fx_le_prim = Mprim(Mfx_le, 2, "fx<=");
    fx_gt_prim = Mprim(Mfx_gt, 2, "fx>");
    fx_lt_prim = Mprim(Mfx_lt, 2, "fx<");

    error_prim = Mprim(error_prim, 3, "error");

    apply_prim = Mprim(apply_proc, -3, "apply");
    Mprim_specialp(apply_prim) = 1;
    callcc_prim = Mprim(callcc_proc, 1, "call-with-current-continuation");
    Mprim_specialp(callcc_prim) = 1;
    callwv_prim = Mprim(callwv_proc, 2, "call-with-values");
    Mprim_specialp(callwv_prim) = 1;
    dynwind_prim = Mprim(dynwind_proc, 3, "dynamic-wind");
    Mprim_specialp(dynwind_prim) = 1;
    list_prim = Mprim(list_proc, -1, "list");
    Mprim_specialp(list_prim) = 1;
    values_prim = Mprim(values_proc, -1, "values");
    Mprim_specialp(values_prim) = 1;
    void_prim = Mprim(void_proc, -1, "void");
    Mprim_specialp(void_prim) = 1;
    exit_prim = Mprim(exit_proc, 1, "exit");
    Mprim_specialp(exit_prim) = 1;
}

#define env_add_prim(e, p)  \
    env_insert(e, Mprim_name(p), p)

obj prim_env(obj env) {
    env = env_extend(env);

    env_add_prim(env, nullp_prim);
    env_add_prim(env, symbolp_prim);
    env_add_prim(env, fixnump_prim);
    env_add_prim(env, charp_prim);
    env_add_prim(env, stringp_prim);
    env_add_prim(env, consp_prim);
    env_add_prim(env, procp_prim);

    env_add_prim(env, car_prim);
    env_add_prim(env, cdr_prim);
    env_add_prim(env, cons_prim);
    env_add_prim(env, length_prim);
    env_add_prim(env, reverse_prim);
    env_add_prim(env, append_prim);

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

    env_add_prim(env, error_prim);

    env_add_prim(env, apply_prim);
    env_insert(env, Mintern("call/cc"), callcc_prim);
    env_add_prim(env, callcc_prim);
    env_add_prim(env, callwv_prim);
    env_add_prim(env, exit_prim);
    env_add_prim(env, dynwind_prim);
    env_add_prim(env, list_prim);
    env_add_prim(env, values_prim);
    env_add_prim(env, void_prim);

    return env;
}
