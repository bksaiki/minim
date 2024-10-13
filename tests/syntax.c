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
    obj p, e, x, env;
    char *str;

    p = Minput_string_port(Mstring(input));
    e = read_object(p);

    env = empty_env();
    env = prim_env(env);
    x = eval_expr(e, env);

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

int main(int argc, char **argv) {
    GC_init();
    minim_init();

    return_code = 0;

    log_test("quote", test_quote);
    log_test("if", test_if);
    log_test("begin", test_begin);
    log_test("let", test_let);
    log_test("lambda", test_lambda);

    minim_shutdown(0);
    GC_deinit();

    return return_code;
}
