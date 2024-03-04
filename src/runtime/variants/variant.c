#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include "variant.h"
#include "../../utils/data_types/callable.h"


typedef enum variant_enum_type {
    VT_NULL,
    VT_BOOL,
    VT_INT,
    VT_FLOAT,
    VT_STR,
    VT_LIST,
    VT_DICT,
    VT_CALLABLE,
    VT_EXCEPTION,
} variant_enum_type;

struct variant {
    class *class;
    variant_enum_type type;
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
            variant *inner;
        } exception;
    } per_type;
    const char *str_repr;
};

class *variant_class = &(class){
    .classdef_magic = CLASSDEF_MAGIC,
    .type_name = "variant",
    .are_equal = (are_equal_func)variants_are_equal,
    .describe = (describe_func)variant_describe
};

variant *new_null_variant() {
    variant *v = malloc(sizeof(variant));
    memset(v, 0, sizeof(variant));
    v->class = variant_class;
    v->type = VT_NULL;
    return v;
}

variant *new_bool_variant(bool b) {
    variant *v = new_null_variant();
    v->type = VT_BOOL;
    v->per_type.bool_ = b;
    return v;
}

variant *new_int_variant(int i) {
    variant *v = new_null_variant();
    v->type = VT_INT;
    v->per_type.int_ = i;
    return v;
}

variant *new_float_variant(float f) {
    variant *v = new_null_variant();
    v->type = VT_FLOAT;
    v->per_type.float_ = f;
    return v;
}

variant *new_str_variant(const char *p) {
    variant *v = new_null_variant();
    v->type = VT_STR;
    v->per_type.s.len = strlen(p);
    v->per_type.s.ptr = malloc(v->per_type.s.len + 1);
    strcpy(v->per_type.s.ptr, p);
    return v;
}

variant *new_list_variant(list *l) {
    variant *v = new_null_variant();
    v->type = VT_LIST;
    v->per_type.list_ = l;
    return v;
}

variant *new_dict_variant(dict *d) {
    variant *v = new_null_variant();
    v->type = VT_DICT;
    v->per_type.dict_ = d;
    return v;
}

variant *new_callable_variant(callable *c) {
    variant *v = new_null_variant();
    v->type = VT_CALLABLE;
    v->per_type.callable_ = c;
    return v;
}

variant *new_exception_variant(const char *script_filename, int script_line, int script_column, variant *inner, const char *fmt, ...) {
    char buffer[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    char *msg = malloc(strlen(buffer) + 1);
    strcpy(msg, buffer);

    variant *v = new_null_variant();
    v->type = VT_EXCEPTION;
    v->per_type.exception.msg = msg;
    v->per_type.exception.script_filename = script_filename;
    v->per_type.exception.script_line = script_line;
    v->per_type.exception.script_column = script_column;
    v->per_type.exception.inner = inner;
    return v;
}

bool variant_is_null(variant *v) {
    return v->type == VT_NULL;
}

bool variant_is_bool(variant *v) {
    return v->type == VT_BOOL;
}

bool variant_is_int(variant *v) {
    return v->type == VT_INT;
}

bool variant_is_float(variant *v) {
    return v->type == VT_FLOAT;
}

bool variant_is_str(variant *v) {
    return v->type == VT_STR;
}

bool variant_is_list(variant *v) {
    return v->type == VT_LIST;
}

bool variant_is_dict(variant *v) {
    return v->type == VT_DICT;
}

bool variant_is_callable(variant *v) {
    return v->type == VT_CALLABLE;
}

bool variant_is_exception(variant *v) {
    return v->type == VT_EXCEPTION;
}

bool variant_as_bool(variant *v) {
    switch (v->type) {
        case VT_NULL:
            return false;
        case VT_BOOL:
            return v->per_type.bool_;
        case VT_INT:
            return v->per_type.int_ != 0;
        case VT_FLOAT:
            return v->per_type.float_ != 0.0;
        case VT_STR:
            return (
                strcmp(v->per_type.s.ptr, "true") == 0 || 
                strcmp(v->per_type.s.ptr, "1") == 0
            );
        case VT_LIST:
            return v->per_type.list_ != NULL && list_length(v->per_type.list_) > 0;
        case VT_DICT:
            return v->per_type.dict_ != NULL && dict_count(v->per_type.dict_) > 0;
        case VT_CALLABLE:
            return false;
        default:
            return false;
    }
}

int variant_as_int(variant *v) {
    switch (v->type) {
        case VT_NULL:
            return 0;
        case VT_BOOL:
            return v->per_type.bool_ ? 1 : 0;
        case VT_INT:
            return v->per_type.int_;
        case VT_FLOAT:
            return (int)v->per_type.float_;
        case VT_STR:
            return atoi(v->per_type.s.ptr);
        case VT_LIST:
            return v->per_type.list_ == NULL ? 0 : list_length(v->per_type.list_);
        case VT_DICT:
            return v->per_type.list_ == NULL ? 0 : dict_count(v->per_type.dict_);
        case VT_CALLABLE:
            return 0;
        default:
            return 0;
    }
}

float variant_as_float(variant *v) {
    switch (v->type) {
        case VT_NULL:
            return 0.0;
        case VT_BOOL:
            return v->per_type.bool_ ? 1.0 : 0.0;
        case VT_INT:
            return (float)v->per_type.int_;
        case VT_FLOAT:
            return v->per_type.float_;
        case VT_STR:
            return atof(v->per_type.s.ptr);
        case VT_LIST:
            return v->per_type.list_ == NULL ? 0.0 : (float)list_length(v->per_type.list_);
        case VT_DICT:
            return v->per_type.list_ == NULL ? 0.0 : (float)dict_count(v->per_type.dict_);
        case VT_CALLABLE:
            return 0.0;
        default:
            return 0.0;
    }
}

const char *variant_as_str(variant *v) {
    switch (v->type) {
        case VT_NULL:
            return "(null)";
        case VT_BOOL:
            return v->per_type.bool_ ? "true" : "false";
        case VT_INT:
            if (v->str_repr == NULL) {
                v->str_repr = malloc(64);
                memset((char *)v->str_repr, 0, 64);
                snprintf((char *)v->str_repr, 64, "%d", v->per_type.int_);
            }
            return v->str_repr;
        case VT_FLOAT:
            if (v->str_repr == NULL) {
                v->str_repr = malloc(64);
                memset((char *)v->str_repr, 0, 64);
                snprintf((char *)v->str_repr, 64, "%f", v->per_type.float_);
            }
            return v->str_repr;
        case VT_STR:
            return v->per_type.s.ptr;
        case VT_LIST:
            if (v->str_repr == NULL) {
                if (v->per_type.list_ != NULL) {
                    str_builder *sb = new_str_builder();
                    list_describe(v->per_type.list_, ", ", sb);
                    v->str_repr = strdup(str_builder_charptr(sb));
                    str_builder_free(sb);
                }
            }
            return v->str_repr;
        case VT_DICT:
            if (v->str_repr == NULL) {
                if (v->per_type.dict_ != NULL) {
                    str_builder *sb = new_str_builder();
                    dict_describe(v->per_type.dict_, ": ", ", ", sb);
                    v->str_repr = strdup(str_builder_charptr(sb));
                    str_builder_free(sb);
                }
            }
            return v->str_repr;
        case VT_CALLABLE:
            return callable_name(v->per_type.callable_);
        case VT_EXCEPTION:
            if (v->str_repr == NULL) {
                str_builder *sb = new_str_builder();
                str_builder_addf(sb, "%s, at %s:%d:%d",
                    v->per_type.exception.msg,
                    v->per_type.exception.script_filename,
                    v->per_type.exception.script_line,
                    v->per_type.exception.script_column);
                if (v->per_type.exception.inner != NULL)
                    str_builder_addf(sb, "\n\t%s", variant_as_str(v->per_type.exception.inner));
                v->str_repr = malloc(strlen(str_builder_charptr(sb)) + 1);
                strcpy((char *)v->str_repr, str_builder_charptr(sb));
                str_builder_free(sb);
            }
            return v->str_repr;

            return v->per_type.exception.msg;
        default:
            return NULL;
    }
}

list *variant_as_list(variant *v) {
    switch (v->type) {
        case VT_NULL:
            return NULL;
        case VT_BOOL:
            return list_of(variant_class, 1, v);
        case VT_INT:
            return list_of(variant_class, 1, v);
        case VT_FLOAT:
            return list_of(variant_class, 1, v);
        case VT_STR:
            return list_of(variant_class, 1, v);
        case VT_LIST:
            return v->per_type.list_;
        case VT_DICT:
            return dict_get_values(v->per_type.dict_);
        case VT_CALLABLE:
            return NULL;
        default:
            return NULL;
    }
}

dict *variant_as_dict(variant *v) {
    switch (v->type) {
        case VT_NULL:
            return NULL;
        case VT_BOOL:
            return new_dict(variant_class);
        case VT_INT:
            return new_dict(variant_class);
        case VT_FLOAT:
            return new_dict(variant_class);
        case VT_STR:
            return new_dict(variant_class);
        case VT_LIST:
            return new_dict(variant_class);
        case VT_DICT:
            return v->per_type.dict_;
        case VT_CALLABLE:
            return NULL;
        default:
            return NULL;
    }
}

callable *variant_as_callable(variant *v) {
    switch (v->type) {
        case VT_NULL:
        case VT_BOOL:
        case VT_INT:
        case VT_FLOAT:
        case VT_STR:
        case VT_LIST:
        case VT_DICT:
            return NULL;
        case VT_CALLABLE:
            return v->per_type.callable_;
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

    // if we allow ("4"==4) we have the pitfalls of javascript
    if (a->type != b->type)
        return false;
    
    switch (a->type) {
        case VT_NULL:
            return true;
        case VT_BOOL:
            return a->per_type.bool_ == b->per_type.bool_;
        case VT_INT:
            return a->per_type.int_ == b->per_type.int_;
        case VT_FLOAT:
            return a->per_type.float_ == b->per_type.float_;
        case VT_STR:
            if (a->per_type.s.len != b->per_type.s.len)
                return false;
            return memcmp(a->per_type.s.ptr, b->per_type.s.ptr, a->per_type.s.len) == 0;
        case VT_LIST:
            return lists_are_equal(a->per_type.list_, b->per_type.list_);
        case VT_DICT:
            return dicts_are_equal(a->per_type.dict_, b->per_type.dict_);
        case VT_CALLABLE:
            return callables_are_equal(a->per_type.callable_, b->per_type.callable_);
        case VT_EXCEPTION:
            return strcmp(a->per_type.exception.msg, b->per_type.exception.msg) == 0;
    }

    // we shouldn't get here...
    return false;
}

const void variant_describe(variant *v, str_builder *sb) {
    str_builder_add(sb, variant_as_str(v));
}


