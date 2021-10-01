#ifndef _MINIM_GC_H_
#define _MINIM_GC_H_

#include <limits.h>
#include <stddef.h>
#include <stdint.h>

#include "gc-types.h"

/* Debug */
#define GC_DEBUG        0

/* GC Parameters */
#define GC_MIN_AUTO_COLLECT_SIZE     (8 * 1024 * 1024)
#define GC_TABLE_LOAD_FACTOR         0.5
#define GC_MINOR_PER_MAJOR           15
#define GC_CUSTOM_MARKER_MODE        2

/* Heap heuristic */
#if defined (_WIN32) || defined (_WIN64)
#define GC_HEAP_HEURISTIC   0
#else
#define GC_HEAP_HEURISTIC   1
#endif

/************ gc_t interface ************/

gc_t *gc_create(void *stack);
void gc_destroy(gc_t* gc);

void gc_pause(gc_t *gc);
void gc_resume(gc_t *gc);

void gc_add(gc_t *gc, void *ptr, size_t size, gc_dtor_t dtor, gc_mark_t mrk);
void gc_remove(gc_t *gc, void *ptr, int destroy);

gc_block_t *gc_get_block(gc_t *gc, void *ptr);
void gc_update_block(gc_t *gc, gc_block_t *block, size_t size, gc_dtor_t dtor, gc_mark_t mrk);

void gc_collect(gc_t *gc);
void gc_collect_young(gc_t *gc);

void gc_register_dtor(gc_t *gc, void *ptr, gc_dtor_t dtor);
void gc_register_mrk(gc_t *gc, void *ptr, gc_mark_t mrk);
void gc_register_root(gc_t *gc, void *ptr);

size_t gc_get_allocated(gc_t *gc);
size_t gc_get_reachable(gc_t *gc);
size_t gc_get_collectable(gc_t *gc);

#endif