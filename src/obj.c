// obj.c: objects

#include "minim.h"

int Meqp(obj x, obj y) {
    if (x == y) {
        return 1;
    } else if (obj_type(x) != obj_type(y)) {
        return 0;
    } else {
        switch (obj_type(x)) {
        // fixnums (compare numerical values)
        case FIXNUM_OBJ_TYPE:
            return Mfixnum_value(x) == Mfixnum_value(y);
        default:
            return 0;
        }
    }
}

int Mequalp(obj x, obj y) {
    if (Meqp(x, y)) {
        return 1;
    } else if (obj_type(x) != obj_type(y)) {
        return 0;
    } else {
        switch (obj_type(x)) {
        case STRING_OBJ_TYPE:
            return strcmp(Mstring_value(x), Mstring_value(y)) == 0;
        case CONS_OBJ_TYPE:
            return Mequalp(Mcar(x), Mcar(y)) && Mequalp(Mcdr(x), Mcdr(y));
        default:
            return 0;
        }
    }
}
