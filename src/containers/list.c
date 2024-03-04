#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "../utils/class.h"
#include "../utils/str_builder.h"
#include "list.h"

typedef struct list_entry {
    void *item;
    struct list_entry *next;
} list_entry;

typedef struct list {
    int length;
    list_entry *head;
    list_entry *tail;
    class *item_class;
} list;


list *new_list(class *item_class) {
    list *l = malloc(sizeof(list));
    l->length = 0;
    l->head = NULL;
    l->tail = NULL;
    l->item_class = item_class;
    return l;
}

list *list_of(class *item_class, int items_count, ...) {
    list *l = new_list(item_class);
    va_list args;
    va_start(args, items_count);
    while (items_count-- > 0)
        list_add(l, va_arg(args, void *));
    va_end(args);
    return l;
}

class *list_contained_item(list *l) {
    return l->item_class;
}

bool list_empty(list *l) {
    return l->length == 0;
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

void list_set(list *l, int index, void *item) {
    // make sure this position is indexable
    while (l->length < index)
        list_add(l, NULL);
        
    // walk and set
    list_entry *e = l->head;
    while (index-- > 0 && e != NULL)
        e = e->next;
    if (e != NULL)
        e->item = item;
}

void list_insert(list *l, int index, void *item) {
    list_entry *entry = malloc(sizeof(list_entry));
    entry->item = item;
    entry->next = NULL;

    if (index == 0) {
        entry->next = l->head;
        l->head = entry;
    } else {
        list_entry *prev = l->head;
        while (index-- > 1 && prev->next != NULL)
            prev = prev->next;
        entry->next = prev->next;
        prev->next = entry;
    }
    l->length++;
}

void list_remove(list *l, int index) {
    if (index == 0) {
        if (l->head != NULL) {
            l->head = l->head->next;
            l->length++;
        }
    } else {
        list_entry *prev = l->head;
        while (index-- > 1 && prev->next != NULL)
            prev = prev->next;
        if (prev != NULL && prev->next != NULL) {
            prev->next = prev->next->next;
            l->length--;
        }
    }
}






typedef struct list_iterator_private_data {
    list *list;
    list_entry *curr_entry;
} list_iterator_private_data;
static void *list_iterator_reset(iterator *it) {
    list_iterator_private_data *pd = (list_iterator_private_data *)it->private_data;
    pd->curr_entry = pd->list->head; // can be NULL if list is empty
    return pd->curr_entry == NULL ? NULL : pd->curr_entry->item;
}
static bool list_iterator_valid(iterator *it) {
    list_iterator_private_data *pd = (list_iterator_private_data *)it->private_data;
    return pd->curr_entry != NULL;
}
static void *list_iterator_next(iterator *it) {
    list_iterator_private_data *pd = (list_iterator_private_data *)it->private_data;
    if (pd->curr_entry != NULL)
        pd->curr_entry = pd->curr_entry->next;
    return pd->curr_entry == NULL ? NULL : pd->curr_entry->item;
}
static void *list_iterator_curr(iterator *it) {
    list_iterator_private_data *pd = (list_iterator_private_data *)it->private_data;
    return pd->curr_entry == NULL ? NULL : pd->curr_entry->item;
}
static void *list_iterator_peek(iterator *it) {
    list_iterator_private_data *pd = (list_iterator_private_data *)it->private_data;
    if (pd->curr_entry == NULL || pd->curr_entry->next == NULL)
        return NULL;
    return pd->curr_entry->next->item;
}
iterator *list_iterator(list *l) {
    list_iterator_private_data *pd = malloc(sizeof(list_iterator_private_data));
    pd->list = l;
    pd->curr_entry = NULL;
    iterator *it = malloc(sizeof(iterator));
    it->reset = list_iterator_reset;
    it->valid = list_iterator_valid;
    it->next = list_iterator_next;
    it->curr = list_iterator_curr;
    it->peek = list_iterator_peek;
    it->private_data = pd;
    return it;
}


bool lists_are_equal(list *a, list *b) {
    if (a == NULL && b == NULL)
        return true;
    if ((a == NULL && b != NULL) || (a != NULL && b == NULL))
        return false;
    if (a == b)
        return true;
    
    if (!classes_are_equal(a->item_class, b->item_class))
        return false;
    if (a->length != b->length)
        return false;

    // compare items
    struct list_entry *entry_a = a->head;
    struct list_entry *entry_b = b->head;
    while (entry_a != NULL && entry_b != NULL) {
        bool equal;
        if (a->item_class != NULL && a->item_class->are_equal != NULL)
            equal = a->item_class->are_equal(entry_a->item, entry_b->item);
        else
            equal = entry_a->item == entry_b->item;
        
        if (!equal)
            return false;
        entry_a = entry_a->next;
        entry_b = entry_b->next;
    }

    return true;
}

void list_describe(list *l, const char *separator, str_builder *sb) {
    list_entry *e = l->head;
    while (e != NULL) {
        if (e != l->head)
            str_builder_add(sb, separator);
        
        if (l->item_class != NULL && l->item_class->describe != NULL)
            l->item_class->describe(e->item, sb);
        else
            str_builder_addf(sb, "@0x%p", e->item);
        
        e = e->next;
    }
}

static void list_describe_default(list *l, str_builder *sb) {
    list_describe(l, ", ", sb);
}


class *list_class = &(class){
    .classdef_magic = CLASSDEF_MAGIC,
    .type_name = "list",
    .are_equal = (are_equal_func)lists_are_equal,
    .describe = (describe_func)list_describe_default,
};

STRONGLY_TYPED_FAILABLE_PTR_IMPLEMENTATION(list);
