// prims.c: tests for primitives

#include "test.h"

int return_code, passed;

char *write_debug(obj o) {
    FILE *stream;
    char *buffer;
    size_t len, read;

    stream = tmpfile();
    write_obj(stream, o);
    len = ftell(stream);
    fseek(stream, 0, SEEK_SET);

    buffer = GC_malloc_atomic((len + 1) * sizeof(char));
    read = fread(buffer, 1, len, stream);
    buffer[len] = '\0';
    if (read != len) {
        fprintf(stderr, "read error occured");
        exit(1);
    }

    fclose(stream);
    return buffer;
}

void check_equal(const char *input, const char *expect) {
    obj p, e, tc, x;
    char *str;

    p = Minput_string_port(Mstring(input));
    e = read_object(p);

    tc = Mcurr_tc();
    Mtc_env(tc) = prim_env(empty_env());
    x = eval_expr(e);

    str = write_debug(x);
    if (strcmp(str, expect) != 0) {
        log_failed_case(input, expect, str);
        passed = 0;
    }
}

int test_callwv(void) {
    passed = 1;

    check_equal("(call-with-values (lambda () (values)) (lambda xs xs))", "()");
    check_equal("(call-with-values (lambda () (values 1)) (lambda xs xs))", "(1)");
    check_equal("(call-with-values (lambda () (values 1 2 3)) (lambda xs xs))", "(1 2 3)");
    check_equal("(call-with-values (lambda () (values 1 2)) fx2+)", "3");

    return passed;
}


int test_list(void) {
    passed = 1;

    check_equal("(list)", "()");
    check_equal("(list 1)", "(1)");
    check_equal("(list 1 2)", "(1 2)");
    check_equal("(list 1 2 3)", "(1 2 3)");

    check_equal("(length '())", "0");
    check_equal("(length '(1))", "1");
    check_equal("(length '(1 2))", "2");
    check_equal("(length '(1 2 3))", "3");

    check_equal("(reverse '())", "()");
    check_equal("(reverse '(1))", "(1)");
    check_equal("(reverse '(1 2))", "(2 1)");
    check_equal("(reverse '(1 2 3))", "(3 2 1)");

    check_equal("(append '() '())", "()");
    check_equal("(append '(1) '())", "(1)");
    check_equal("(append '(1 2 3) '())", "(1 2 3)");
    check_equal("(append '() '(1))", "(1)");
    check_equal("(append '() '(1 2 3))", "(1 2 3)");
    check_equal("(append '(a b c) '(d))", "(a b c d)");
    check_equal("(append '(a b c) '(d e f))", "(a b c d e f)");


    return passed;
}

int test_callcc(void) {
    passed = 1;

    check_equal("(call/cc (lambda (k) 1))", "1");
    check_equal("(call/cc (lambda (k) (k 1) 2))", "1");
    check_equal("(let ([x #f]) (cons 1 (call/cc (lambda (k) (set! x k) 2))))", "(1 . 2)");

    // from ChezScheme documentation
    check_equal("(call/cc (lambda (k) (fx2* 5 (k 4))))", "4");
    check_equal("(fx2+ 2 (call/cc (lambda (k) (fx2* 5 (k 4)))))", "6");
    check_equal("(letrec ([product "
                  "(lambda (xs) "
                    "(call/cc "
                      "(lambda (break) "
                        "(if (null? xs) "
                            "1 "
                            "(if (fx2= (car xs) 0) "
                                "(break 0) "
                                "(fx2* (car xs) (product (cdr xs))))))))]) "
                  "(product '(7 3 8 0 1 9 5)))",
                "0");
    check_equal("(let ([x (call/cc (lambda (k) k))]) "
                  "(x (lambda (ignore) \"hi\")))",
                "\"hi\"");
    
    check_equal("(letrec ([k* #f] "
                         "[y (fx1+ (call/cc (lambda (k) (set! k* k) 0)))]) "
                  "(if (fx2< y 5) "
                      "(k* y) "
                      "y))",
                "5");

    return passed;
}

int test_dynamic_wind(void) {
    passed = 1;

    check_equal("(dynamic-wind (lambda () 1) (lambda () 2) (lambda () 3))", "2");
    check_equal("(dynamic-wind (lambda () (values)) (lambda () 1) (lambda () (values 1 2)))", "1");

    check_equal(
        "(let ((path '()) (c #f)) "
          "(let ((add (lambda (s) "
                       "(set! path (cons s path))))) "
            "(dynamic-wind "
              "(lambda () (add 'connect)) "
              "(lambda () "
                "(add (call/cc "
                        "(lambda (c0) "
                          "(set! c c0) "
                          "'talk1)))) "
              "(lambda () (add 'disconnect))) "
           "(if (fx2< (length path) 4) "
               "(c 'talk2) "
               "(reverse path))))",
        "(connect talk1 disconnect connect talk2 disconnect)"
    );

    return passed;
}


int main(int argc, char **argv) {
    GC_init();
    minim_init();

    return_code = 0;

    log_test("list", test_list, return_code);
    log_test("call-with-values", test_callwv, return_code);
    log_test("call/cc", test_callcc, return_code);
    log_test("dynamic-wind", test_dynamic_wind, return_code);

    minim_shutdown(return_code);
}

