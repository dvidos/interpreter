#ifndef _LIST_H
#define _LIST_H

#include "failable.h"

typedef struct list list;

list *new_list();

int   list_length(list *l);
void  list_add(list *l, void *item);
void *list_get(list *l, int index);

STRONGLY_TYPED_FAILABLE_DECLARATION(list);

#endif
