#ifndef _DICT_H
#define _DICT_H

#include <stdbool.h>
#include "failable.h"
#include "value.h"

typedef struct dict dict;

dict *new_dict(int capacity);

void   dict_set(dict *d, const char *key, value *v);
bool   dict_has(dict *d, const char *key);
value *dict_get(dict *d, const char *key);

STRONGLY_TYPED_FAILABLE_DECLARATION(dict);

#endif
