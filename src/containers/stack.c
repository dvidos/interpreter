#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "../utils/item_info.h"
#include "../utils/str_builder.h"
#include "stack.h"

typedef struct stack_entry {
    void *item;
    struct stack_entry *next;
} stack_entry;

typedef struct stack {
    int length;
    stack_entry *head;
    item_info *item_info;
} stack;


stack *new_stack(item_info *item_info) {
    stack *s = malloc(sizeof(stack));
    s->item_info = stack_class;
    s->length = 0;
    s->head = NULL;
    s->item_info = item_info;
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



void stack_describe(stack *s, const char *separator, str_builder *sb) {
    stack_entry *e = s->head;
    while (e != NULL) {
        if (e != s->head)
            str_builder_add(sb, separator);
        
        if (s->item_info != NULL && s->item_info->describe != NULL)
            s->item_info->describe(e->item, sb);
        else
            str_builder_addf(sb, "@0x%p", e->item);
        
        e = e->next;
    }
}

static void stack_describe_default(stack *s, const char *separator, str_builder *sb) {
    stack_describe(s, ", ", sb);
}

item_info *stack_class = &(item_info){
    .item_info_magic = ITEM_INFO_MAGIC,
    .type_name = "stack",
    .describe = (describe_item_func)stack_describe_default,
    .are_equal = NULL
};
