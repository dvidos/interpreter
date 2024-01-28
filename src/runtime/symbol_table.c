#include <stdlib.h>
#include "symbol_table.h"

symbol_table *new_symbol_table(symbol_table *parent) {
    symbol_table *st = malloc(sizeof(symbol_table));
    st->symbols = new_dict(containing_variants, 64);
    st->parent = parent;
    return st;
}

failable register_symbol(symbol_table *st, const char *name, variant *v) {
    if (dict_has(st->symbols, name))
        return failed(NULL, "symbol %s already registered", name);
    dict_set(st->symbols, name, v);
    return ok();
}

bool symbol_exists(symbol_table *current, const char *name) {
    while (current != NULL) {
        if (dict_has(current->symbols, name))
            return true;
        current = current->parent;
    }
    return false;
}


variant *resolve_symbol(symbol_table *current, const char *name) {
    while (current != NULL) {
        if (dict_has(current->symbols, name))
            return dict_get(current->symbols, name);
        current = current->parent;
    }
    return NULL;
}

failable update_symbol(symbol_table *current, const char *name, variant *new_value) {
    while (current != NULL) {
        if (dict_has(current->symbols, name)) {
            dict_set(current->symbols, name, new_value);
            return ok();
        }
        current = current->parent;
    }
    return failed(NULL, "Symbol %s not found", name);
}
