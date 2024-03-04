#ifndef _QUEUE_H
#define _QUEUE_H

#include <stdbool.h>
#include "iterator.h"

typedef struct queue queue;
extern class *queue_class;

queue *new_queue(class *item_class);

int   queue_length(queue *s);
bool  queue_empty(queue *s);
void  queue_put(queue *s, void *item);
void *queue_peek(queue *s);
void *queue_get(queue *s);
iterator *queue_iterator(queue *s);
const void queue_describe(queue *s, const char *separator, str_builder *sb);

#define for_queue(list_var, iter_var, item_type, item_var)  \
    iterator *iter_var = queue_iterator(list_var); \
    for_iterator(iter_var, item_type, item_var)



#endif
