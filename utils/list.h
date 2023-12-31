#ifndef _LIST_H
#define _LIST_H

#include "failable.h"

typedef struct list list;
typedef struct iterator iterator;

list *new_list();

int   list_length(list *l);
void  list_add(list *l, void *item);
void *list_get(list *l, int index);

iterator *list_iterator(list *l);
bool      iterator_valid(iterator *it);
iterator *iterator_next(iterator *it);
void     *iterator_current(iterator *it);



STRONGLY_TYPED_FAILABLE_DECLARATION(list);

#endif
