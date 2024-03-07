#ifndef _DICT_H
#define _DICT_H

#include <stdbool.h>
#include "../utils/failable.h"
#include "iterator.h"
#include "../utils/item_info.h"
#include "list.h"

typedef struct dict dict;
extern item_info *dict_item_info;

dict *new_dict(item_info *item_info);
dict *dict_of(item_info *item_info, int pairs_count, ...);

void  dict_set(dict *d, const char *key, void *item);
bool  dict_has(dict *d, const char *key);
void *dict_get(dict *d, const char *key);
bool  dict_del(dict *d, const char *key);
int dict_count(dict *d);
bool dict_is_empty(dict *d);
iterator *dict_keys_iterator(dict *d);
list *dict_get_keys(dict *d);
list *dict_get_values(dict *d);

bool dicts_are_equal(dict *a, dict *b);
const void dict_describe(dict *l, const char *key_value_separator, const char *entries_separator, str_builder *sb);


#define for_dict(dict_var, iter_var, item_type, item_var)  \
    iterator *iter_var = dict_keys_iterator(dict_var); \
    for_iterator(iter_var, item_type, item_var)


STRONGLY_TYPED_FAILABLE_PTR_DECLARATION(dict);
#define failed_dict(inner, fmt, ...)  __failed_dict(inner, __func__, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

#endif
