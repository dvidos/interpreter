#ifndef _SYMBOL_TABLE_H
#define _SYMBOL_TABLE_H

#include "../../utils/failable.h"
#include "../../utils/containers/_module.h"
#include "symbol.h"

typedef struct symbol_table {
    dict *symbols;
    struct symbol_table *parent;
} symbol_table;


symbol_table *new_symbol_table(symbol_table *parent);

failable register_symbol(symbol_table *st, const char *name, symbol *s);

symbol *resolve_symbol(symbol_table *current, const char *name);


#endif
