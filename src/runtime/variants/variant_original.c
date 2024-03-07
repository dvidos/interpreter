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
    VT_LIST,
    VT_DICT,
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

item_info *variant_class = &(item_info){
    .item_info_magic = ITEM_INFO_MAGIC,
    .type_name = "variant",
    .are_equal = (items_equal_func)variants_are_equal,
    .describe = (describe_item_func)variant_describe
};

// variant *new_null_variant() {
//     variant_original *v = malloc(sizeof(variant_original));
//     memset(v, 0, sizeof(variant_original));
//     v->class = variant_class;
//     v->enum_type = VT_NULL;
//     return (variant *)v;
// }

// variant *new_bool_variant(bool b) {
//     variant_original *v = (variant_original *)new_null_variant();
//     v->enum_type = VT_BOOL;
//     v->per_type.bool_ = b;
//     return (variant *)v;
// }

// variant *new_int_variant(int i) {
//     variant_original *v = (variant_original *)new_null_variant();
//     v->enum_type = VT_INT;
//     v->per_type.int_ = i;
//     return (variant *)v;
// }

// variant *new_float_variant(float f) {
//     variant_original *v = (variant_original *)new_null_variant();
//     v->enum_type = VT_FLOAT;
//     v->per_type.float_ = f;
//     return (variant *)v;
// }

// variant *new_str_variant(const char *p) {
//     variant_original *v = (variant_original *)new_null_variant();
//     v->enum_type = VT_STR;
//     v->per_type.s.len = strlen(p);
//     v->per_type.s.ptr = malloc(v->per_type.s.len + 1);
//     strcpy(v->per_type.s.ptr, p);
//     return (variant *)v;
// }

variant *new_list_variant(list *l) {
    variant_original *v = malloc(sizeof(variant_original));
    memset(v, 0, sizeof(variant_original));
    v->class = variant_class;
    v->enum_type = VT_LIST;
    v->per_type.list_ = l;
    return (variant *)v;
}

variant *new_dict_variant(dict *d) {
    variant_original *v = malloc(sizeof(variant_original));
    memset(v, 0, sizeof(variant_original));
    v->class = variant_class;
    v->enum_type = VT_DICT;
    v->per_type.dict_ = d;
    return (variant *)v;
}

variant *new_callable_variant(callable *c) {
    variant_original *v = malloc(sizeof(variant_original));
    memset(v, 0, sizeof(variant_original));
    v->class = variant_class;
    v->enum_type = VT_CALLABLE;
    v->per_type.callable_ = c;
    return (variant *)v;
}

// variant *new_exception_variant(const char *script_filename, int script_line, int script_column, variant *inner, const char *fmt, ...) {
//     char buffer[256];
//     va_list args;
//     va_start(args, fmt);
//     vsnprintf(buffer, sizeof(buffer), fmt, args);
//     va_end(args);

//     char *msg = malloc(strlen(buffer) + 1);
//     strcpy(msg, buffer);

//     variant_original *v = malloc(sizeof(variant_original));
//     memset(v, 0, sizeof(variant_original));
//     v->class = variant_class;
//     v->enum_type = VT_EXCEPTION;
//     v->per_type.exception.msg = msg;
//     v->per_type.exception.script_filename = script_filename;
//     v->per_type.exception.script_line = script_line;
//     v->per_type.exception.script_column = script_column;
//     v->per_type.exception.inner = (variant_original *)inner;
//     return (variant *)v;
// }

bool variant_is_null(variant *v) {
    return variant_is(v, void_type);
    //return ((variant_original *)v)->enum_type == VT_NULL;
}

bool variant_is_bool(variant *v) {
    return variant_is(v, bool_type);
    // return ((variant_original *)v)->enum_type == VT_BOOL;
}

bool variant_is_int(variant *v) {
    return variant_is(v, int_type);
    // return ((variant_original *)v)->enum_type == VT_INT;
}

bool variant_is_float(variant *v) {
    return variant_is(v, float_type);
    //return ((variant_original *)v)->enum_type == VT_FLOAT;
}

bool variant_is_str(variant *v) {
    return variant_is(v, str_type);
    // return ((variant_original *)v)->enum_type == VT_STR;
}

bool variant_is_list(variant *v) {
    return ((variant_original *)v)->enum_type == VT_LIST;
}

bool variant_is_dict(variant *v) {
    return ((variant_original *)v)->enum_type == VT_DICT;
}

bool variant_is_callable(variant *v) {
    return ((variant_original *)v)->enum_type == VT_CALLABLE;
}

bool variant_is_exception(variant *v) {
    return variant_is(v, exception_type);
    //return ((variant_original *)v)->enum_type == VT_EXCEPTION;
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
    }

    variant_original *o = (variant_original *)v;
    switch (o->enum_type) {
        // case VT_NULL:
        //     return false;
        // case VT_BOOL:
        //     return o->per_type.bool_;
        // case VT_INT:
        //     return o->per_type.int_ != 0;
        // case VT_FLOAT:
        //     return o->per_type.float_ != 0.0;
        // case VT_STR:
        //     return (
        //         strcmp(o->per_type.s.ptr, "true") == 0 || 
        //         strcmp(o->per_type.s.ptr, "1") == 0
        //     );
        case VT_LIST:
            return o->per_type.list_ != NULL && list_length(o->per_type.list_) > 0;
        case VT_DICT:
            return o->per_type.dict_ != NULL && dict_count(o->per_type.dict_) > 0;
        case VT_CALLABLE:
            return false;
        default:
            return false;
    }
}

int variant_as_int(variant *v) {
    if (variant_is(v, void_type)) {
        return 0;
    } else if (variant_is(v, str_type)) {
        return atoi(str_variant_as_str(v));
    } else if (variant_is(v, int_type)) {
        return int_variant_as_int(v);
    } else if (variant_is(v, bool_type)) {
        return bool_variant_as_bool(v) ? 1 : 0;
    } else if (variant_is(v, float_type)) {
        return (int)float_variant_as_float(v);
    }

    variant_original *o = (variant_original *)v;
    switch (o->enum_type) {
        // case VT_NULL:
        //     return 0;
        // case VT_BOOL:
        //     return o->per_type.bool_ ? 1 : 0;
        // case VT_INT:
        //     return o->per_type.int_;
        // case VT_FLOAT:
        //     return (int)o->per_type.float_;
        // case VT_STR:
        //     return atoi(o->per_type.s.ptr);
        case VT_LIST:
            return o->per_type.list_ == NULL ? 0 : list_length(o->per_type.list_);
        case VT_DICT:
            return o->per_type.list_ == NULL ? 0 : dict_count(o->per_type.dict_);
        case VT_CALLABLE:
            return 0;
        default:
            return 0;
    }
}

float variant_as_float(variant *v) {
    if (variant_is(v, void_type)) {
        return 0.0;
    } else if (variant_is(v, str_type)) {
        return atof(str_variant_as_str(v));
    } else if (variant_is(v, int_type)) {
        return (float)int_variant_as_int(v);
    } else if (variant_is(v, bool_type)) {
        return bool_variant_as_bool(v) ? 1.0 : 0.0;
    } else if (variant_is(v, float_type)) {
        return float_variant_as_float(v);
    }

    variant_original *o = (variant_original *)v;
    switch (o->enum_type) {
        // case VT_NULL:
        //     return 0.0;
        // case VT_BOOL:
        //     return o->per_type.bool_ ? 1.0 : 0.0;
        // case VT_INT:
        //     return (float)o->per_type.int_;
        // case VT_FLOAT:
        //     return o->per_type.float_;
        // case VT_STR:
        //     return atof(o->per_type.s.ptr);
        case VT_LIST:
            return o->per_type.list_ == NULL ? 0.0 : (float)list_length(o->per_type.list_);
        case VT_DICT:
            return o->per_type.list_ == NULL ? 0.0 : (float)dict_count(o->per_type.dict_);
        case VT_CALLABLE:
            return 0.0;
        default:
            return 0.0;
    }
}

const char *variant_as_str(variant *v) {
    // see if this this the new variants
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
    }

    // else, keep compatibility with the old variants
    variant_original *o = (variant_original *)v;
    switch (o->enum_type) {
        // case VT_NULL:
        //     return "(null)";
        // case VT_BOOL:
        //     return o->per_type.bool_ ? "true" : "false";
        // case VT_INT:
        //     if (o->str_repr == NULL) {
        //         o->str_repr = malloc(64);
        //         memset((char *)o->str_repr, 0, 64);
        //         snprintf((char *)o->str_repr, 64, "%d", o->per_type.int_);
        //     }
        //     return o->str_repr;
        // case VT_FLOAT:
        //     if (o->str_repr == NULL) {
        //         o->str_repr = malloc(64);
        //         memset((char *)o->str_repr, 0, 64);
        //         snprintf((char *)o->str_repr, 64, "%f", o->per_type.float_);
        //     }
        //     return o->str_repr;
        // case VT_STR:
        //     return o->per_type.s.ptr;
        case VT_LIST:
            if (o->str_repr == NULL) {
                if (o->per_type.list_ != NULL) {
                    str_builder *sb = new_str_builder();
                    list_describe(o->per_type.list_, ", ", sb);
                    o->str_repr = strdup(str_builder_charptr(sb));
                    str_builder_free(sb);
                }
            }
            return o->str_repr;
        case VT_DICT:
            if (o->str_repr == NULL) {
                if (o->per_type.dict_ != NULL) {
                    str_builder *sb = new_str_builder();
                    dict_describe(o->per_type.dict_, ": ", ", ", sb);
                    o->str_repr = strdup(str_builder_charptr(sb));
                    str_builder_free(sb);
                }
            }
            return o->str_repr;
        case VT_CALLABLE:
            return callable_name(o->per_type.callable_);
        // case VT_EXCEPTION:
        //     if (o->str_repr == NULL) {
        //         str_builder *sb = new_str_builder();
        //         str_builder_addf(sb, "%s, at %s:%d:%d",
        //             o->per_type.exception.msg,
        //             o->per_type.exception.script_filename,
        //             o->per_type.exception.script_line,
        //             o->per_type.exception.script_column);
        //         if (o->per_type.exception.inner != NULL)
        //             str_builder_addf(sb, "\n\t%s", variant_as_str((variant *)o->per_type.exception.inner));
        //         o->str_repr = malloc(strlen(str_builder_charptr(sb)) + 1);
        //         strcpy((char *)o->str_repr, str_builder_charptr(sb));
        //         str_builder_free(sb);
        //     }
        //     return o->str_repr;

        //     return o->per_type.exception.msg;
        default:
            return NULL;
    }
}

list *variant_as_list(variant *v) {
    variant_original *o = (variant_original *)v;
    switch (o->enum_type) {
        // case VT_NULL:
        //     return NULL;
        // case VT_BOOL:
        //     return list_of(variant_class, 1, v);
        // case VT_INT:
        //     return list_of(variant_class, 1, v);
        // case VT_FLOAT:
        //     return list_of(variant_class, 1, v);
        // case VT_STR:
        //     return list_of(variant_class, 1, v);
        case VT_LIST:
            return o->per_type.list_;
        case VT_DICT:
            return dict_get_values(o->per_type.dict_);
        case VT_CALLABLE:
            return NULL;
        default:
            return NULL;
    }
}

dict *variant_as_dict(variant *v) {
    variant_original *o = (variant_original *)v;
    switch (o->enum_type) {
        // case VT_NULL:
        //     return NULL;
        // case VT_BOOL:
        //     return new_dict(variant_class);
        // case VT_INT:
        //     return new_dict(variant_class);
        // case VT_FLOAT:
        //     return new_dict(variant_class);
        // case VT_STR:
        //     return new_dict(variant_class);
        case VT_LIST:
            return new_dict(variant_class);
        case VT_DICT:
            return o->per_type.dict_;
        case VT_CALLABLE:
            return NULL;
        default:
            return NULL;
    }
}

callable *variant_as_callable(variant *v) {
    variant_original *o = (variant_original *)v;
    switch (o->enum_type) {
        // case VT_NULL:
        // case VT_BOOL:
        // case VT_INT:
        // case VT_FLOAT:
        // case VT_STR:
        case VT_LIST:
        case VT_DICT:
            return NULL;
        case VT_CALLABLE:
            return o->per_type.callable_;
        default:
            return NULL;
    }
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
    }

    variant_original *va = (variant_original *)a;
    variant_original *vb = (variant_original *)b;

    // if we allow ("4"==4) we have the pitfalls of javascript
    if (va->enum_type != vb->enum_type)
        return false;
    
    switch (va->enum_type) {
        // case VT_NULL:
        //     return true;
        // case VT_BOOL:
        //     return va->per_type.bool_ == vb->per_type.bool_;
        // case VT_INT:
        //     return va->per_type.int_ == vb->per_type.int_;
        // case VT_FLOAT:
        //     return va->per_type.float_ == vb->per_type.float_;
        // case VT_STR:
        //     if (va->per_type.s.len != vb->per_type.s.len)
        //         return false;
        //     return memcmp(va->per_type.s.ptr, vb->per_type.s.ptr, va->per_type.s.len) == 0;
        case VT_LIST:
            return lists_are_equal(va->per_type.list_, vb->per_type.list_);
        case VT_DICT:
            return dicts_are_equal(va->per_type.dict_, vb->per_type.dict_);
        case VT_CALLABLE:
            return callables_are_equal(va->per_type.callable_, vb->per_type.callable_);
        // case VT_EXCEPTION:
        //     return strcmp(va->per_type.exception.msg, vb->per_type.exception.msg) == 0;
    }

    // we shouldn't get here...
    return false;
}

const void variant_describe(variant *v, str_builder *sb) {
    str_builder_add(sb, variant_as_str(v));
}


