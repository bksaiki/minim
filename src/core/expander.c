#include "minimpriv.h"

#define EXPAND_REC(ref, fn, exp, env, stx, analyiss) \
  if (MINIM_SYNTAX(ref) == (fn)) { return exp(env, stx, analysis); }

//
//  Analysis
//

static void
init_local_var_analysis(LocalVariableAnalysis **panalysis,
                        LocalVariableAnalysis *prev_analysis)
{
    LocalVariableAnalysis *analysis = GC_alloc(sizeof(LocalVariableAnalysis));
    init_minim_symbol_table(&analysis->symbols);
    analysis->prev = prev_analysis;
    *panalysis = analysis;
}

static void
local_var_analysis_add(LocalVariableAnalysis *analysis, const char *sym)
{
    minim_symbol_table_add(analysis->symbols, sym, minim_null);
}

static bool
local_var_analysis_contains(LocalVariableAnalysis *analysis, const char *sym)
{
    if (minim_symbol_table_get(analysis->symbols, sym) == NULL)
        return (analysis->prev ? local_var_analysis_contains(analysis->prev, sym) : false);
    return true;
}

//
//  Expander
//

static MinimObject *
expand_expr(MinimEnv *env,
            MinimObject *stx,
            LocalVariableAnalysis *analysis);

static MinimObject *
expand_definition_level(MinimEnv *env,
                        MinimObject *stx,
                        LocalVariableAnalysis *analysis);

static MinimObject *
expand_module_level_with_analysis(MinimEnv *env,
                                  MinimObject *stx,
                                  LocalVariableAnalysis *analysis);

static MinimObject *
expand_expr_let_values(MinimEnv *env,
                       MinimObject *stx,
                       LocalVariableAnalysis *analysis);

void
expand_expr_sequence(MinimEnv *env,
                     MinimObject *stx_lst,
                     LocalVariableAnalysis *analysis)
{
    for (MinimObject *it = stx_lst; !minim_nullp(it); it = MINIM_CDR(it))
    {
        MinimObject *expr = MINIM_CAR(it);
        if (!MINIM_STX_NULLP(expr) && MINIM_STX_PAIRP(expr) && MINIM_STX_SYMBOL(MINIM_STX_CAR(expr)))
        {
            MinimObject *op = env_get_sym(env, MINIM_STX_SYMBOL(MINIM_STX_CAR(expr)));
            if (op)
            {
                // apply transforms
                while (MINIM_OBJ_TRANSFORMP(op))
                {
                    MinimObject *transformed = transform_loc(env, op, expr);
                    check_syntax(env, transformed);
                    expr = expand_expr(env, transformed, analysis);
                    op = env_get_sym(env, MINIM_STX_SYMBOL(MINIM_STX_CAR(expr)));
                }

                // apply expansion of def-values
                if (MINIM_OBJ_SYNTAXP(op) && MINIM_SYNTAX(op) == minim_builtin_def_values)
                {
                    MinimObject *ids, *body;

                    ids = MINIM_STX_CADR(expr);
                    body = MINIM_CADR(MINIM_STX_CDR(expr));
                    MINIM_CAR(it) = minim_ast(minim_cons(
                        minim_ast(intern("let-values"), MINIM_STX_LOC(expr)),
                        minim_cons(
                            minim_ast(minim_cons(
                                minim_ast(minim_cons(
                                    ids,
                                    minim_cons(body, minim_null)),
                                    MINIM_STX_LOC(expr)), minim_null),
                                MINIM_STX_LOC(expr)),
                            MINIM_CDR(it))),
                        MINIM_STX_LOC(expr));
                    MINIM_CDR(it) = minim_null;

                    MINIM_CAR(it) = expand_expr_let_values(env, MINIM_CAR(it), analysis);
                    return;
                }
            }
        }

        MINIM_CAR(it) = expand_expr(env, MINIM_CAR(it), analysis);
    }
}

static MinimObject *
expand_expr_begin(MinimEnv *env,
                  MinimObject *stx,
                  LocalVariableAnalysis *analysis)
{
    expand_expr_sequence(env, MINIM_STX_CDR(stx), analysis);
    return stx;
}

static MinimObject *
expand_expr_1arg(MinimEnv *env,
                 MinimObject *stx,
                 LocalVariableAnalysis *analysis)
{
    MinimObject *arg = MINIM_STX_CDR(stx);
    MINIM_CAR(arg) = expand_expr(env, MINIM_CAR(arg), analysis);
    return stx;
}

static MinimObject *
expand_expr_if(MinimEnv *env,
               MinimObject *stx,
               LocalVariableAnalysis *analysis)
{
    MinimObject *cond, *ift, *iff;

    cond = MINIM_STX_CDR(stx);
    ift = MINIM_CDR(MINIM_STX_CDR(stx));
    iff = MINIM_CDDR(MINIM_STX_CDR(stx));
    
    MINIM_CAR(cond) = expand_expr(env, MINIM_CAR(cond), analysis);
    MINIM_CAR(ift) = expand_expr(env, MINIM_CAR(ift), analysis);
    MINIM_CAR(iff) = expand_expr(env, MINIM_CAR(iff), analysis);
    return stx;
}

static MinimObject *
expand_expr_lambda(MinimEnv *env,
                   MinimObject *stx,
                   LocalVariableAnalysis *analysis)
{
    LocalVariableAnalysis *analysis2;
    MinimObject *formals;

    init_local_var_analysis(&analysis2, analysis);
    formals = MINIM_CAR(MINIM_STX_CDR(stx));
    if (MINIM_STX_SYMBOLP(formals))
    {
        local_var_analysis_add(analysis2, MINIM_STX_SYMBOL(formals));
    }
    else
    {
        MinimObject *it = MINIM_STX_VAL(formals);
        while (!minim_nullp(it) && MINIM_OBJ_PAIRP(it))
        {
            local_var_analysis_add(analysis2, MINIM_STX_SYMBOL(MINIM_CAR(it)));
            it = MINIM_CDR(it);
        }
        
        if (!minim_nullp(it))
            local_var_analysis_add(analysis2, MINIM_STX_SYMBOL(it));
    }

    expand_expr_sequence(env, MINIM_CDR(MINIM_STX_CDR(stx)), analysis2);
    return stx;
}

static MinimObject *
expand_expr_let_values(MinimEnv *env,
                       MinimObject *stx,
                       LocalVariableAnalysis *analysis)
{
    MinimObject *ids, *body;
    LocalVariableAnalysis *analysis2;

    ids = MINIM_STX_VAL(MINIM_CAR(MINIM_STX_CDR(stx)));
    init_local_var_analysis(&analysis2, analysis);
    for (MinimObject *it = ids; !minim_nullp(it); it = MINIM_CDR(it))
    {
        MinimObject *var, *bind;
        LocalVariableAnalysis *analysis3;
        
        var = MINIM_STX_CAR(MINIM_CAR(it));
        bind = MINIM_STX_CDR(MINIM_CAR(it));
        for (var = MINIM_STX_VAL(var); !minim_nullp(var); var = MINIM_CDR(var))
            local_var_analysis_add(analysis2, MINIM_STX_SYMBOL(MINIM_CAR(var)));

        init_local_var_analysis(&analysis3, analysis2);
        MINIM_CAR(bind) = expand_expr(env, MINIM_CAR(bind), analysis3);
    }

    body = MINIM_CDR(MINIM_STX_CDR(stx));
    expand_expr_sequence(env, body, analysis2);
    return stx;
}

static MinimObject *
expand_expr_def_values(MinimEnv *env,
                       MinimObject *stx,
                       LocalVariableAnalysis *analysis)
{
    MinimObject *ids, *body;

    ids = MINIM_STX_CADR(stx);
    body = MINIM_CDR(MINIM_STX_CDR(stx));
    MINIM_CAR(body) = expand_expr(env, MINIM_CAR(body), analysis);
    for (MinimObject *it = MINIM_STX_VAL(ids); !minim_nullp(it); it = MINIM_CDR(it))
        local_var_analysis_add(analysis, MINIM_STX_SYMBOL(MINIM_CAR(it)));
    return stx;
}

static MinimObject *
expand_expr_setb(MinimEnv *env,
                 MinimObject *stx,
                 LocalVariableAnalysis *analysis)
{
    MinimObject *val = MINIM_CDR(MINIM_STX_CDR(stx));
    MINIM_CAR(val) = expand_expr(env, MINIM_CAR(val), analysis);
    return stx;
}

static MinimObject *
expand_expr_syntax_case(MinimEnv *env,
                        MinimObject *stx,
                        LocalVariableAnalysis *analysis)
{
    MinimObject *it, *clause;

    it = MINIM_STX_CDR(stx);
    MINIM_CAR(it) = expand_expr(env, MINIM_CAR(it), analysis);
    for (it = MINIM_CDDR(it); !minim_nullp(it); it = MINIM_CDR(it))
    {
        clause = MINIM_STX_CDR(MINIM_CAR(it));
        MINIM_CAR(clause) = expand_expr(env, MINIM_CAR(clause), analysis);
    }

    return stx;
}

static MinimObject *
expand_expr(MinimEnv *env,
            MinimObject *stx,
            LocalVariableAnalysis *analysis)
{
    if (MINIM_STX_PAIRP(stx))
    {
        // early exit
        if (MINIM_STX_NULLP(stx))
            return stx;
        
        // (<ident> <thing> ...)
        if (MINIM_STX_SYMBOLP(MINIM_STX_CAR(stx)))
        {
            // variable reference
            if (minim_eqvp(MINIM_STX_VAL(MINIM_STX_CAR(stx)), intern("%top")))
                return stx;
            
            MinimObject *ref = env_get_sym(env, MINIM_STX_SYMBOL(MINIM_STX_CAR(stx)));
            if (ref)
            {
                if (MINIM_OBJ_TRANSFORMP(ref))          // transformer
                {
                    MinimObject *transformed = transform_loc(env, ref, stx);
                    check_syntax(env, transformed);
                    return expand_expr(env, transformed, analysis);
                }
                else if (MINIM_OBJ_SYNTAXP(ref))        // syntax
                {
                    EXPAND_REC(ref, minim_builtin_begin, expand_expr_begin, env, stx, analysis);
                    EXPAND_REC(ref, minim_builtin_callcc, expand_expr_1arg, env, stx, analysis);
                    EXPAND_REC(ref, minim_builtin_delay, expand_expr_1arg, env, stx, analysis);
                    EXPAND_REC(ref, minim_builtin_if, expand_expr_if, env, stx, analysis);
                    EXPAND_REC(ref, minim_builtin_lambda, expand_expr_lambda, env, stx, analysis);
                    EXPAND_REC(ref, minim_builtin_let_values, expand_expr_let_values, env, stx, analysis);
                    EXPAND_REC(ref, minim_builtin_setb, expand_expr_setb, env, stx, analysis);
                    EXPAND_REC(ref, minim_builtin_syntax_case, expand_expr_syntax_case, env, stx, analysis);

                    EXPAND_REC(ref, minim_builtin_def_values, expand_expr_def_values, env, stx, analysis);

                    // minim_builtin_quote
                    // minim_builtin_quasiquote
                    // minim_builtin_unquote
                    // minim_builtin_syntax
                    // minim_builtin_template
                    return stx;
                }
            }

            for (MinimObject *it = MINIM_STX_VAL(stx); !minim_nullp(it); it = MINIM_CDR(it))
                MINIM_CAR(it) = expand_expr(env, MINIM_CAR(it), analysis);
            return stx;
        }
        else
        {
            for (MinimObject *it = MINIM_STX_VAL(stx); !minim_nullp(it); it = MINIM_CDR(it))
                MINIM_CAR(it) = expand_expr(env, MINIM_CAR(it), analysis);
            return stx;
        }
    }
    else if (MINIM_STX_SYMBOLP(stx))
    {
        if (local_var_analysis_contains(analysis, MINIM_STX_SYMBOL(stx)))
        {
            return stx;
        }
        else
        {
            MinimObject *trait = minim_ast(intern("%top"), MINIM_STX_LOC(stx));
            return minim_ast(minim_cons(trait, stx), MINIM_STX_LOC(stx));
        }
    }
    else
    {
        return minim_ast(minim_cons(
            minim_ast(intern("quote"), MINIM_STX_LOC(stx)),
            minim_cons(stx, minim_null)),
            MINIM_STX_LOC(stx));
    }
}

static MinimObject *
expand_definition_level(MinimEnv *env,
                        MinimObject *stx,
                        LocalVariableAnalysis *analysis)
{
    MinimObject *car, *ref;

    if (!MINIM_STX_PAIRP(stx))
        return expand_expr(env, stx, analysis);

    if (MINIM_STX_NULLP(stx))
        return stx;

    // (<ident> <thing> ...)
    car = MINIM_STX_CAR(stx);
    if (MINIM_STX_SYMBOLP(car))
    {
        if (minim_eqp(MINIM_STX_VAL(car), intern("%import")))
        {
            eval_top_level(env, stx, minim_builtin_import);
            return stx;
        }

        if (minim_eqp(MINIM_STX_VAL(car), intern("def-syntaxes")))
        {
            eval_top_level(env, stx, minim_builtin_def_syntaxes);
            return stx;
        }

        ref = env_get_sym(env, MINIM_STX_SYMBOL(MINIM_STX_CAR(stx)));
        if (ref)
        {
            if (MINIM_OBJ_TRANSFORMP(ref))          // transformer
            {
                MinimObject *transformed = transform_loc(env, ref, stx);
                return expand_module_level_with_analysis(env, transformed, analysis);
            }
            else if (MINIM_OBJ_SYNTAXP(ref) &&
                     (MINIM_SYNTAX(ref) == minim_builtin_def_values ||
                      MINIM_SYNTAX(ref) == minim_builtin_def_syntaxes))
            {
                MinimObject *val = MINIM_CDR(MINIM_STX_CDR(stx));
                MINIM_CAR(val) = expand_expr(env, MINIM_CAR(val), analysis);
                return stx;
            }
        }
    }

    return expand_expr(env, stx, analysis);
}

static MinimObject *
expand_module_level_with_analysis(MinimEnv *env,
                                  MinimObject *stx,
                                  LocalVariableAnalysis *analysis)
{
    MinimObject *car;

    if (!MINIM_STX_PAIRP(stx))
        return expand_expr(env, stx, analysis);

    car = MINIM_STX_CAR(stx);
    if (MINIM_OBJ_ASTP(car))
    {
        car = MINIM_STX_VAL(car);
        if (minim_eqp(car, intern("%export")))
            return stx;

        if (minim_eqp(car, intern("begin")))
        {
            for (MinimObject *it = MINIM_STX_CDR(stx); !minim_nullp(it); it = MINIM_CDR(it))
                MINIM_CAR(it) = expand_module_level_with_analysis(env, MINIM_CAR(it), analysis);
            return stx;
        }
    }
    
    return expand_definition_level(env, stx, analysis);
}

static MinimObject *expand_top_level(MinimEnv *env, MinimObject *stx)
{
    MinimObject *car;
    LocalVariableAnalysis *analysis;

    if (!MINIM_STX_PAIRP(stx))
    {
        init_local_var_analysis(&analysis, NULL);
        return expand_expr(env, stx, analysis);
    }

    car = MINIM_STX_CAR(stx);
    if (MINIM_OBJ_ASTP(car))
    {
        car = MINIM_STX_VAL(car);
        if (minim_eqp(car, intern("%module")))
        {
            MinimObject *t = MINIM_CDDR(MINIM_STX_CDR(stx));
            for (t = MINIM_STX_CDR(MINIM_CAR(t)); !minim_nullp(t); t = MINIM_CDR(t))
            {
                init_local_var_analysis(&analysis, NULL);   
                MINIM_CAR(t) = expand_module_level_with_analysis(env, MINIM_CAR(t), analysis);
            }
        }
    }

    return stx;
}


// ================================ Public ================================

MinimObject *expand_module_level(MinimEnv *env, MinimObject *stx)
{
    LocalVariableAnalysis *analysis;
    init_local_var_analysis(&analysis, NULL);
    return expand_module_level_with_analysis(env, stx, analysis);
}

void expand_minim_module(MinimEnv *env, MinimModule *module)
{
    module->body = expand_top_level(env, module->body);
}