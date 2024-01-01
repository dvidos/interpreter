#ifndef _LIST_H
#define _LIST_H

#include "failable.h"
#include "sequential.h"


typedef struct list list;

list *new_list();
list *list_of(int items, ...);

int   list_length(list *l);
void  list_add(list *l, void *item);
void *list_get(list *l, int index);
sequential *list_sequential(list *l);



STRONGLY_TYPED_FAILABLE_DECLARATION(list);

#endif
