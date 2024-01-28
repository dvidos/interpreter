#ifndef _SYMBOL_TABLE_H
#define _SYMBOL_TABLE_H

#include "../utils/failable.h"
#include "../utils/containers/_module.h"
#include "../utils/data_types/variant.h"

typedef struct symbol_table {
    dict *symbols;
    struct symbol_table *parent;
} symbol_table;


symbol_table *new_symbol_table(symbol_table *parent);

failable register_symbol(symbol_table *st, const char *name, variant *v);
bool symbol_exists(symbol_table *current, const char *name);
variant *resolve_symbol(symbol_table *current, const char *name);
failable update_symbol(symbol_table *current, const char *name, variant *new_value);

#endif
