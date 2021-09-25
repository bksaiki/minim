#ifndef _MINIM_BUILTIN_H_
#define _MINIM_BUILTIN_H_

#include "env.h"

typedef struct {
    size_t low, high;
} MinimArity;

#define DEFINE_BUILTIN_FUN(name)  MinimObject *minim_builtin_ ## name(MinimEnv *env, size_t argc, MinimObject **args);

// Variable / Control
DEFINE_BUILTIN_FUN(def_values)
DEFINE_BUILTIN_FUN(setb)
DEFINE_BUILTIN_FUN(if)
DEFINE_BUILTIN_FUN(let_values)
DEFINE_BUILTIN_FUN(letstar_values)
DEFINE_BUILTIN_FUN(begin)
DEFINE_BUILTIN_FUN(quote)
DEFINE_BUILTIN_FUN(quasiquote)
DEFINE_BUILTIN_FUN(unquote)
DEFINE_BUILTIN_FUN(lambda)
DEFINE_BUILTIN_FUN(exit)
DEFINE_BUILTIN_FUN(delay)
DEFINE_BUILTIN_FUN(force)
DEFINE_BUILTIN_FUN(values)
DEFINE_BUILTIN_FUN(callcc)

// Modules
DEFINE_BUILTIN_FUN(export)
DEFINE_BUILTIN_FUN(import)

// Transforms
DEFINE_BUILTIN_FUN(def_syntaxes)
DEFINE_BUILTIN_FUN(syntax)
DEFINE_BUILTIN_FUN(syntaxp)
DEFINE_BUILTIN_FUN(unwrap)
DEFINE_BUILTIN_FUN(syntax_case)
DEFINE_BUILTIN_FUN(to_syntax)
DEFINE_BUILTIN_FUN(template)

// Errors
DEFINE_BUILTIN_FUN(error)
DEFINE_BUILTIN_FUN(argument_error)
DEFINE_BUILTIN_FUN(arity_error)
DEFINE_BUILTIN_FUN(syntax_error)

// Miscellaneous
DEFINE_BUILTIN_FUN(equalp)
DEFINE_BUILTIN_FUN(eqvp)
DEFINE_BUILTIN_FUN(eqp)
DEFINE_BUILTIN_FUN(symbolp)
DEFINE_BUILTIN_FUN(printf)
DEFINE_BUILTIN_FUN(void)
DEFINE_BUILTIN_FUN(version);
DEFINE_BUILTIN_FUN(symbol_count);
DEFINE_BUILTIN_FUN(dump_symbols);
DEFINE_BUILTIN_FUN(def_print_method)

// Procedure
DEFINE_BUILTIN_FUN(procedurep)
DEFINE_BUILTIN_FUN(procedure_arity)

// Promise
DEFINE_BUILTIN_FUN(promisep)

// Boolean
DEFINE_BUILTIN_FUN(boolp)
DEFINE_BUILTIN_FUN(not)

// Number
DEFINE_BUILTIN_FUN(numberp)
DEFINE_BUILTIN_FUN(zerop)
DEFINE_BUILTIN_FUN(positivep)
DEFINE_BUILTIN_FUN(negativep)
DEFINE_BUILTIN_FUN(evenp)
DEFINE_BUILTIN_FUN(oddp)
DEFINE_BUILTIN_FUN(exactp)
DEFINE_BUILTIN_FUN(inexactp)
DEFINE_BUILTIN_FUN(integerp)
DEFINE_BUILTIN_FUN(nanp)
DEFINE_BUILTIN_FUN(infinitep)
DEFINE_BUILTIN_FUN(eq)
DEFINE_BUILTIN_FUN(gt)
DEFINE_BUILTIN_FUN(lt)
DEFINE_BUILTIN_FUN(gte)
DEFINE_BUILTIN_FUN(lte)
DEFINE_BUILTIN_FUN(to_exact)
DEFINE_BUILTIN_FUN(to_inexact)

// Character
DEFINE_BUILTIN_FUN(charp)
DEFINE_BUILTIN_FUN(char_eqp)
DEFINE_BUILTIN_FUN(char_gtp)
DEFINE_BUILTIN_FUN(char_ltp)
DEFINE_BUILTIN_FUN(char_gtep)
DEFINE_BUILTIN_FUN(char_ltep)
DEFINE_BUILTIN_FUN(char_ci_eqp)
DEFINE_BUILTIN_FUN(char_ci_gtp)
DEFINE_BUILTIN_FUN(char_ci_ltp)
DEFINE_BUILTIN_FUN(char_ci_gtep)
DEFINE_BUILTIN_FUN(char_ci_ltep)
DEFINE_BUILTIN_FUN(char_alphabeticp)
DEFINE_BUILTIN_FUN(char_numericp)
DEFINE_BUILTIN_FUN(char_whitespacep)
DEFINE_BUILTIN_FUN(char_upper_casep)
DEFINE_BUILTIN_FUN(char_lower_casep)
DEFINE_BUILTIN_FUN(int_to_char)
DEFINE_BUILTIN_FUN(char_to_int)
DEFINE_BUILTIN_FUN(char_upcase)
DEFINE_BUILTIN_FUN(char_downcase)

// String
DEFINE_BUILTIN_FUN(stringp)
DEFINE_BUILTIN_FUN(make_string)
DEFINE_BUILTIN_FUN(string)
DEFINE_BUILTIN_FUN(string_length)
DEFINE_BUILTIN_FUN(string_ref)
DEFINE_BUILTIN_FUN(string_setb)
DEFINE_BUILTIN_FUN(string_copy)
DEFINE_BUILTIN_FUN(string_fillb)
DEFINE_BUILTIN_FUN(substring)
DEFINE_BUILTIN_FUN(string_to_symbol)
DEFINE_BUILTIN_FUN(symbol_to_string)
DEFINE_BUILTIN_FUN(string_to_number)
DEFINE_BUILTIN_FUN(number_to_string)
DEFINE_BUILTIN_FUN(format)

// Pair
DEFINE_BUILTIN_FUN(cons)
DEFINE_BUILTIN_FUN(consp)
DEFINE_BUILTIN_FUN(car)
DEFINE_BUILTIN_FUN(cdr)

DEFINE_BUILTIN_FUN(caar)
DEFINE_BUILTIN_FUN(cadr)
DEFINE_BUILTIN_FUN(cdar)
DEFINE_BUILTIN_FUN(cddr)

DEFINE_BUILTIN_FUN(caaar)
DEFINE_BUILTIN_FUN(caadr)
DEFINE_BUILTIN_FUN(cadar)
DEFINE_BUILTIN_FUN(caddr)
DEFINE_BUILTIN_FUN(cdaar)
DEFINE_BUILTIN_FUN(cdadr)
DEFINE_BUILTIN_FUN(cddar)
DEFINE_BUILTIN_FUN(cdddr)

DEFINE_BUILTIN_FUN(caaaar)
DEFINE_BUILTIN_FUN(caaadr)
DEFINE_BUILTIN_FUN(caadar)
DEFINE_BUILTIN_FUN(caaddr)
DEFINE_BUILTIN_FUN(cadaar)
DEFINE_BUILTIN_FUN(cadadr)
DEFINE_BUILTIN_FUN(caddar)
DEFINE_BUILTIN_FUN(cadddr)
DEFINE_BUILTIN_FUN(cdaaar)
DEFINE_BUILTIN_FUN(cdaadr)
DEFINE_BUILTIN_FUN(cdadar)
DEFINE_BUILTIN_FUN(cdaddr)
DEFINE_BUILTIN_FUN(cddaar)
DEFINE_BUILTIN_FUN(cddadr)
DEFINE_BUILTIN_FUN(cdddar)
DEFINE_BUILTIN_FUN(cddddr)

// List
DEFINE_BUILTIN_FUN(list)
DEFINE_BUILTIN_FUN(listp)
DEFINE_BUILTIN_FUN(nullp)
DEFINE_BUILTIN_FUN(length)
DEFINE_BUILTIN_FUN(append)
DEFINE_BUILTIN_FUN(reverse)
DEFINE_BUILTIN_FUN(list_ref)
DEFINE_BUILTIN_FUN(map)
DEFINE_BUILTIN_FUN(andmap)
DEFINE_BUILTIN_FUN(ormap)
DEFINE_BUILTIN_FUN(apply)

// Hash table
DEFINE_BUILTIN_FUN(hash)
DEFINE_BUILTIN_FUN(hashp)
DEFINE_BUILTIN_FUN(hash_keyp)
DEFINE_BUILTIN_FUN(hash_ref)
DEFINE_BUILTIN_FUN(hash_remove)
DEFINE_BUILTIN_FUN(hash_set)
DEFINE_BUILTIN_FUN(hash_setb)
DEFINE_BUILTIN_FUN(hash_removeb)
DEFINE_BUILTIN_FUN(hash_to_list)

// Vector
DEFINE_BUILTIN_FUN(vector)
DEFINE_BUILTIN_FUN(make_vector)
DEFINE_BUILTIN_FUN(vectorp)
DEFINE_BUILTIN_FUN(vector_length)
DEFINE_BUILTIN_FUN(vector_ref)
DEFINE_BUILTIN_FUN(vector_setb)
DEFINE_BUILTIN_FUN(vector_to_list)
DEFINE_BUILTIN_FUN(list_to_vector)
DEFINE_BUILTIN_FUN(vector_fillb)

// Sequence
DEFINE_BUILTIN_FUN(sequence)
DEFINE_BUILTIN_FUN(sequencep)
DEFINE_BUILTIN_FUN(sequence_first)
DEFINE_BUILTIN_FUN(sequence_rest)
DEFINE_BUILTIN_FUN(sequence_donep)

// Math
DEFINE_BUILTIN_FUN(add)
DEFINE_BUILTIN_FUN(sub)
DEFINE_BUILTIN_FUN(mul)
DEFINE_BUILTIN_FUN(div)
DEFINE_BUILTIN_FUN(abs)
DEFINE_BUILTIN_FUN(max)
DEFINE_BUILTIN_FUN(min)
DEFINE_BUILTIN_FUN(modulo)
DEFINE_BUILTIN_FUN(remainder)
DEFINE_BUILTIN_FUN(quotient)
DEFINE_BUILTIN_FUN(numerator)
DEFINE_BUILTIN_FUN(denominator)
DEFINE_BUILTIN_FUN(gcd)
DEFINE_BUILTIN_FUN(lcm)

DEFINE_BUILTIN_FUN(floor)
DEFINE_BUILTIN_FUN(ceil)
DEFINE_BUILTIN_FUN(trunc)
DEFINE_BUILTIN_FUN(round)

DEFINE_BUILTIN_FUN(sqrt)
DEFINE_BUILTIN_FUN(exp)
DEFINE_BUILTIN_FUN(log)
DEFINE_BUILTIN_FUN(pow)
DEFINE_BUILTIN_FUN(sin)
DEFINE_BUILTIN_FUN(cos)
DEFINE_BUILTIN_FUN(tan)
DEFINE_BUILTIN_FUN(asin)
DEFINE_BUILTIN_FUN(acos)
DEFINE_BUILTIN_FUN(atan)

// Port
DEFINE_BUILTIN_FUN(current_input_port)
DEFINE_BUILTIN_FUN(current_output_port)
DEFINE_BUILTIN_FUN(call_with_input_file)
DEFINE_BUILTIN_FUN(call_with_output_file)
DEFINE_BUILTIN_FUN(with_input_from_file)
DEFINE_BUILTIN_FUN(with_output_from_file)
DEFINE_BUILTIN_FUN(portp)
DEFINE_BUILTIN_FUN(input_portp)
DEFINE_BUILTIN_FUN(output_portp)
DEFINE_BUILTIN_FUN(open_input_file)
DEFINE_BUILTIN_FUN(open_output_file)
DEFINE_BUILTIN_FUN(close_input_port)
DEFINE_BUILTIN_FUN(close_output_port)

DEFINE_BUILTIN_FUN(read)
DEFINE_BUILTIN_FUN(read_char)
DEFINE_BUILTIN_FUN(peek_char)
DEFINE_BUILTIN_FUN(char_readyp)
DEFINE_BUILTIN_FUN(eofp)

DEFINE_BUILTIN_FUN(write)
DEFINE_BUILTIN_FUN(display)
DEFINE_BUILTIN_FUN(newline)
DEFINE_BUILTIN_FUN(write_char)

// Loads a single function into the environment
void minim_load_builtin(MinimEnv *env, const char *name, MinimObjectType type, ...);

// Loads every builtin symbol in the base library.
void minim_load_builtins(MinimEnv *env);

// Loads builtins
void init_builtins();

#endif
