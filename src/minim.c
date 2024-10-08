// minim.c: entry point

#include "minim.h"

int main(int argc, char **argv) {
    printf("Minim v%s\n", MINIM_VERSION);

    GC_init();
    minim_init();

    obj x, env;

    env = empty_env();
    env_insert(env, Mintern("x"), Mfixnum(1));

    x = eval_expr(Mintern("x"), env);
    write_obj(stdout, x);
    putc('\n', stdout);
    
    minim_shutdown(0);
}
