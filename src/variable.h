#ifndef _MINIM_VARIABLE_H_
#define _MINIM_VARIABLE_H_

#include "env.h"

// Internals

bool minim_symbolp(MinimObject *obj);
bool assert_symbol(MinimObject *obj, MinimObject **res, const char* msg);

// Builtins

void env_load_module_variable(MinimEnv *env);

MinimObject *minim_builtin_if(MinimEnv *env, int argc, MinimObject **args);
MinimObject *minim_builtin_def(MinimEnv *env, int argc, MinimObject **args);
MinimObject *minim_builtin_let(MinimEnv *env, int argc, MinimObject **args);
MinimObject *minim_builtin_letstar(MinimEnv *env, int argc, MinimObject **args);
MinimObject *minim_builtin_quote(MinimEnv *env, int argc, MinimObject **args);
MinimObject *minim_builtin_setb(MinimEnv *env, int argc, MinimObject **args);

MinimObject *minim_builtin_begin(MinimEnv *env, int argc, MinimObject **args);

MinimObject *minim_builtin_symbolp(MinimEnv *env, int argc, MinimObject **args);
MinimObject *minim_builtin_equalp(MinimEnv *env, int argc, MinimObject **args);

#endif