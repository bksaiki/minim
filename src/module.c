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

void module_import(obj spec) {
    obj tc, c_kernel;

    tc = Mcurr_tc();
    if (Mconsp(spec)) {
        minim_error1("do_import()", "unimplemented", spec);
    } else if (Msymbolp(spec)) {
        c_kernel = Mintern("#%c-kernel");
    
        if (spec == c_kernel) {
            import_env(Mtc_env(tc), prim_env(empty_env()));
        } else if (spec == Mkernel_symbol) {
            if (Mkernel == NULL) {
                minim_error("import", "attempting to load uninitialized kernel");
            }

            import_env(Mtc_env(tc), Mmodule_env(Mkernel));
        } else {
            minim_error1("import", "cannot find library", spec);
        }
    } else {
        minim_error1("module_import()", "unreachable", spec);
    }
}

void module_export(obj mod, obj spec) {
    obj tc, cell;

    tc = Mcurr_tc();
    if (Mconsp(spec)) {
        minim_error1("module_export()", "unimplemented", spec);
    } else if (Msymbolp(spec)) {
        cell = env_find(Mtc_env(tc), spec);
        if (Mfalsep(cell)) {
            minim_error1("export", "unbound identifier", spec);
        }

        env_insert(Mmodule_env(mod), spec, Mcdr(cell));
    } else {
        minim_error1("module_export()", "unreachable", spec);
    }
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
    
    // set kernel global
    Mkernel = mod;
    GC_add_roots(Mkernel, ptr_add(Mkernel, Mmodule_size));
}   
