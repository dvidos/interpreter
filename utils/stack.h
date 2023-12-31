#ifndef _STACK_H
#define _STACK_H

#include <stdbool.h>

typedef struct stack stack;

stack *new_stack();

bool  stack_empty(stack *s);
void  stack_push(stack *s, void *item);
void *stack_peek(stack *s);
void *stack_pop(stack *s);


#endif
