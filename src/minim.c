// minim.c: entry point

#include "minim.h"

int main(int argc, char **argv) {
    printf("Minim v%s\n", MINIM_VERSION);

    GC_init();
    minim_init();

    obj x, e, env;

    env = empty_env();
    env = prim_env(env);
    env_insert(env, Mintern("x"), Mfixnum(1));

    // e = Mlist3(Mintern("cons"), Mintern("x"), Mfixnum(2));
    // e = Mlist2(Mintern("cdr"), e);
    e = Mlist4(Mintern("if"), Mfalse, Mfixnum(1), Mfixnum(0));
    x = eval_expr(e, env);

    // x = eval_expr(Mintern("cons"), env);
    write_obj(stdout, x);
    putc('\n', stdout);
    
    minim_shutdown(0);
}
