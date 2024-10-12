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

// macros

#define ptr_size        sizeof(void*)
#define ptr_add(p, d)   ((void *) ((uptr) (p)) + (d))

// Object types

typedef enum {
    SPECIAL_OBJ_TYPE,
    SYMBOL_OBJ_TYPE,
    FIXNUM_OBJ_TYPE,
    CONS_OBJ_TYPE,
    PRIM_OBJ_TYPE,
    CONTINUATON_OBJ_TYPE,
} obj_type_t;

// Objects

#define obj_type(o)       (*((byte*) o))

// Special objects

extern obj Mnull;
extern obj Mtrue;
extern obj Mfalse;
extern obj Mvoid;

#define Mnullp(x)   ((x) == Mnull)
#define Mtruep(x)   ((x) == Mtrue)
#define Mfalsep(x)  ((x) == Mfalse)
#define Mvoidp(x)   ((x) == Mvoid)

#define Mboolp(x)   (Mtruep(x) || Mfalsep(x))
#define Mnot(x)     (Mfalsep(x) ? Mtrue : Mfalse)

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

#define Mlist1(x)           Mcons(x, Mnull)
#define Mlist2(x, y)        Mcons(x, Mlist1(y))
#define Mlist3(x, y, z)     Mcons(x, Mlist2(y, z))

// Primitive
// +------------+
// |    type    | [0, 1)
// |     fn     | [8, 16)
// |    arity   | [16, 24)
// |    name    | [24, 32)
// +------------+
#define Mprim_size          (4 * ptr_size)
#define Mprimp(o)           (obj_type(o) == PRIM_OBJ_TYPE)
#define Mprim_value(o)      (*((void **) ptr_add(o, ptr_size)))
#define Mprim_arity(o)      (*((iptr*) ptr_add(o, 2 * ptr_size)))
#define Mprim_name(o)       (*((obj *) ptr_add(o, 3 * ptr_size)))

obj Mprim(void *fn, iptr arity, const char *name);

// Continuation
// +------------+
// |    type    | [0, 1)
// | cont_type  | [0, 2)
// |    prev    | [8, 16)
// |    ...     |
// +------------+
#define Mcontinuation_size(n)       ((n + 2) * ptr_size)
#define Mcontinuationp(o)           (obj_type(o) == CONTINUATON_OBJ_TYPE)
#define Mcontinuation_type(o)       (*((byte*) ptr_add(o, 1)))
#define Mcontinuation_prev(o)       (*((obj*) ptr_add(o, ptr_size)))

typedef enum {
    NULL_CONT_TYPE,
    APP_CONT_TYPE
} cont_type_t;

#define Mcontinuation_nullp(o)      (Mcontinuation_type(o) == NULL_CONT_TYPE)

#define Mcontinuation_appp(o)       (Mcontinuation_type(o) == APP_CONT_TYPE)
#define Mcontinuation_app_hd(o)  (*((obj*) ptr_add(o, 2 * ptr_size)))
#define Mcontinuation_app_tl(o)   (*((obj*) ptr_add(o, 3 * ptr_size)))

obj Mnull_continuation(void);
obj Mapp_continuation(obj prev, obj args);

// Environments

obj empty_env(void);
obj env_extend(obj env);
obj env_find(obj env, obj k);
void env_insert(obj env, obj k, obj v);

// List

iptr list_length(obj x);

obj car_proc(obj x);
obj cdr_proc(obj x);
obj Mlength(obj x);

// Hashing

size_t hash_bytes(const void *data, size_t len);

// Primitives

void init_prims(void);
void prim_env(obj env);

// Evaluation

obj eval_expr(obj e, obj env);

// Printing

void write_obj(FILE *out, obj o);

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

// System

void minim_init(void);
NORETURN void minim_shutdown(int);

#endif  // _MINIM_H_
