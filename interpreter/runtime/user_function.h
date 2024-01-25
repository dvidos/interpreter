#ifndef _USER_FUNCTION_H
#define _USER_FUNCTION_H

#include "exec_context.h"

struct user_func {
    list *arg_names;
    list *statements;
};

failable_variant execute_user_function(
    user_func *f,
    list *positioned_args,
    dict *named_args,
    symbol_table *parent
);


#endif
