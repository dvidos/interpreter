#ifndef _SYMBOL_H
#define _SYMBOL_H

#include "../../utils/data_types/callable.h"
#include "../../utils/containers/_module.h"

typedef enum symbol_type symbol_type;

enum symbol_type {
    BUILT_IN_FUNCTION,
    USER_FUNCTION,
    VARIABLE,
};

typedef struct symbol {
    const char *name;
    symbol_type type;
    union {
        struct built_in {
            callable *call;
        } built_in;
        struct user_func {
            list *arg_names;
            list *statements;
        } user_func;
        struct variable {
            variant *value;
        } variable;
    } per_type;
} symbol;


symbol *new_built_in_function_symbol(const char *name, callable *c);
symbol *new_user_function_symbol(const char *name, list *arg_names, list *statements);
symbol *new_variable_symbol(const char *name, variant *value);

const char *symbol_to_string(symbol *s);
bool symbols_are_equal(symbol *a, symbol *b);

extern contained_item *containing_symbols;

#endif
