#ifndef _STACK_FRAME_H
#define _STACK_FRAME_H

#include <stdbool.h>
#include "../containers/_module.h"
#include "variant.h"
#include "stack_frame.h"

typedef struct stack_frame stack_frame;

stack_frame *new_stack_frame(const char *func_name);
void stack_frame_initialization(stack_frame *f, list *arg_names, list *arg_values, dict *named_values, variant *this_value);

variant *stack_frame_resolve_symbol(stack_frame *f, const char *name);
bool stack_frame_symbol_exists(stack_frame *f, const char *name);
failable stack_frame_register_symbol(stack_frame *f, const char *name, variant *v);
failable stack_frame_update_symbol(stack_frame *f, const char *name, variant *v);


const void stack_frame_describe(stack_frame *f, str_builder *sb);
bool stack_frames_are_equal(stack_frame *a, stack_frame *b);

extern contained_item *containing_stack_frames;




#endif
