// minim.c: entry point

#include "minim.h"

int main(int argc, char **argv) {
    obj tc, e, v;

    printf("Minim v%s\n", MINIM_VERSION);

    GC_init();
    minim_init();

    tc = Mcurr_tc();
    Mtc_env(tc) = prim_env(Mtc_env(tc));

    load_kernel();

    while (1) {
        fputs("> ", stdout);
        fflush(stdout);

        e = read_object(Mtc_ip(tc));
        v = eval_expr(e);
    
        write_obj(stdout, v);
        fputc('\n', stdout);
        fflush(stdout);
    }
    
    minim_shutdown(0);
}
