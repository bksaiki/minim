// syntax.c: tests for evaluation

#include "../src/minim.h"

int return_code, passed;

#define log_test(name, t) {             \
    if (t() == 1) {                     \
        printf("[ \033[32mPASS\033[0m ] %s\n", name);  \
    } else {                            \
        return_code = 1;                \
        printf("[ \033[31mFAIL\033[0m ] %s\n", name);  \
    }                                   \
}

#define log_failed_case(s, expect, actual) {                        \
    printf(" %s => expected: %s, actual: %s\n", s, expect, actual); \
}

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

int test_quote(void) {
    passed = 1;

    check_equal("'1", "1");
    check_equal("'a", "a");
    check_equal("'foo", "foo");
    check_equal("'()", "()");
    check_equal("'(1)", "(1)");
    check_equal("'(1 2 3)", "(1 2 3)");

    return passed;
}

int test_if(void) {
    passed = 1;

    check_equal("(if #f 1 0)", "0");
    check_equal("(if #t 1 0)", "1");
    check_equal("(if 'a 1 0)", "1");
    check_equal("(if 1 1 0)", "1");

    return passed;
}

int test_begin(void) {
    passed = 1;

    check_equal("(begin)", "#<void>");
    check_equal("(begin 1)", "1");
    check_equal("(begin 1 2 3)", "3");
    check_equal("(begin (begin) (begin 1 2) 3)", "3");

    return passed;
}

int test_let(void) {
    passed = 1;

    check_equal("(let () 1)", "1");
    check_equal("(let ([x 1]) x)", "1");
    check_equal("(let ([x 1] [y 2]) y)", "2");
    check_equal("(let ([x 1] [y 2] [z 3]) (cons x (cons y z)))", "(1 2 . 3)");

    check_equal("(let ([x 1]) (cons (let ([x 2]) x) x))", "(2 . 1)");
    check_equal("(let ([x 1]) (begin (let ([x 2]) x) x))", "1");
    check_equal("(let ([x 1]) (if (let ([x 2]) #t) x 0))", "1");
    check_equal("(let ([x 1]) (if (let ([x 2]) #f) x 0))", "0");

    return passed;
}

int test_letrec(void) {
    passed = 1;

    check_equal("(letrec () 1)", "1");
    check_equal("(letrec ([x 1]) x)", "1");
    check_equal("(letrec ([x 1] [y 2]) y)", "2");
    check_equal("(letrec ([x 1] [y 2] [z 3]) (cons x (cons y z)))", "(1 2 . 3)");

    check_equal("(letrec ([x 1] [y x]) y)", "1");
    check_equal("(letrec ([f (lambda () f)]) (f))", "#<procedure>");
    check_equal("(letrec ([f (lambda () f)] [g (lambda () f)]) (g))", "#<procedure>");
    check_equal("(letrec ([f (lambda () g)] [g (lambda () f)]) (g))", "#<procedure>");

    return passed;
}

int test_let_loop(void) {
    passed = 1;

    check_equal("(let f () 1)", "1");
    check_equal("(let f ([x 5]) (if (fx2<= x 0) #t (f (fx1- x))))", "#t");
    check_equal("(let f ([x 5] [y 3]) (if (fx2<= x 0) y (f (fx1- x) (fx1+ y))))", "8");

    return passed;
}

int test_lambda(void) {
    passed = 1;

    check_equal("(lambda () 1)", "#<procedure>");
    check_equal("(lambda xs xs)", "#<procedure>");
    check_equal("(lambda (x) x)", "#<procedure>");
    check_equal("(lambda (x y z) (cons x z))", "#<procedure>");
    check_equal("(lambda (x y . zs) (cons x zs))", "#<procedure>");

    check_equal("((lambda () 1))", "1");
    check_equal("((lambda xs xs))", "()");
    check_equal("((lambda (x) x) 1)", "1");
    check_equal("((lambda (x y z) (cons x z)) 1 2 3)", "(1 . 3)");
    check_equal("((lambda (x y . zs) (cons x zs)) 1 2)", "(1)");
    check_equal("((lambda (x y . zs) (cons x zs)) 1 2 3)", "(1 3)");
    check_equal("((lambda (x y . zs) (cons x zs)) 1 2 3 4 5)", "(1 3 4 5)");

    return passed;
}

int test_setb(void) {
    passed = 1;

    check_equal("(let ([x 1]) (set! x 2) x)", "2");
    check_equal("(let ([x 1] [y 2]) (set! x 2) (set! y 3) (cons x y))", "(2 . 3)");
    check_equal("(let ([x 1]) ((lambda () (set! x 2))) x) ", "2");
    check_equal("(let ([x 1]) ((lambda (y) (set! x 2) (cons x y)) x)) ", "(2 . 1)");

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

int main(int argc, char **argv) {
    GC_init();
    minim_init();

    return_code = 0;

    log_test("quote", test_quote);
    log_test("if", test_if);
    log_test("begin", test_begin);
    log_test("let", test_let);
    log_test("letrec", test_letrec);
    log_test("let (loop)", test_let_loop);
    log_test("lambda", test_lambda);
    log_test("set!", test_setb);
    log_test("call/cc", test_callcc);

    minim_shutdown(return_code);
}
