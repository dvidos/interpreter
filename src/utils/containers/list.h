#ifndef _LIST_H
#define _LIST_H

#include "../failable.h"
#include "../iterator.h"
#include "../class.h"


typedef struct list list;
extern class *list_class;

list *new_list(class *item_class);
list *list_of(class *item_class, int items, ...);
class *list_contained_item(list *l);

bool list_empty(list *l);
int   list_length(list *l);
void  list_add(list *l, void *item);
void *list_get(list *l, int index);
void  list_set(list *l, int index, void *item);
void list_insert(list *l, int index, void *item);
void list_remove(list *l, int index);

iterator *list_iterator(list *l);

bool lists_are_equal(list *a, list *b);
const void list_describe(list *l, const char *separator, str_builder *sb);


#define for_list(list_var, iter_var, item_type, item_var)  \
    iterator *iter_var = list_iterator(list_var); \
    for_iterator(iter_var, item_type, item_var)


STRONGLY_TYPED_FAILABLE_PTR_DECLARATION(list);
#define failed_list(inner, fmt, ...)  __failed_list(inner, __func__, __FILE__, __LINE__, fmt, ##__VA_ARGS__)


#endif
