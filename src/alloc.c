// alloc.c: object allocation

#include "minim.h"

obj Msymbol(const char *s) {
    obj x;
    size_t l;

    x = GC_malloc(Msymbol_size);
    obj_type(x) = SYMBOL_OBJ_TYPE;

    l = strlen(s);
    Msymbol_value(x) = GC_malloc_atomic(l + 1);
    strncpy(Msymbol_value(x), s, l + 1);

    return x;
}

obj Mfixnum(iptr v) {
    obj x = GC_malloc(Mfixnum_size);
    obj_type(x) = FIXNUM_OBJ_TYPE;
    Mfixnum_value(x) = v;
    return x;
}

obj Mchar(int c) {
    obj x = GC_malloc(Mchar_size);
    obj_type(x) = CHAR_OBJ_TYPE;
    Mchar_value(x) = c;
    return x;
}

obj Mstring(const char* s) {
    obj x = GC_malloc(Mstring_size);
    obj_type(x) = STRING_OBJ_TYPE;
    Mstring_length(x) = strlen(s);
    Mstring_value(x) = GC_malloc_atomic(Mstring_length(x) + 1);
    strncpy(Mstring_value(x), s, Mstring_length(x) + 1);
    return x;
}

obj Mcons(obj car, obj cdr) {
    obj x = GC_malloc(Mcons_size);
    obj_type(x) = CONS_OBJ_TYPE;
    Mcar(x) = car;
    Mcdr(x) = cdr;
    return x;
}

obj Mprim(void *fn, iptr arity, const char *name) {
    obj x = GC_malloc(Mprim_size);
    obj_type(x) = PRIM_OBJ_TYPE;
    Mprim_value(x) = fn;
    Mprim_arity(x) = arity;
    Mprim_name(x) = Mintern(name);
    return x;
}

obj Mclosure(obj env, obj formals, obj body) {
    obj x;
    iptr arity;
    
    x = GC_malloc(Mclosure_size);
    obj_type(x) = CLOSURE_OBJ_TYPE;
    Mclosure_env(x) = env;
    Mclosure_body(x) = body;
    Mclosure_formals(x) = formals;
    Mclosure_name(x) = Mfalse;

    for (arity = 0; Mconsp(formals); formals = Mcdr(formals), ++arity);
    Mclosure_arity(x) = Mnullp(formals) ? arity : (-arity - 1);

    return x;
}

obj Minput_file_port(FILE *f) {
    obj x = GC_malloc(Mport_size);
    obj_type(x) = PORT_OBJ_TYPE;
    Mport_flags(x) = PORT_FLAG_READ;
    Mport_file(x) = f;
    Mport_count(x) = fseek(f, 0, SEEK_CUR);
    return x;
}

obj Moutput_file_port(FILE *f) {
    obj x = GC_malloc(Mport_size);
    obj_type(x) = PORT_OBJ_TYPE;
    Mport_flags(x) = 0x0;
    Mport_file(x) = f;
    Mport_count(x) = fseek(f, 0, SEEK_CUR);
    return x;
}

obj Minput_string_port(obj s) {
    obj x = GC_malloc(Mport_size);
    obj_type(x) = PORT_OBJ_TYPE;
    Mport_flags(x) = PORT_FLAG_READ | PORT_FLAG_STR;
    Mport_buffer(x) = s;
    Mport_count(x) = 0;
    return x;
}

obj Mnull_continuation(obj env) {
    obj x = GC_malloc(Mcontinuation_null_size);
    obj_type(x) = CONTINUATON_OBJ_TYPE;
    Mcontinuation_type(x) = NULL_CONT_TYPE;
    Mcontinuation_immutablep(x) = 0;
    Mcontinuation_prev(x) = Mnull;
    Mcontinuation_env(x) = env;
    return x;
}

obj Mapp_continuation(obj prev, obj env, obj args) {
    obj x = GC_malloc(Mcontinuation_app_size);
    obj_type(x) = CONTINUATON_OBJ_TYPE;
    Mcontinuation_type(x) = APP_CONT_TYPE;
    Mcontinuation_immutablep(x) = 0;
    Mcontinuation_prev(x) = prev;
    Mcontinuation_env(x) = env;
    Mcontinuation_app_hd(x) = args;
    Mcontinuation_app_tl(x) = NULL;
    return x;
}

obj Mcond_continuation(obj prev, obj env, obj ift, obj iff) {
    obj x = GC_malloc(Mcontinuation_cond_size);
    obj_type(x) = CONTINUATON_OBJ_TYPE;
    Mcontinuation_type(x) = COND_CONT_TYPE;
    Mcontinuation_immutablep(x) = 0;
    Mcontinuation_prev(x) = prev;
    Mcontinuation_env(x) = env;
    Mcontinuation_cond_ift(x) = ift;
    Mcontinuation_cond_iff(x) = iff;
    return x;
}

obj Mseq_continuation(obj prev, obj env, obj seq) {
    obj x = GC_malloc(Mcontinuation_seq_size);
    obj_type(x) = CONTINUATON_OBJ_TYPE;
    Mcontinuation_type(x) = SEQ_CONT_TYPE;
    Mcontinuation_immutablep(x) = 0;
    Mcontinuation_prev(x) = prev;
    Mcontinuation_env(x) = env;
    Mcontinuation_seq_value(x) = seq;
    return x;
}

obj Mlet_continuation(obj prev, obj env, obj bindings, obj body) {
    obj x = GC_malloc(Mcontinuation_let_size);
    obj_type(x) = CONTINUATON_OBJ_TYPE;
    Mcontinuation_type(x) = LET_CONT_TYPE;
    Mcontinuation_immutablep(x) = 0;
    Mcontinuation_prev(x) = prev;
    Mcontinuation_env(x) = env;
    Mcontinuation_let_env(x) = env_extend(env);
    Mcontinuation_let_bindings(x) = bindings;
    Mcontinuation_let_body(x) = body;
    return x;   
}

obj Msetb_continuation(obj prev, obj env, obj name) {
    obj x = GC_malloc(Mcontinuation_seq_size);
    obj_type(x) = CONTINUATON_OBJ_TYPE;
    Mcontinuation_type(x) = SETB_CONT_TYPE;
    Mcontinuation_immutablep(x) = 0;
    Mcontinuation_prev(x) = prev;
    Mcontinuation_env(x) = env;
    Mcontinuation_setb_name(x) = name;
    return x;
}

obj Mcallcc_continuation(obj prev, obj env) {
    obj x = GC_malloc(Mcontinuation_seq_size);
    obj_type(x) = CONTINUATON_OBJ_TYPE;
    Mcontinuation_type(x) = CALLCC_CONT_TYPE;
    Mcontinuation_immutablep(x) = 0;
    Mcontinuation_prev(x) = prev;
    Mcontinuation_env(x) = env;
    return x;
}

obj Mthread_context(void) {
    obj x = GC_malloc(Mtc_size);
    obj_type(x) = THREAD_OBJ_TYPE;
    Mtc_cc(x) = Mnull;
    Mtc_env(x) = Mnull;
    Mtc_vb(x) = GC_malloc(INIT_VALUES_BUFFER_LEN * sizeof(obj));
    Mtc_vc(x) = INIT_VALUES_BUFFER_LEN;
    return x;
}
