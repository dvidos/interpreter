#ifndef _LIST_H
#define _LIST_H

#include "failable.h"
#include "sequential.h"
#include "iterator.h"


typedef struct list list;

list *new_list();
list *list_of(int items, ...);

int   list_length(list *l);
void  list_add(list *l, void *item);
void *list_get(list *l, int index);
void  list_set(list *l, int index, void *item);
sequential *list_sequential(list *l);
iterator *list_iterator(list *l);

bool lists_are_equal(list *a, list *b);
const char *list_to_string(list *l, const char *separator);


STRONGLY_TYPED_FAILABLE_DECLARATION(list);

#endif
