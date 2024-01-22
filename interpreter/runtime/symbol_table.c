#include <stdlib.h>
#include "symbol_table.h"

symbol_table *new_symbol_table(symbol_table *parent) {
    symbol_table *st = malloc(sizeof(symbol_table));
    st->symbols = new_dict(containing_symbols, 64);
    st->parent = parent;
    return st;
}

failable register_symbol(symbol_table *st, const char *name, symbol *s) {
    if (dict_has(st->symbols, name)) {
        return failed("symbol %s already registered", name);
    }
    dict_set(st->symbols, name, s);
    return ok();
}

symbol *resolve_symbol(symbol_table *current, const char *name) {
    while (current != NULL) {
        if (dict_has(current->symbols, name))
            return dict_get(current->symbols, name);
        current = current->parent;
    }
    return NULL;
}
