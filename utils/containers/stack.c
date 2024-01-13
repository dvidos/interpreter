#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "containable.h"
#include "../strbld.h"
#include "stack.h"

typedef struct stack_entry {
    void *item;
    struct stack_entry *next;
} stack_entry;

typedef struct stack {
    int length;
    stack_entry *head;
    contained_item_info *contained_item_info;
} stack;


stack *new_stack(contained_item_info *contained_item_info) {
    stack *s = malloc(sizeof(stack));
    s->length = 0;
    s->head = NULL;
    s->contained_item_info = contained_item_info;
    return s;
}

int stack_length(stack *s) {
    return s->length;
}

bool stack_empty(stack *s) {
    return s->length == 0;
}

void stack_push(stack *s, void *item) {
    stack_entry *entry = malloc(sizeof(stack_entry));
    entry->item = item;
    entry->next = s->head;
    s->head = entry;
    s->length += 1;
}

void *stack_peek(stack *s) {
    return s->head == NULL ? NULL : s->head->item;
}

void *stack_pop(stack *s) {
    if (s->head == NULL)
        return NULL;
    
    void *item = s->head->item;
    s->head = s->head->next;
    s->length -= 1;
    return item;
}


typedef struct stack_iterator_private_data {
    stack *stack;
    stack_entry *curr_entry;
} stack_iterator_private_data;
static void *stack_iterator_reset(iterator *it) {
    stack_iterator_private_data *pd = (stack_iterator_private_data *)it->private_data;
    pd->curr_entry = pd->stack->head; // can be NULL if stack is empty
    return pd->curr_entry == NULL ? NULL : pd->curr_entry->item;
}
static bool stack_iterator_valid(iterator *it) {
    stack_iterator_private_data *pd = (stack_iterator_private_data *)it->private_data;
    return pd->curr_entry != NULL;
}
static void *stack_iterator_next(iterator *it) {
    stack_iterator_private_data *pd = (stack_iterator_private_data *)it->private_data;
    if (pd->curr_entry != NULL)
        pd->curr_entry = pd->curr_entry->next;
    return pd->curr_entry == NULL ? NULL : pd->curr_entry->item;
}
static void *stack_iterator_curr(iterator *it) {
    stack_iterator_private_data *pd = (stack_iterator_private_data *)it->private_data;
    return pd->curr_entry == NULL ? NULL : pd->curr_entry->item;
}
static void *stack_iterator_peek(iterator *it) {
    stack_iterator_private_data *pd = (stack_iterator_private_data *)it->private_data;
    if (pd->curr_entry == NULL || pd->curr_entry->next == NULL)
        return NULL;
    return pd->curr_entry->next->item;
}
iterator *stack_iterator(stack *s) {
    stack_iterator_private_data *pd = malloc(sizeof(stack_iterator_private_data));
    pd->stack = s;
    pd->curr_entry = NULL;
    iterator *it = malloc(sizeof(iterator));
    it->reset = stack_iterator_reset;
    it->valid = stack_iterator_valid;
    it->next = stack_iterator_next;
    it->curr = stack_iterator_curr;
    it->peek = stack_iterator_peek;
    it->private_data = pd;
    return it;
}



const char *stack_to_string(stack *s, const char *separator) {
    strbld *sb = new_strbld();
    stack_entry *e = s->head;
    while (e != NULL) {
        if (e != s->head)
            strbld_cat(sb, separator);
        
        if (is_containable_instance(e->item))
            strbld_cat(sb, containable_to_string(e->item));
        else
            strbld_catf(sb, "@0x%p", e->item);
        
        e = e->next;
    }
    return strbld_charptr(sb);
}
