#ifndef _MINIM_H_
#define _MINIM_H_

#include <setjmp.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "gc.h"
#include "../build/config.h"

// Config

#define MINIM_VERSION      "0.3.5"

#if defined (__GNUC__)
#define MINIM_GCC     1
#define NORETURN    __attribute__ ((noreturn))
#else
#error "compiler not supported"
#endif

// Type aliases

typedef uint8_t         byte;
typedef uintptr_t       uptr;
typedef intptr_t        iptr;
typedef void            *obj;

// system constants

#define SYMBOL_MAX_LENGTH       4096
#define INIT_VALUES_BUFFER_LEN  10

// macros

#define ptr_size        sizeof(void*)
#define ptr_add(p, d)   ((void *) ((uptr) (p)) + (d))

#define NUL_CHAR        ((int) 0x00)        // null
#define BEL_CHAR        ((int) 0x07)        // alarm / bell
#define BS_CHAR         ((int) 0x08)        // backspace
#define HT_CHAR         ((int) 0x09)        // horizontal tab
#define LF_CHAR         ((int) 0x0A)        // line feed
#define VT_CHAR         ((int) 0x0B)        // vertical tab
#define FF_CHAR         ((int) 0x0C)        // form feed (page break)
#define CR_CHAR         ((int) 0x0D)        // carriage return
#define ESC_CHAR        ((int) 0x1B)        // escape
#define SP_CHAR         ((int) 0x20)        // space
#define DEL_CHAR        ((int) 0x7F)        // delete

// Syntax

extern obj Mbegin_symbol;
extern obj Mcallcc_symbol;
extern obj Mcallwv_symbol;
extern obj Mdynwind_symbol;
extern obj Mif_symbol;
extern obj Mlambda_symbol;
extern obj Mlet_symbol;
extern obj Mlet_values_symbol;
extern obj Mletrec_symbol;
extern obj Mletrec_values_symbol;
extern obj Mquote_symbol;
extern obj Msetb_symbol;

// Object types

typedef enum {
    SPECIAL_OBJ_TYPE,
    SYMBOL_OBJ_TYPE,
    FIXNUM_OBJ_TYPE,
    CHAR_OBJ_TYPE,
    STRING_OBJ_TYPE,
    CONS_OBJ_TYPE,
    PRIM_OBJ_TYPE,
    CLOSURE_OBJ_TYPE,
    CONTINUATON_OBJ_TYPE,
    PORT_OBJ_TYPE,
    THREAD_OBJ_TYPE
} obj_type_t;

// Objects

#define obj_type(o)       (*((byte*) o))

// Special objects

extern obj Mnull;
extern obj Mtrue;
extern obj Mfalse;
extern obj Mvoid;
extern obj Meof;
extern obj Mvalues;
extern obj Munbound;

#define Mnullp(x)       ((x) == Mnull)
#define Mtruep(x)       ((x) == Mtrue)
#define Mfalsep(x)      ((x) == Mfalse)
#define Mvoidp(x)       ((x) == Mvoid)
#define Meofp(x)        ((x) == Meof)
#define Mvaluesp(x)     ((x) == Mvalues)
#define Munboundp(x)    ((x) == Munbound)

#define Mnot(x)         (Mfalsep(x) ? Mtrue : Mfalse)
#define Mbool(x)        (((x) == 0) ? Mfalse : Mtrue)

// Symbol
// +------------+
// |    type    | [0, 1)
// |    str     | [8, 16)
// +------------+
#define Msymbol_size        (2 * ptr_size)
#define Msymbolp(o)         (obj_type(o) == SYMBOL_OBJ_TYPE)
#define Msymbol_value(o)    (*((char **) ptr_add(o, ptr_size)))

obj Msymbol(const char *s);

// Fixnum
// +------------+
// |    type    | [0, 1)
// |     v      | [8, 16)
// +------------+ 
#define Mfixnum_size        (2 * ptr_size)
#define Mfixnump(o)         (obj_type(o) == FIXNUM_OBJ_TYPE)
#define Mfixnum_value(o)    (*((iptr*) ptr_add(o, ptr_size)))

obj Mfixnum(iptr v);

// Char
// +------------+
// |    type    | [0, 1)
// |     v      | [4, 8)
// +------------+ 
#define Mchar_size          (1 * ptr_size)
#define Mcharp(o)           (obj_type(o) == CHAR_OBJ_TYPE)
#define Mchar_value(o)      (*((int*) ptr_add(o, sizeof(int))))

obj Mchar(int c);

// String
// +------------+
// |    type    | [0, 1)
// |    str     | [8, 16)
// |    len     | [16, 24)
// +------------+ 
#define Mstring_size            (3 * ptr_size)
#define Mstringp(o)             (obj_type(o) == STRING_OBJ_TYPE)
#define Mstring_value(o)        (*((char**) ptr_add(o, ptr_size)))
#define Mstring_length(o)       (*((uptr*) ptr_add(o, 2 * ptr_size)))
#define Mstring_ref(o, i)       (Mstring_value(o)[i])

obj Mstring(const char* s);

// Pair
// +------------+
// |    type    | [0, 1)
// |    car     | [8, 16)
// |    cdr     | [16, 24)
// +------------+ 
#define Mcons_size          (3 * ptr_size)
#define Mconsp(o)           (obj_type(o) == CONS_OBJ_TYPE)
#define Mcar(o)             (*((obj*) ptr_add(o, ptr_size)))
#define Mcdr(o)             (*((obj*) ptr_add(o, 2 * ptr_size)))

#define Mcaar(o)            (Mcar(Mcar(o)))
#define Mcadr(o)            (Mcar(Mcdr(o)))
#define Mcdar(o)            (Mcdr(Mcar(o)))
#define Mcddr(o)            (Mcdr(Mcdr(o)))

#define Mcaaar(o)           (Mcar(Mcaar(o)))
#define Mcdaar(o)           (Mcdr(Mcaar(o)))
#define Mcaadr(o)           (Mcar(Mcadr(o)))
#define Mcdadr(o)           (Mcdr(Mcadr(o)))
#define Mcadar(o)           (Mcar(Mcdar(o)))
#define Mcddar(o)           (Mcdr(Mcdar(o)))
#define Mcaddr(o)           (Mcar(Mcddr(o)))
#define Mcdddr(o)           (Mcdr(Mcddr(o)))

obj Mcons(obj car, obj cdr);

#define Mlist1(x)               Mcons(x, Mnull)
#define Mlist2(x, y)            Mcons(x, Mlist1(y))
#define Mlist3(x, y, z)         Mcons(x, Mlist2(y, z))
#define Mlist4(w, x, y, z)      Mcons(w, Mlist3(x, y, z))

// Primitive
// +------------+
// |    type    | [0, 1)
// |    fn      | [8, 16)
// |    arity   | [16, 24)
// |    name    | [24, 32)
// +------------+
#define Mprim_size          (4 * ptr_size)
#define Mprimp(o)           (obj_type(o) == PRIM_OBJ_TYPE)
#define Mprim_value(o)      (*((void **) ptr_add(o, ptr_size)))
#define Mprim_arity(o)      (*((iptr*) ptr_add(o, 2 * ptr_size)))
#define Mprim_name(o)       (*((obj *) ptr_add(o, 3 * ptr_size)))

obj Mprim(void *fn, iptr arity, const char *name);

// Closure
// +------------+
// |    type    | [0, 1)
// |    env     | [8, 16)
// |    code    | [16, 24)
// |   formals  | [24, 32)
// |    arity   | [32, 40)
// |    name    | [40, 48)
// +------------+

#define Mclosure_size           (6 * ptr_size)
#define Mclosurep(o)            (obj_type(o) == CLOSURE_OBJ_TYPE)
#define Mclosure_env(o)         (*((obj *) ptr_add(o, ptr_size)))
#define Mclosure_body(o)        (*((obj *) ptr_add(o, 2 * ptr_size)))
#define Mclosure_formals(o)     (*((obj *) ptr_add(o, 3 * ptr_size)))
#define Mclosure_arity(o)       (*((iptr*) ptr_add(o, 4 * ptr_size)))
#define Mclosure_name(o)        (*((obj*) ptr_add(o, 5 * ptr_size)))

obj Mclosure(obj env, obj formals, obj body);

// Continuation
// +------------+
// |    type    | [0, 1)
// | cont_type  | [1, 2)
// |  immutable | [2, 3)
// |  captured  | [3, 4)
// |    prev    | [8, 16)
// |    env     | [16, 24)
// |    ...     |
// +------------+
#define Mcontinuation_size(n)           ((n + 3) * ptr_size)
#define Mcontinuationp(o)               (obj_type(o) == CONTINUATON_OBJ_TYPE)
#define Mcontinuation_type(o)           (*((byte*) ptr_add(o, 1)))
#define Mcontinuation_immutablep(o)     (*((byte*) ptr_add(o, 2)))
#define Mcontinuation_capturedp(o)      (*((byte*) ptr_add(o, 3)))
#define Mcontinuation_prev(o)           (*((obj*) ptr_add(o, ptr_size)))
#define Mcontinuation_env(o)            (*((obj*) ptr_add(o, 2 * ptr_size)))

typedef enum {
    NULL_CONT_TYPE,
    APP_CONT_TYPE,
    COND_CONT_TYPE,
    SEQ_CONT_TYPE,
    LET_CONT_TYPE,
    SETB_CONT_TYPE,
    CALLCC_CONT_TYPE,
    CALLWV_CONT_TYPE,
    DYNWIND_CONT_TYPE,
    WINDERS_CONT_TYPE
} cont_type_t;

#define Mcontinuation_null_size     Mcontinuation_size(0)
#define Mcontinuation_nullp(o)      (Mcontinuation_type(o) == NULL_CONT_TYPE)

#define Mcontinuation_app_size      Mcontinuation_size(2)
#define Mcontinuation_appp(o)       (Mcontinuation_type(o) == APP_CONT_TYPE)
#define Mcontinuation_app_hd(o)     (*((obj*) ptr_add(o, 3 * ptr_size)))
#define Mcontinuation_app_tl(o)     (*((obj*) ptr_add(o, 4 * ptr_size)))

#define Mcontinuation_cond_size         Mcontinuation_size(2)
#define Mcontinuation_condp(o)          (Mcontinuation_type(o) == COND_CONT_TYPE)
#define Mcontinuation_cond_ift(o)       (*((obj*) ptr_add(o, 3 * ptr_size)))
#define Mcontinuation_cond_iff(o)       (*((obj*) ptr_add(o, 4 * ptr_size)))

#define Mcontinuation_seq_size          Mcontinuation_size(1)
#define Mcontinuation_seqp(o)           (Mcontinuation_type(o) == SEQ_CONT_TYPE)
#define Mcontinuation_seq_value(o)      (*((obj*) ptr_add(o, 3 * ptr_size)))

#define Mcontinuation_let_size          Mcontinuation_size(3)
#define Mcontinuation_letp(o)           (Mcontinuation_type(o) == LET_CONT_TYPE)
#define Mcontinuation_let_env(o)        (*((obj*) ptr_add(o, 3 * ptr_size)))
#define Mcontinuation_let_bindings(o)   (*((obj*) ptr_add(o, 4 * ptr_size)))
#define Mcontinuation_let_body(o)       (*((obj*) ptr_add(o, 5 * ptr_size)))

#define Mcontinuation_setb_size         Mcontinuation_size(1)
#define Mcontinuation_setbp(o)          (Mcontinuation_type(o) == SETB_CONT_TYPE)
#define Mcontinuation_setb_name(o)      (*((obj*) ptr_add(o, 3 * ptr_size)))

#define Mcontinuation_callcc_size           Mcontinuation_size(1)
#define Mcontinuation_callccp(o)            (Mcontinuation_type(o) == CALLCC_CONT_TYPE)
#define Mcontinuation_callcc_winders(o)     (*((obj*) ptr_add(o, 3 * ptr_size)))

#define Mcontinuation_callwv_size               Mcontinuation_size(2)
#define Mcontinuation_callwvp(o)                (Mcontinuation_type(o) == CALLWV_CONT_TYPE)
#define Mcontinuation_callwv_producer(o)        (*((obj*) ptr_add(o, 3 * ptr_size)))
#define Mcontinuation_callwv_consumer(o)        (*((obj*) ptr_add(o, 4 * ptr_size)))

#define Mcontinuation_dynwind_size          Mcontinuation_size(4)
#define Mcontinuation_dynwindp(o)           (Mcontinuation_type(o) == DYNWIND_CONT_TYPE)
#define Mcontinuation_dynwind_pre(o)        (*((obj*) ptr_add(o, 3 * ptr_size)))
#define Mcontinuation_dynwind_val(o)        (*((obj*) ptr_add(o, 4 * ptr_size)))
#define Mcontinuation_dynwind_post(o)       (*((obj*) ptr_add(o, 5 * ptr_size)))
#define DYNWIND_NEW     0x0
#define DYNWIND_PRE     0x1
#define DYNWIND_VAL     0x2
#define DYNWIND_POST    0x3
#define Mcontinuation_dynwind_state(o)          (*((byte*) ptr_add(o, 6 * ptr_size)))

#define Mcontinuation_winders_size          Mcontinuation_size(1)
#define Mcontinuation_windersp(o)           (Mcontinuation_type(o) == WINDERS_CONT_TYPE)
#define Mcontinuation_winders_value(o)      (*((obj*) ptr_add(o, 3 * ptr_size)))

obj Mnull_continuation(obj env);
obj Mapp_continuation(obj prev, obj env, obj args);
obj Mcond_continuation(obj prev, obj env, obj ift, obj iff);
obj Mseq_continuation(obj prev, obj env, obj seq);
obj Mlet_continuation(obj prev, obj env, obj bindings, obj body);
obj Msetb_continuation(obj prev, obj env, obj name);
obj Mcallcc_continuation(obj prev, obj env, obj winders);
obj Mcallwv_continuation(obj prev, obj env, obj producer);
obj Mdynwind_continuation(obj prev, obj env, obj val, obj post);
obj Mwinders_continuation(obj prev, obj env, obj winders);

// Port 
// +------------+
// |    type    | [0, 1)
// |    flags   | [1, 2)
// |    buffer  | [8, 16)
// |    count   | [16, 24)
// +------------+
#define Mport_size          (3 * ptr_size)
#define Mportp(o)           (obj_type(o) == PORT_OBJ_TYPE)
#define Mport_flags(o)      (*((byte*) ptr_add(o, 1)))
#define Mport_file(o)       (*((FILE**) ptr_add(o, ptr_size)))
#define Mport_buffer(o)     (*((char**) ptr_add(o, ptr_size)))
#define Mport_count(o)      (*((uptr*) ptr_add(o, 2 * ptr_size)))

#define PORT_FLAG_OPEN      0x1
#define PORT_FLAG_READ      0x2
#define PORT_FLAG_STR       0x4

obj Minput_file_port(FILE *f);
obj Moutput_file_port(FILE *f);
obj Minput_string_port(obj s);

int port_readyp(obj p);
int port_peek(obj p);
int port_read(obj p);
void port_write(int c, obj p);

// Thread context
// +------------+
// |    type    | [0, 1)
// |    cc      | [8, 16)       // current continuation
// |   winders  | [16, 24)      // current winders
// |    env     | [24, 32)      // current environment
// |    vb      | [32, 40)      // values buffer
// |    va      | [40, 48)      // values buffer allocation size
// |    vc      | [48, 56)      // values buffer count
// +------------+
#define Mtc_size            (7 * ptr_size)
#define Mtcp(o)             (obj_type(o) == THREAD_OBJ_TYPE)
#define Mtc_cc(o)           (*((obj*) ptr_add(o, ptr_size)))
#define Mtc_wnd(o)          (*((obj*) ptr_add(o, 2 * ptr_size)))
#define Mtc_env(o)          (*((obj*) ptr_add(o, 3 * ptr_size)))
#define Mtc_vb(o)           (*((obj**) ptr_add(o, 4 * ptr_size)))
#define Mtc_va(o)           (*((uptr*) ptr_add(o, 5 * ptr_size)))
#define Mtc_vc(o)           (*((uptr*) ptr_add(o, 6 * ptr_size)))

obj Mthread_context(void);

// Composite predicates

#define Mboolp(x)       (Mtruep(x) || Mfalsep(x))
#define Mprocp(x)       (Mprimp(x) || Mclosurep(x) || Mcontinuationp(x))

#define Minput_portp(o)     (Mportp(o) && (Mport_flags(o) & PORT_FLAG_READ))
#define Moutput_portp(o)    (Mportp(o) && !(Mport_flags(o) & PORT_FLAG_READ))
#define Mfile_portp(o)      (Mportp(o) && !(Mport_flags(o) & PORT_FLAG_STR))
#define Mstring_portp(o)    (Mportp(o) && (Mport_flags(o) & PORT_FLAG_STR))

// Environments

obj empty_env(void);
obj env_extend(obj env);
obj env_find(obj env, obj k);
void env_insert(obj env, obj k, obj v);

// Fixnums

obj Mfx_neg(obj x);
obj Mfx_inc(obj x);
obj Mfx_dec(obj x);

obj Mfx_add(obj x, obj y);
obj Mfx_sub(obj x, obj y);
obj Mfx_mul(obj x, obj y);
obj Mfx_div(obj x, obj y);

obj Mfx_eq(obj x, obj y);
obj Mfx_ge(obj x, obj y);
obj Mfx_le(obj x, obj y);
obj Mfx_gt(obj x, obj y);
obj Mfx_lt(obj x, obj y);

// List

int Mlistp(obj x);
iptr list_length(obj x);
obj Mlength(obj x);
obj Mreverse(obj x);
obj Mappend(obj x, obj y);
obj list_tail(obj x, iptr i);

// Continuations

// Marks a continuation chain as immutable.
// Unwinding through an immutable continuation chain requires
// copying each continuation as needed.
void continuation_set_immutable(obj k);

// Safely returns a mutable version of the current continuation.
// If the continuation chain is immutable, a copy is made.
// Otherwise, the argument is returned.
obj continuation_mutable(obj k);

// Restores a continuation. The continuation must have been captured
// by `call/cc`. May add a continuation frame to handle any winders
// installed by `dynamic-wind`.
obj continuation_restore(obj tc, obj k);

// For debugging
void print_continuation(obj cc);

// Hashing

size_t hash_bytes(const void *data, size_t len);

// Primitives

void init_prims(void);
obj prim_env(obj env);

// Evaluation

int Mimmediatep(obj x);

void check_expr(obj e);
obj expand_expr(obj e);
obj eval_expr(obj e);

// Reading

obj read_object(obj ip);

// Printing

void write_obj(FILE *out, obj o);

#define writeln_object(out, o) { \
    write_obj(out, o); \
    fputc('\n', out); \
}

// Prims

extern obj nullp_prim;
extern obj cons_prim;
extern obj car_prim;
extern obj cdr_prim;

extern obj fx_neg_prim;
extern obj fx_inc_prim;
extern obj fx_dec_prim;
extern obj fx_add_prim;
extern obj fx_sub_prim;
extern obj fx_mul_prim;
extern obj fx_div_prim;
extern obj fx_eq_prim;
extern obj fx_ge_prim;
extern obj fx_le_prim;
extern obj fx_gt_prim;
extern obj fx_lt_prim;

extern obj values_prim;

// Errors

NORETURN void minim_error(const char *name, const char *msg);
NORETURN void minim_error1(const char *name, const char *msg, obj x);
NORETURN void minim_error2(const char *name, const char *msg, obj x, obj y);
NORETURN void minim_error3(const char *name, const char *msg, obj x, obj y, obj z);

// Intern table

typedef struct intern_table {
    obj *buckets;
    size_t *alloc_ptr;
    size_t alloc, size;
} intern_table;

extern intern_table *itab;

intern_table *make_intern_table(void);
obj intern(intern_table *tab, const char *s);

#define Mintern(x)      intern(itab, x)

obj Mgensym(const char *s);

// System

extern obj *Mcurr_tc_box;

#define Mcurr_tc()    (*((obj*) Mcurr_tc_box))

void minim_init(void);
NORETURN void minim_shutdown(int);

#endif  // _MINIM_H_
