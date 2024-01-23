#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "contained_item.h"
#include "../str_builder.h"
#include "queue.h"

typedef struct queue_entry {
    void *item;
    struct queue_entry *next; // pointing from exit to entrance
} queue_entry;

typedef struct queue {
    int length;
    queue_entry *entrance;
    queue_entry *exit;
    contained_item *contained_item;
} queue;

queue *new_queue(contained_item *contained_item) {
    queue *q = malloc(sizeof(queue));
    q->length = 0;
    q->entrance = NULL;
    q->exit = NULL;
    q->contained_item = contained_item;
    return q;
}

int queue_length(queue *q) {
    return q->length;
}

bool queue_empty(queue *q) {
    return q->length == 0;
}

void queue_put(queue *q, void *item) {
    queue_entry *entry = malloc(sizeof(queue_entry));
    entry->item = item;
    entry->next = NULL;
    if (q->length == 0) {
        q->entrance = entry;
        q->exit = entry;
    } else {
        q->entrance->next = entry;
        q->entrance = entry;
    }
    q->length += 1;
}

void *queue_peek(queue *q) {
    return q->exit == NULL ? NULL : q->exit->item;
}

void *queue_get(queue *q) {
    if (q->exit == NULL)
        return NULL;
    
    void *item = q->exit->item;
    if (q->length == 1) {
        q->exit = NULL;
        q->entrance = NULL;
    } else {
        q->exit = q->exit->next;
    }
    q->length -= 1;
    return item;
}


typedef struct queue_iterator_private_data {
    queue *queue;
    queue_entry *curr_entry;
} queue_iterator_private_data;
static void *queue_iterator_reset(iterator *it) {
    queue_iterator_private_data *pd = (queue_iterator_private_data *)it->private_data;
    pd->curr_entry = pd->queue->exit; // can be NULL if queue is empty
    return pd->curr_entry == NULL ? NULL : pd->curr_entry->item;
}
static bool queue_iterator_valid(iterator *it) {
    queue_iterator_private_data *pd = (queue_iterator_private_data *)it->private_data;
    return pd->curr_entry != NULL;
}
static void *queue_iterator_next(iterator *it) {
    queue_iterator_private_data *pd = (queue_iterator_private_data *)it->private_data;
    if (pd->curr_entry != NULL)
        pd->curr_entry = pd->curr_entry->next;
    return pd->curr_entry == NULL ? NULL : pd->curr_entry->item;
}
static void *queue_iterator_curr(iterator *it) {
    queue_iterator_private_data *pd = (queue_iterator_private_data *)it->private_data;
    return pd->curr_entry == NULL ? NULL : pd->curr_entry->item;
}
static void *queue_iterator_peek(iterator *it) {
    queue_iterator_private_data *pd = (queue_iterator_private_data *)it->private_data;
    if (pd->curr_entry == NULL || pd->curr_entry->next == NULL)
        return NULL;
    return pd->curr_entry->next->item;
}
iterator *queue_iterator(queue *q) {
    queue_iterator_private_data *pd = malloc(sizeof(queue_iterator_private_data));
    pd->queue = q;
    pd->curr_entry = NULL;
    iterator *it = malloc(sizeof(iterator));
    it->reset = queue_iterator_reset;
    it->valid = queue_iterator_valid;
    it->next = queue_iterator_next;
    it->curr = queue_iterator_curr;
    it->peek = queue_iterator_peek;
    it->private_data = pd;
    return it;
}



const char *queue_to_string(queue *q, const char *separator) {
    str_builder *sb = new_str_builder();
    queue_entry *e = q->exit;
    while (e != NULL) {
        if (e != q->exit)
            str_builder_cat(sb, separator);
        
        if (q->contained_item != NULL && q->contained_item->to_string != NULL)
            str_builder_cat(sb, q->contained_item->to_string(e->item));
        else
            str_builder_catf(sb, "@0x%p", e->item);
        
        e = e->next;
    }
    return str_builder_charptr(sb);
}
