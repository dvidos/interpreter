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
    return l;
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

sequential *list_sequential(list *l) {
    return (sequential *)l->head;
}

bool lists_are_equal(list *a, list *b) {
    if (a == NULL && b == NULL)
        return true;
    if ((a == NULL && b != NULL) || (a != NULL && b == NULL))
        return false;
    if (a == b)
        return true;
    
    if (a->length != b->length)
        return false;

    // compare items, if they are values, use values_are_same()
    for (int i = 0; i < a->length; i++) {
        // must find to get a "compare" or "equals" function.
    }

    return true;
}


STRONGLY_TYPED_FAILABLE_IMPLEMENTATION(list);


