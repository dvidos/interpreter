#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include "variant_original.h"
#include "../../utils/data_types/callable.h"

#include "int_variant.h"
#include "str_variant.h"
#include "bool_variant.h"
#include "float_variant.h"

typedef enum variant_enum_type {
    // VT_NULL,
    // VT_BOOL, 
    // VT_INT,
    // VT_FLOAT,
    // VT_STR,
    // VT_LIST,
    // VT_DICT,
    VT_CALLABLE,
    // VT_EXCEPTION,
} variant_enum_type;

typedef struct variant_original variant_original;

struct variant_original {
    BASE_VARIANT_FIRST_ATTRIBUTES;
    // then these:
    item_info *class;
    variant_enum_type enum_type;
    union {
        bool bool_;
        int int_;
        float float_;
        struct {
            char *ptr;
            int len;
        } s;
        list *list_;
        dict *dict_;
        callable *callable_;
        struct {
            const char *msg;
            const char *script_filename;
            int script_line;
            int script_column;
            variant_original *inner;
        } exception;
    } per_type;
    const char *str_repr;
};

bool variants_are_equal(variant *a, variant *b);
const void variant_describe(variant *v, str_builder *sb);

item_info *variant_item_info = &(item_info){
    .item_info_magic = ITEM_INFO_MAGIC,
    .type_name = "variant",
    .are_equal = (items_equal_func)variants_are_equal,
    .describe = (describe_item_func)variant_describe
};


bool variant_is_null(variant *v) {
    return variant_is(v, void_type);
}

bool variant_is_bool(variant *v) {
    return variant_is(v, bool_type);
}

bool variant_is_int(variant *v) {
    return variant_is(v, int_type);
}

bool variant_is_float(variant *v) {
    return variant_is(v, float_type);
}

bool variant_is_str(variant *v) {
    return variant_is(v, str_type);
}

bool variant_is_list(variant *v) {
    return variant_is(v, list_type);
}

bool variant_is_dict(variant *v) {
    return variant_is(v, dict_type);
}

bool variant_is_callable(variant *v) {
    return variant_is(v, callable_type);
}

bool variant_is_exception(variant *v) {
    return variant_is(v, exception_type);
}

bool variant_as_bool(variant *v) {
    if (variant_is(v, void_type)) {
        return false;
    } else if (variant_is(v, str_type)) {
        return strcmp(str_variant_as_str(v), "true") == 0 || 
               strcmp(str_variant_as_str(v), "1") == 0;
    } else if (variant_is(v, int_type)) {
        return int_variant_as_int(v) != 0;
    } else if (variant_is(v, bool_type)) {
        return bool_variant_as_bool(v);
    } else if (variant_is(v, float_type)) {
        return float_variant_as_float(v) != 0.0;
    } else if (variant_is(v, list_type)) {
        return !list_empty(list_variant_as_list(v));
    } else if (variant_is(v, dict_type)) {
        return !dict_is_empty(dict_variant_as_dict(v));
    }

    return false;
}

int variant_as_int(variant *v) {
    if (variant_is(v, str_type)) {
        return atoi(str_variant_as_str(v));
    } else if (variant_is(v, int_type)) {
        return int_variant_as_int(v);
    } else if (variant_is(v, bool_type)) {
        return bool_variant_as_bool(v) ? 1 : 0;
    } else if (variant_is(v, float_type)) {
        return (int)float_variant_as_float(v);
    }

    return 0;
}

float variant_as_float(variant *v) {
    if (variant_is(v, str_type)) {
        return atof(str_variant_as_str(v));
    } else if (variant_is(v, int_type)) {
        return (float)int_variant_as_int(v);
    } else if (variant_is(v, float_type)) {
        return float_variant_as_float(v);
    }

    return 0.0;
}

const char *variant_as_str(variant *v) {
    // we should not return the char here, we should return the str_variant,
    // so that caller can drop the reference, when they are done.
    if (variant_is(v, void_type)) {
        return "(void)";
    } else if (variant_is(v, str_type)) {
        return str_variant_as_str(v);
    } else if (variant_is(v, int_type)) {
        return str_variant_as_str(variant_to_string(v));
    } else if (variant_is(v, bool_type)) {
        return str_variant_as_str(variant_to_string(v));
    } else if (variant_is(v, float_type)) {
        return str_variant_as_str(variant_to_string(v));
    } else if (variant_is(v, list_type)) {
        return str_variant_as_str(variant_to_string(v));
    } else if (variant_is(v, dict_type)) {
        return str_variant_as_str(variant_to_string(v));
    }

    return NULL;
}

list *variant_as_list(variant *v) {
    if (variant_is(v, list_type)) {
        return list_variant_as_list(v);
    } else {
        return NULL;
    }
}

dict *variant_as_dict(variant *v) {
    if (variant_is(v, dict_type)) {
        return dict_variant_as_dict(v);
    } else {
        return NULL;
    }
}

callable *variant_as_callable(variant *v) {
    if (variant_is(v, callable_type))
        return callable_variant_as_callable(v);
    else
        return NULL;
}

bool variants_are_equal(variant *a, variant *b) {
    if (a == NULL && b == NULL)
        return true;
    if ((a == NULL && b != NULL) || (a != NULL && b == NULL))
        return false;
    if (a == b)
        return true;

    if (variant_is(a, void_type) && variant_is(b, void_type)) {
        return true;
    } else if (variant_is(a, str_type) && variant_is(b, str_type)) {
        return a->_type->equality_checker(a, b);
    } else if (variant_is(a, int_type) && variant_is(b, int_type)) {
        return a->_type->equality_checker(a, b);
    } else if (variant_is(a, bool_type) && variant_is(b, bool_type)) {
        return a->_type->equality_checker(a, b);
    } else if (variant_is(a, float_type) && variant_is(b, float_type)) {
        return a->_type->equality_checker(a, b);
    } else if (variant_is(a, list_type) && variant_is(b, list_type)) {
        return a->_type->equality_checker(a, b);
    } else if (variant_is(a, dict_type) && variant_is(b, dict_type)) {
        return a->_type->equality_checker(a, b);
    } else if (variant_is(a, callable_type) && variant_is(b, callable_type)) {
        return a->_type->equality_checker(a, b);
    }

    return false;
}

const void variant_describe(variant *v, str_builder *sb) {
    str_builder_add(sb, variant_as_str(v));
}
