#ifndef _STACK_H
#define _STACK_H

#include <stdbool.h>
#include "iterator.h"

typedef struct stack stack;
extern contained_item_info *stack_item_info;

stack *new_stack(contained_item_info *item_info);

int   stack_length(stack *s);
bool  stack_empty(stack *s);
void  stack_push(stack *s, void *item);
void *stack_peek(stack *s);
void *stack_pop(stack *s);
iterator *stack_iterator(stack *s);
const void stack_describe(stack *s, const char *separator, str_builder *sb);

#define for_stack(list_var, iter_var, item_type, item_var)  \
    iterator *iter_var = stack_iterator(list_var); \
    for_iterator(iter_var, item_type, item_var)



#endif
