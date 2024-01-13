#ifndef _LIST_H
#define _LIST_H

#include "../failable.h"
#include "../iterator.h"
#include "containable.h"


typedef struct list list;

list *new_list(contained_item_info *contained_item_info);
list *list_of(contained_item_info *contained_item_info, int items, ...);

int   list_length(list *l);
void  list_add(list *l, void *item);
void *list_get(list *l, int index);
void  list_set(list *l, int index, void *item);
iterator *list_iterator(list *l);

bool lists_are_equal(list *a, list *b);
const char *list_to_string(list *l, const char *separator);


#define for_list(list_var, iter_var, item_type, item_var)  \
    iterator *iter_var = list_iterator(list_var); \
    for_iterator(iter_var, item_type, item_var)


STRONGLY_TYPED_FAILABLE_DECLARATION(list);

#endif
