/*
    Vectors
*/

#include "../minim.h"

mobj *make_vector(long len, mobj *init) {
    minim_vector_object *o;
    long i;
    
    o = GC_alloc(sizeof(minim_vector_object));
    o->type = MINIM_VECTOR_TYPE;
    o->len = len;
    o->arr = GC_alloc(len * sizeof(mobj *));

    if (init != NULL) {
        for (i = 0; i < len; ++i)
            o->arr[i] = init;
    }

    return ((mobj *) o);
}

mobj list_to_vector(mobj lst) {
    mobj v, it;
    msize i;
    
    it = v = make_vector(list_length(lst), NULL);
    for (i = 0; i < minim_vector_len(v); ++i) {
        minim_vector_ref(v, i) = minim_car(it);
        it = minim_cdr(it);
    }

    return v;
}

static void vector_out_of_bounds_exn(const char *name, mobj *v, long idx) {
    fprintf(stderr, "%s, index out of bounds\n", name);
    fprintf(stderr, " length: %ld\n", minim_vector_len(v));
    fprintf(stderr, " index:  %ld\n", idx);
    minim_shutdown(1);
}

//
//  Primitives
//

mobj *is_vector_proc(int argc, mobj **args) {
    // (-> any boolean)
    return (minim_is_vector(args[0]) ? minim_true : minim_false);
}

mobj *make_vector_proc(int argc, mobj **args) {
    // (-> non-negative-integer vector)
    // (-> non-negative-integer any vector)
    mobj *init;
    long len;

    if (!minim_is_fixnum(args[0]) || minim_fixnum(args[0]) < 0)
        bad_type_exn("make-vector", "non-negative-integer?", args[0]);
    len = minim_fixnum(args[0]);

    if (len == 0) {
        // special case:
        return minim_empty_vec;
    }

    if (argc == 1) {
        // 1st case
        init = Mfixnum(0);
    } else {
        // 2nd case
        init = args[1];
    }

    return make_vector(len, init);
}

mobj *vector_proc(int argc, mobj **args) {
    // (-> any ... vector)
    mobj *v;
    
    v = make_vector(argc, NULL);
    memcpy(minim_vector_arr(v), args, argc * sizeof(mobj*));
    return v;
}

mobj *vector_length_proc(int argc, mobj **args) {
    // (-> vector non-negative-integer?)
    mobj *v;
    
    v = args[0];
    if (!minim_is_vector(v))
        bad_type_exn("vector-length", "vector?", v);
    return Mfixnum(minim_vector_len(v));
}

mobj *vector_ref_proc(int argc, mobj **args) {
    // (-> vector non-negative-integer? any)
    mobj *v, *idx;
    
    v = args[0];
    if (!minim_is_vector(v))
        bad_type_exn("vector-ref", "vector?", v);

    idx = args[1];
    if (!minim_is_fixnum(idx) || minim_fixnum(idx) < 0)
        bad_type_exn("vector-ref", "non-negative-integer?", idx);
    if (minim_fixnum(idx) >= minim_vector_len(v))
        vector_out_of_bounds_exn("vector-ref", v, minim_fixnum(idx));

    return minim_vector_ref(v, minim_fixnum(idx));
}

mobj *vector_set_proc(int argc, mobj **args) {
    // (-> vector non-negative-integer? any void)
    mobj *v, *idx;
    
    v = args[0];
    if (!minim_is_vector(v))
        bad_type_exn("vector-set!", "vector?", v);

    idx = args[1];
    if (!minim_is_fixnum(idx) || minim_fixnum(idx) < 0)
        bad_type_exn("vector-set!", "non-negative-integer?", idx);
    if (minim_fixnum(idx) >= minim_vector_len(v))
        vector_out_of_bounds_exn("vector-set!", v, minim_fixnum(idx));

    minim_vector_ref(v, minim_fixnum(idx)) = args[2];
    return minim_void;
}

mobj *vector_fill_proc(int argc, mobj **args) {
    // (-> vector any void)
    mobj *v, *o;
    long i;

    v = args[0];
    if (!minim_is_vector(v))
        bad_type_exn("vector-fill!", "vector?", v);

    o = args[1];
    for (i = 0; i < minim_vector_len(v); ++i)
        minim_vector_ref(v, i) = o;

    return minim_void;
}

mobj *vector_to_list_proc(int argc, mobj **args) {
    // (-> vector list)
    mobj *v, *lst;
    long i;
    
    v = args[0];
    if (!minim_is_vector(v))
        bad_type_exn("vector->list", "vector?", v);

    lst = minim_null;
    for (i = minim_vector_len(v) - 1; i >= 0; --i)
        lst = Mcons(minim_vector_ref(v, i), lst);

    return lst;
}

mobj *list_to_vector_proc(int argc, mobj **args) {
    // (-> list vector)
    mobj *v, *lst, *it;
    long i;
    
    lst = args[0];
    if (!is_list(lst))
        bad_type_exn("list->vector", "list?", lst);
    return list_to_vector(lst);
}
