// minim.c: entry point

#include "minim.h"

int main(int argc, char **argv) {
    obj tc, ip, e, v;

    printf("Minim v%s\n", MINIM_VERSION);

    GC_init();
    minim_init();

    tc = Mcurr_tc();
    ip = Minput_file_port(stdin);
    Mtc_env(tc) = prim_env(empty_env());

    while (1) {
        fputs("> ", stdout);
        fflush(stdout);

        e = read_object(ip);
        v = eval_expr(e);
    
        write_obj(stdout, v);
        fputc('\n', stdout);
        fflush(stdout);
    }
    
    minim_shutdown(0);
}
