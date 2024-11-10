// module.c: modules

#include "minim.h"

static obj read_file(obj ip) {
    obj hd, tl, e;
    
    hd = tl = Mnull;
    while (port_readyp(ip)) {
        e = read_object(ip);
        if (Meofp(e))
            break;

        if (Mnullp(hd)) {
            hd = tl = Mlist1(e);
        } else {
            Mcdr(tl) = Mlist1(e);
            tl = Mcdr(tl);
        }
    }

    return hd;
}

void load_kernel(void) {
    obj ip, es, mod;
    FILE *f;

    if ((f = fopen(KERNEL_PATH, "r")) == NULL) {
        minim_error("load_kernel()", "could not open kernel");
    }

    // read expressions from file
    ip = Minput_file_port(f);
    es = read_file(ip);

    // create a module for the kernel and load it
    mod = Mmodule(Mintern("#%kernel"), Mfalse, es);
    eval_module(mod);
}   
