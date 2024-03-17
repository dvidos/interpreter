#include <stdlib.h>
#include "stack_frame.h"
#include "../../utils/str.h"


stack_frame *new_stack_frame(const char *func_name, const char *call_filename, int call_line_no, int call_column_no) {
    stack_frame *f = malloc(sizeof(stack_frame));
    f->item_info = stack_frame_item_info;
    f->func_name = func_name;
    f->call_filename = call_filename;
    f->call_line_no = call_line_no;
    f->call_column_no = call_column_no;
    f->symbols = new_dict(variant_item_info);
    return f;
}

void stack_frame_initialization(stack_frame *f, list *arg_names, list *arg_values, variant *this_obj) {

    if (arg_names != NULL && arg_values != NULL) {
        for (int i = 0; i < list_length(arg_names); i++) {
            stack_frame_register_symbol(f, list_get(arg_names, i), list_get(arg_values, i));
        }
    }

    if (this_obj != NULL)
        stack_frame_register_symbol(f, "this", this_obj);
}


variant *stack_frame_resolve_symbol(stack_frame *f, const char *name) {
    if (dict_has(f->symbols, name))
        return dict_get(f->symbols, name);
    return NULL;
}

bool stack_frame_symbol_exists(stack_frame *f, const char *name) {
    return dict_has(f->symbols, name);
}

failable stack_frame_register_symbol(stack_frame *f, const char *name, variant *v) {
    if (dict_has(f->symbols, name))
        return failed("Symbol %s already exists", name);
    dict_set(f->symbols, name, v);
    return ok();
}

failable stack_frame_update_symbol(stack_frame *f, const char *name, variant *v) {
    if (!dict_has(f->symbols, name))
        return failed("Symbol %s does not exist", name);
    dict_set(f->symbols, name, v);
    return ok();
}

failable stack_frame_unregister_symbol(stack_frame *f, const char *name) {
    if (!dict_has(f->symbols, name))
        return failed("Symbol %s does not exist", name);
    dict_del(f->symbols, name);
    return ok();
}



const void stack_frame_describe(stack_frame *f, str_builder *sb) {
    str_builder_add(sb, "stack_frame");
}

bool stack_frames_are_equal(stack_frame *a, stack_frame *b) {
    if (a == NULL && b == NULL) return true;
    if ((a == NULL && b != NULL) || (a != NULL && b == NULL)) return false;
    if (a == b) return true;

    if (!strs_are_equal(a->func_name, b->func_name))
        return false;
    if (!dicts_are_equal(a->symbols, b->symbols))
        return false;

    return true;
}

item_info *stack_frame_item_info = &(item_info) {
    .item_info_magic = ITEM_INFO_MAGIC,
    .type_name = "stack_frame",
    .describe = (describe_item_func)stack_frame_describe,
    .are_equal = (items_equal_func)stack_frames_are_equal
};


