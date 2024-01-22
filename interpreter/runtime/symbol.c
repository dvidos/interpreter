#include <stdlib.h>
#include <string.h>
#include "symbol.h"


symbol *new_built_in_function_symbol(const char *name, callable *c) {
    symbol *s = malloc(sizeof(symbol));
    s->name = name;
    s->type = BUILT_IN_FUNCTION;
    s->per_type.built_in.call = c;
    return s;
}

symbol *new_user_function_symbol(const char *name, list *arg_names, list *statements) {
    symbol *s = malloc(sizeof(symbol));
    s->name = name;
    s->type = USER_FUNCTION;
    s->per_type.user_func.arg_names = arg_names;
    s->per_type.user_func.statements = statements;
    return s;
}

symbol *new_variable_symbol(const char *name, variant *value) {
    symbol *s = malloc(sizeof(symbol));
    s->name = name;
    s->type = VARIABLE;
    s->per_type.variable.value = value;
    return s;
}

const char *symbol_to_string(symbol *s) {
    // simple for now
    return s->name;
}

bool symbols_are_equal(symbol *a, symbol *b) {
    if (a == NULL && b == NULL) return true;
    if ((a == NULL && b != NULL) || (a != NULL && b == NULL)) return false;
    if (a == b) return true;
    
    if (strcmp(a->name, b->name) != 0) return false;
    if (a->type != b->type) return false;
    switch (a->type) {
        case BUILT_IN_FUNCTION:
            if (!callables_are_equal(a->per_type.built_in.call, b->per_type.built_in.call))
                return false;
            break;
        case USER_FUNCTION:
            if (!lists_are_equal(a->per_type.user_func.arg_names, b->per_type.user_func.arg_names))
                return false;
            if (!lists_are_equal(a->per_type.user_func.statements, b->per_type.user_func.statements))
                return false;
            break;
        case VARIABLE:
            if (!variants_are_equal(a->per_type.variable.value, b->per_type.variable.value))
                return false;
            break;
    }

    return true;
}

contained_item *containing_symbols = &(contained_item) {
    .type_name = "symbol",
    .to_string = (to_string_func)symbol_to_string,
    .are_equal = (are_equal_func)symbols_are_equal,
    .hash = NULL
};

