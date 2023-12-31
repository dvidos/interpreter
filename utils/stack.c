#include <stdlib.h>
#include <string.h>
#include <stddef.h>
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
}

