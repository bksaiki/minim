#include <setjmp.h>
#include <string.h>

#include "../gc/gc.h"
#include "arity.h"
#include "ast.h"
#include "builtin.h"
#include "error.h"
#include "eval.h"
#include "jmp.h"
#include "lambda.h"
#include "list.h"
#include "tail_call.h"

//
//  Builtins
//

MinimObject *minim_builtin_if(MinimEnv *env, size_t argc, MinimObject **args)
{
    MinimObject *res, *cond;

    eval_ast_no_check(env, MINIM_AST(args[0]), &cond);
    eval_ast_no_check(env, (coerce_into_bool(cond) ? MINIM_AST(args[1]) : MINIM_AST(args[2])), &res);
    return res;
}

MinimObject *minim_builtin_let_values(MinimEnv *env, size_t argc, MinimObject **args)
{
    MinimObject *val;
    SyntaxNode *binding, *names;
    MinimEnv *env2;
    
    // Bind names and values
    init_env(&env2, env, NULL);
    for (size_t i = 0; i < MINIM_AST(args[0])->childc; ++i)
    {
        binding = MINIM_AST(args[0])->children[i];
        eval_ast_no_check(env, binding->children[1], &val);
        names = binding->children[0];
        if (!MINIM_OBJ_VALUESP(val))
        {
            if (names->childc != 1)
                return minim_values_arity_error("let-values", names->childc, 1, names);
            
            env_intern_sym(env2, names->children[0]->sym, val);
            if (MINIM_OBJ_CLOSUREP(val))
                env_intern_sym(MINIM_CLOSURE(val)->env, names->children[0]->sym, val);
        }
        else
        {
            if (MINIM_VALUES_SIZE(val) != names->childc)
                return minim_values_arity_error("let-values", names->childc, MINIM_VALUES_SIZE(val), names);

            for (size_t i = 0; i < names->childc; ++i)
            {
                env_intern_sym(env2, names->children[i]->sym, MINIM_VALUES_REF(val, i));
                if (MINIM_OBJ_CLOSUREP(MINIM_VALUES_REF(val, i)))
                    env_intern_sym(MINIM_CLOSURE(MINIM_VALUES_REF(val, i))->env,
                                   names->children[0]->sym,
                                   MINIM_VALUES_REF(val, i));
            }
        }
    }

    // Evaluate body
    return minim_builtin_begin(env2, argc - 1, &args[1]);
}

MinimObject *minim_builtin_letstar_values(MinimEnv *env, size_t argc, MinimObject **args)
{
    MinimObject *val;
    SyntaxNode *binding, *names;
    MinimEnv *env2;
    
    // Bind names and values
    init_env(&env2, env, NULL);
    for (size_t i = 0; i < MINIM_AST(args[0])->childc; ++i)
    {
        binding = MINIM_AST(args[0])->children[i];
        names = binding->children[0];
        eval_ast_no_check(env2, binding->children[1], &val);
        if (!MINIM_OBJ_VALUESP(val))
        {
            if (names->childc != 1)
                return minim_values_arity_error("let*-values", names->childc, 1, names);
            
            env_intern_sym(env2, names->children[0]->sym, val);
            if (MINIM_OBJ_CLOSUREP(val))
                env_intern_sym(MINIM_CLOSURE(val)->env, names->children[0]->sym, val);
        }
        else
        {
            if (MINIM_VALUES_SIZE(val) != names->childc)
                return minim_values_arity_error("let*-values", names->childc, MINIM_VALUES_SIZE(val), names);

            for (size_t i = 0; i < names->childc; ++i)
            {
                env_intern_sym(env2, names->children[i]->sym, MINIM_VALUES_REF(val, i));
                if (MINIM_OBJ_CLOSUREP(MINIM_VALUES_REF(val, i)))
                    env_intern_sym(MINIM_CLOSURE(MINIM_VALUES_REF(val, i))->env,
                                   names->children[0]->sym,
                                   MINIM_VALUES_REF(val, i));
            }
        }
    }

    // Evaluate body
    return minim_builtin_begin(env2, argc - 1, &args[1]);
}

MinimObject *minim_builtin_begin(MinimEnv *env, size_t argc, MinimObject **args)
{
    MinimObject *res, *val;
    MinimEnv *env2;

    if (argc == 0)
        return minim_void;

    init_env(&env2, env, NULL);
    for (size_t i = 0; i < argc; ++i)
    {
        eval_ast_no_check(env2, MINIM_AST(args[i]), &val);
        if (i + 1 == argc)
            res = val;
    }

    return res;
}

MinimObject *minim_builtin_case(MinimEnv *env, size_t argc, MinimObject **args)
{
    MinimObject *res, *key;
    MinimEnv *env2;

    if (argc < 2)
        return minim_void;
        

    unsyntax_ast(env, MINIM_AST(args[0]), &key);
    for (size_t i = 1; i < argc; ++i)
    {
        MinimObject *ce_pair, *cs, *val;

        unsyntax_ast(env, MINIM_AST(args[i]), &ce_pair);
        unsyntax_ast_rec(env, MINIM_AST(MINIM_CAR(ce_pair)), &cs);
        if (minim_nullp(cs))
            continue;

        for (MinimObject *it = cs; !minim_nullp(it); it = MINIM_CDR(it))
        {
            if (minim_equalp(MINIM_CAR(it), key))
            {
                if (minim_list_length(ce_pair) > 2)
                {
                    init_env(&env2, env, NULL);
                    for (MinimObject *it = MINIM_CDR(ce_pair); !minim_nullp(it); it = MINIM_CDR(it))
                    {
                        eval_ast_no_check(env2, MINIM_AST(MINIM_CAR(it)), &val);
                        if (minim_nullp(MINIM_CDR(it)))
                            return val;   
                    }
                }
                else
                {
                    eval_ast_no_check(env, MINIM_AST(MINIM_CADR(ce_pair)), &res);
                    return res;
                }
            }
        }
    }

    return minim_void;
}

MinimObject *minim_builtin_values(MinimEnv *env, size_t argc, MinimObject **args)
{
    return minim_values(argc, args);
}

MinimObject *minim_builtin_callcc(MinimEnv *env, size_t argc, MinimObject **args)
{
    MinimArity arity;
    MinimObject *proc, *cont, *val;
    jmp_buf *jmp;

    eval_ast_no_check(env, MINIM_AST(args[0]), &proc);
    if (!MINIM_OBJ_CLOSUREP(proc))
        return minim_argument_error("procedure of 1 argument", "call/cc", 0, proc);

    minim_get_lambda_arity(MINIM_CLOSURE(proc), &arity);
    if (arity.low != 1 || arity.high != 1)
        return minim_argument_error("procedure of 1 argument", "call/cc", 0, proc);

    jmp = GC_alloc_atomic(sizeof(jmp_buf));
    cont = minim_jmp(jmp, NULL);
    if (setjmp(*jmp) == 0)   // continuation not used
    {
        val = eval_lambda(MINIM_CLOSURE(proc), env, 1, &cont);
        return val; 
    }
    else                    // continuation used
    {
        return MINIM_JUMP_VAL(cont);
    }
}

MinimObject *minim_builtin_exit(MinimEnv *env, size_t argc, MinimObject **args)
{
    if (minim_exit_handler != NULL)     // no return
        minim_long_jump(minim_exit_handler, env, argc, args);

    printf("exit handler not set up\n");
    return NULL;    // panic
}
