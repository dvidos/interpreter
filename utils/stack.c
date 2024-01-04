#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "containable.h"
#include "strbuff.h"
#include "stack.h"

typedef struct stack_entry {
    void *item;
    struct stack_entry *next;
} stack_entry;

typedef struct stack {
    int length;
    stack_entry *head;
} stack;


stack *new_stack() {
    stack *s = malloc(sizeof(stack));
    s->length = 0;
    s->head = NULL;
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

sequential *stack_sequential(stack *s) {
    return (sequential *)s->head;
}

const char *stack_to_string(stack *s, const char *separator) {
    strbuff *sb = new_strbuff();
    stack_entry *e = s->head;
    while (e != NULL) {
        if (e != s->head)
            strbuff_cat(sb, separator);
        
        if (is_containable_instance(e->item))
            strbuff_cat(sb, containable_to_string(e->item));
        else
            strbuff_catf(sb, "@0x%p", e->item);
        
        e = e->next;
    }
    return strbuff_charptr(sb);
}
