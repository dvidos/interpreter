#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "contained_item.h"
#include "../strbld.h"
#include "list.h"

typedef struct list_entry {
    void *item;
    struct list_entry *next;
} list_entry;

typedef struct list {
    int length;
    list_entry *head;
    list_entry *tail;
    contained_item *contained_item;
} list;


list *new_list(contained_item *contained_item) {
    list *l = malloc(sizeof(list));
    l->length = 0;
    l->head = NULL;
    l->tail = NULL;
    l->contained_item = contained_item;
    return l;
}

list *list_of(contained_item *contained_item, int items_count, ...) {
    list *l = new_list(contained_item);
    va_list args;
    va_start(args, items_count);
    while (items_count-- > 0)
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
    
    if (!contained_item_info_are_equal(a->contained_item, b->contained_item))
        return false;
    if (a->length != b->length)
        return false;

    // compare items, if they are values, use variants_are_equal()
    struct list_entry *entry_a = a->head;
    struct list_entry *entry_b = b->head;
    while (entry_a != NULL && entry_b != NULL) {
        bool equal;
        if (a->contained_item != NULL && a->contained_item->are_equal != NULL)
            equal = a->contained_item->are_equal(entry_a->item, entry_b->item);
        else
            equal = entry_a->item == entry_b->item;
        
        if (!equal)
            return false;
        entry_a = entry_a->next;
        entry_b = entry_b->next;
    }

    return true;
}

const char *list_to_string(list *l, const char *separator) {
    strbld *sb = new_strbld();
    list_entry *e = l->head;
    while (e != NULL) {
        if (e != l->head)
            strbld_cat(sb, separator);
        
        if (l->contained_item != NULL && l->contained_item->to_string != NULL)
            strbld_cat(sb, l->contained_item->to_string(e->item));
        else
            strbld_catf(sb, "@0x%p", e->item);
        
        e = e->next;
    }
    return strbld_charptr(sb);
}

STRONGLY_TYPED_FAILABLE_IMPLEMENTATION(list);
