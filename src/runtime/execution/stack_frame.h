#ifndef _STACK_FRAME_H
#define _STACK_FRAME_H

#include <stdbool.h>
#include "../../runtime/variants/_module.h"
#include "../../containers/_module.h"
#include "stack_frame.h"

typedef struct statement statement;
typedef struct expression expression;

typedef struct stack_frame stack_frame;
extern item_info *stack_frame_item_info;

struct stack_frame {
    item_info *item_info;
    const char *func_name;
    statement *func_stmt;
    expression *func_expr;
    origin *call_origin;
    variant_type *method_owning_class; // if curr function is a method.
    dict *symbols;
};

stack_frame *new_stack_frame(const char *func_name, origin *call_origin);
void stack_frame_initialization(stack_frame *f, list *arg_names, list *arg_values, variant *this_value);

variant *stack_frame_resolve_symbol(stack_frame *f, const char *name);
bool stack_frame_symbol_exists(stack_frame *f, const char *name);
failable stack_frame_register_symbol(stack_frame *f, const char *name, variant *v);
failable stack_frame_update_symbol(stack_frame *f, const char *name, variant *v);
failable stack_frame_unregister_symbol(stack_frame *f, const char *name);
bool stack_frame_is_method_owned_by(stack_frame *f, variant_type *class_type);

const void stack_frame_describe(stack_frame *f, str_builder *sb);
bool stack_frames_are_equal(stack_frame *a, stack_frame *b);

extern item_info *stack_frame_item_info;




#endif
