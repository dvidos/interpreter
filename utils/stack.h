#ifndef _STACK_H
#define _STACK_H

#include <stdbool.h>
#include "sequential.h"

typedef struct stack stack;

stack *new_stack();

int   stack_length(stack *s);
bool  stack_empty(stack *s);
void  stack_push(stack *s, void *item);
void *stack_peek(stack *s);
void *stack_pop(stack *s);
sequential *stack_sequential(stack *s);
const char *stack_to_string(stack *s, const char *separator);

#endif
