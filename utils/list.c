#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "list.h"

typedef struct list_entry {
    void *item;
    struct list_entry *next;
} list_entry;

typedef struct list {
    int length;
    list_entry *head;
    list_entry *tail;
} list;


list *new_list() {
    list *l = malloc(sizeof(list));
    l->length = 0;
    l->head = NULL;
    l->tail = NULL;
}

list *list_of(int items, ...) {
    list *l = new_list();
    va_list args;
    va_start(args, items);
    while (items-- > 0)
        list_add(l, va_arg(args, void *));
    va_end(args);
    return l;
}

int list_length(list *l) {
    return l->length;
}

void list_add(list *l, void *item) {
    list_entry *entry = malloc(sizeof(list_entry));
    entry->item = item;
    entry->next = NULL;

    if (l->length == 0) {
        l->head = entry;
        l->tail = entry;
        l->length = 1;
    } else {
        l->tail->next = entry;
        l->tail = entry;
        l->length += 1;
    }
}

void *list_get(list *l, int index) {
    list_entry *e = l->head;
    while (index-- > 0 && e != NULL)
        e = e->next;
    
    return e == NULL ? NULL : e->item;
}

iterator *list_iterator(list *l) {
    return (iterator *)l->head;
}

bool iterator_valid(iterator *it) {
    return (it != NULL);
}

iterator *iterator_next(iterator *it) {
    list_entry *e = (list_entry *)it;
    if (e != NULL)
        e = e->next;
    return (iterator *)e;
}

void *iterator_current(iterator *it) {
    list_entry *e = (list_entry *)it;
    return e == NULL ? NULL : e->item;
}

STRONGLY_TYPED_FAILABLE_IMPLEMENTATION(list);


