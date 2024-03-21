#ifndef _LIST_H
#define _LIST_H

#include "../utils/failable.h"
#include "iterator.h"
#include "contained_item_info.h"


typedef struct list list;
extern contained_item_info *list_item_info;

list *new_list(contained_item_info *item_info);
list *list_of(contained_item_info *item_info, int items, ...);
contained_item_info *list_contained_item(list *l);

bool list_empty(list *l);
int   list_length(list *l);
void  list_add(list *l, void *item);
void *list_get(list *l, int index);
void  list_set(list *l, int index, void *item);
void list_insert(list *l, int index, void *item);
void list_remove(list *l, int index);

iterator *list_iterator(list *l);

bool lists_are_equal(list *a, list *b);
const void list_describe(list *l, const char *separator, str *str);

void list_free(list *l);

#define for_list(list_var, iter_var, item_type, item_var)  \
    iterator *iter_var = list_iterator(list_var); \
    for_iterator(iter_var, item_type, item_var)


STRONGLY_TYPED_FAILABLE_PTR_DECLARATION(list);
#define failed_list(inner, fmt, ...)  __failed_list(inner, __func__, __FILE__, __LINE__, fmt, ##__VA_ARGS__)


#endif
