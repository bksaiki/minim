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
    obj x = GC_malloc(Mcontinuation_size(0));
    obj_type(x) = CONTINUATON_OBJ_TYPE;
    Mcontinuation_type(x) = NULL_CONT_TYPE;
    Mcontinuation_prev(x) = Mnull;
    Mcontinuation_env(x) = env;
    return x;
}

obj Mapp_continuation(obj prev, obj env, obj args) {
    obj x = GC_malloc(Mcontinuation_size(2));
    obj_type(x) = CONTINUATON_OBJ_TYPE;
    Mcontinuation_type(x) = APP_CONT_TYPE;
    Mcontinuation_prev(x) = prev;
    Mcontinuation_env(x) = env;
    Mcontinuation_app_hd(x) = args;
    Mcontinuation_app_tl(x) = NULL;
    return x;
}

obj Mcond_continuation(obj prev, obj env, obj ift, obj iff) {
    obj x = GC_malloc(Mcontinuation_size(2));
    obj_type(x) = CONTINUATON_OBJ_TYPE;
    Mcontinuation_type(x) = COND_CONT_TYPE;
    Mcontinuation_prev(x) = prev;
    Mcontinuation_env(x) = env;
    Mcontinuation_cond_ift(x) = ift;
    Mcontinuation_cond_iff(x) = iff;
    return x;
}

obj Mseq_continuation(obj prev, obj env, obj seq) {
    obj x = GC_malloc(Mcontinuation_size(1));
    obj_type(x) = CONTINUATON_OBJ_TYPE;
    Mcontinuation_type(x) = SEQ_CONT_TYPE;
    Mcontinuation_prev(x) = prev;
    Mcontinuation_env(x) = env;
    Mcontinuation_seq_value(x) = seq;
    return x;
}
