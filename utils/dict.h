#ifndef _DICT_H
#define _DICT_H

#include <stdbool.h>
#include "failable.h"

typedef struct dict dict;

dict *new_dict(int capacity);

void  dict_set(dict *d, const char *key, void *item);
bool  dict_has(dict *d, const char *key);
void *dict_get(dict *d, const char *key);
int dict_count(dict *d);
bool dict_is_empty(dict *d);

bool dicts_are_equal(dict *a, dict *b);
const char *dict_to_string(dict *l, const char *key_value_separator, const char *entries_separator);


STRONGLY_TYPED_FAILABLE_DECLARATION(dict);

#endif
