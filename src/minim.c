// minim.c: entry point

#include "minim.h"

int main(int argc, char **argv) {
    printf("Minim v%s\n", MINIM_VERSION);

    GC_init();
    minim_init();

    eval_expr(Mfixnum(1), empty_env());
    
    minim_shutdown(0);
}
