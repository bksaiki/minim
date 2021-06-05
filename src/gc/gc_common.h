#ifndef _MINIM_GC_H_
#define _MINIM_GC_H_

#include <limits.h>
#include <stddef.h>
#include <stdint.h>

/* Debug */
#define GC_DEBUG        1

/* Block flag */
#define GC_BLOCK_MARK       0x1
#define GC_BLOCK_ROOT       0x2

/* GC Parameters */
#define GC_MIN_AUTO_COLLECT_SIZE     (4 * 1024 * 1024)
#define GC_TABLE_LOAD_FACTOR         0.5

/* Array type forces value on stack */
typedef void *gc_addr_t[1];

/* Destructor signature */
typedef void (*gc_dtor_t)(void*);

/* Marking signature */
typedef void (*gc_mark_t)(void*,void*,void*);

/* GC block type */
typedef struct gc_block_t {
    void *ptr;
    gc_dtor_t dtor;
    gc_mark_t mrk;
    size_t size, hash;
    uint8_t flags;
} gc_block_t;

/* Main GC type */
typedef struct gc_t {
    gc_block_t *items;
    void *stack_bottom;
    double load_factor;
    size_t itemc, nitems;
    size_t allocs, dirty;
} gc_t;

/************ gc_t interface ************/

gc_t *gc_create(void *stack);
void gc_destroy(gc_t* gc);

void gc_add(gc_t *gc, void *ptr, size_t size, gc_dtor_t dtor, gc_mark_t mrk);
void gc_remove(gc_t *gc, void *ptr, int destroy);

gc_block_t *gc_get_block(gc_t *gc, void *ptr);
gc_block_t *gc_remove_block(gc_t *gc, gc_block_t *block);
void gc_update_block(gc_t *gc, gc_block_t *block, size_t size, gc_dtor_t dtor, gc_mark_t mrk);

void gc_collect(gc_t *gc);

void gc_register_dtor(gc_t *gc, void *ptr, gc_dtor_t dtor);
void gc_register_mrk(gc_t *gc, void *ptr, gc_mark_t mrk);
void gc_register_root(gc_t *gc, void *ptr);

size_t gc_get_allocated(gc_t *gc);
size_t gc_get_reachable(gc_t *gc);
size_t gc_get_collectable(gc_t *gc);

#endif
