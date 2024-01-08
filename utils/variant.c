#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include "containable.h"
#include "variant.h"


typedef enum value_type {
    VT_NULL,
    VT_BOOL,
    VT_INT,
    VT_FLOAT,
    VT_STR,
    VT_LIST,
    VT_DICT,
} value_type;

struct variant {
    containable *containable;
    value_type type;
    union {
        bool b;
        int i;
        float f;
        struct {
            char *ptr;
            int len;
        } s;
        list *lst;
        dict *dct;
    } per_type;
    const char *str_repr;
};

variant *new_variant() {
    variant *v = malloc(sizeof(variant));
    memset(v, 0, sizeof(variant));
    v->containable = new_containable("value", 
        (are_equal_func)variants_are_same, 
        (to_string_func)variant_to_string);
    v->type = VT_NULL;
    return v;
}

variant *new_bool_variant(bool b) {
    variant *v = new_variant();
    v->type = VT_BOOL;
    v->per_type.b = b;
    return v;
}

variant *new_int_variant(int i) {
    variant *v = new_variant();
    v->type = VT_INT;
    v->per_type.i = i;
    return v;
}

variant *new_float_variant(float f) {
    variant *v = new_variant();
    v->type = VT_FLOAT;
    v->per_type.f = f;
    return v;
}

variant *new_str_variant(const char *p) {
    variant *v = new_variant();
    v->type = VT_STR;
    v->per_type.s.len = strlen(p);
    v->per_type.s.ptr = malloc(v->per_type.s.len + 1);
    strcpy(v->per_type.s.ptr, p);
    return v;
}

variant *new_list_variant(list *l) {
    variant *v = new_variant();
    v->type = VT_LIST;
    v->per_type.lst = l;
    return v;
}

variant *new_dict_variant(dict *d) {
    variant *v = new_variant();
    v->type = VT_DICT;
    v->per_type.dct = d;
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

bool value_is_float(variant *v) {
    return v->type == VT_FLOAT;
}

bool value_is_str(variant *v) {
    return v->type == VT_STR;
}

bool variant_is_list(variant *v) {
    return v->type == VT_LIST;
}

bool variant_is_dict(variant *v) {
    return v->type == VT_DICT;
}

bool variant_as_bool(variant *v) {
    switch (v->type) {
        case VT_NULL:
            return false;
        case VT_BOOL:
            return v->per_type.b;
        case VT_INT:
            return v->per_type.i != 0;
        case VT_FLOAT:
            return v->per_type.f != 0.0;
        case VT_STR:
            return (
                strcmp(v->per_type.s.ptr, "true") == 0 || 
                strcmp(v->per_type.s.ptr, "1") == 0
            );
        case VT_LIST:
            return v->per_type.lst != NULL && list_length(v->per_type.lst) > 0;
        case VT_DICT:
            return v->per_type.dct != NULL && dict_count(v->per_type.dct) > 0;
        default:
            return false;
    }
}

int variant_as_int(variant *v) {
    switch (v->type) {
        case VT_NULL:
            return 0;
        case VT_BOOL:
            return v->per_type.b ? 1 : 0;
        case VT_INT:
            return v->per_type.i;
        case VT_FLOAT:
            return (int)v->per_type.f;
        case VT_STR:
            return atoi(v->per_type.s.ptr);
        case VT_LIST:
            return v->per_type.lst == NULL ? 0 : list_length(v->per_type.lst);
        case VT_DICT:
            return v->per_type.lst == NULL ? 0 : dict_count(v->per_type.dct);
        default:
            return false;
    }
}

float variant_as_float(variant *v) {
    switch (v->type) {
        case VT_NULL:
            return 0.0;
        case VT_BOOL:
            return v->per_type.b ? 1.0 : 0.0;
        case VT_INT:
            return (float)v->per_type.i;
        case VT_FLOAT:
            return v->per_type.f;
        case VT_STR:
            return atof(v->per_type.s.ptr);
        case VT_LIST:
            return v->per_type.lst == NULL ? 0.0 : (float)list_length(v->per_type.lst);
        case VT_DICT:
            return v->per_type.lst == NULL ? 0.0 : (float)dict_count(v->per_type.dct);
        default:
            return false;
    }
}

const char *variant_as_str(variant *v) {
    switch (v->type) {
        case VT_NULL:
            return NULL;
        case VT_BOOL:
            return v->per_type.b ? "true" : "false";
        case VT_INT:
            if (v->str_repr == NULL) {
                v->str_repr = malloc(64);
                memset((char *)v->str_repr, 0, 64);
                snprintf((char *)v->str_repr, 64, "%d", v->per_type.i);
            }
            return v->str_repr;
        case VT_FLOAT:
            if (v->str_repr == NULL) {
                v->str_repr = malloc(64);
                memset((char *)v->str_repr, 0, 64);
                snprintf((char *)v->str_repr, 64, "%f", v->per_type.f);
            }
            return v->str_repr;
        case VT_STR:
            return v->per_type.s.ptr;
        case VT_LIST:
            if (v->str_repr == NULL) {
                if (v->per_type.lst != NULL)
                    v->str_repr = list_to_string(v->per_type.lst, ", ");
            }
            return v->str_repr;
        case VT_DICT:
            if (v->str_repr == NULL) {
                if (v->per_type.dct != NULL)
                    v->str_repr = dict_to_string(v->per_type.dct, ":", ", ");
            }
            return v->str_repr;
        default:
            return NULL;
    }
}

list *variant_as_list(variant *v) {
    switch (v->type) {
        case VT_NULL:
            return NULL;
        case VT_BOOL:
            return list_of(1, v);
        case VT_INT:
            return list_of(1, v);
        case VT_FLOAT:
            return list_of(1, v);
        case VT_STR:
            return list_of(1, v);
        case VT_LIST:
            return v->per_type.lst;
        case VT_DICT:
            // should get values of the dict...
            return new_list();
        default:
            return NULL;
    }
}

dict *variant_as_dict(variant *v) {
    switch (v->type) {
        case VT_NULL:
            return NULL;
        case VT_BOOL:
            return new_dict(10);
        case VT_INT:
            return new_dict(10);
        case VT_FLOAT:
            return new_dict(10);
        case VT_STR:
            return new_dict(10);
        case VT_LIST:
            return new_dict(10);
        case VT_DICT:
            return v->per_type.dct;
        default:
            return NULL;
    }
}

bool variants_are_same(variant *a, variant *b) {
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
            return a->per_type.b == b->per_type.b;
        case VT_INT:
            return a->per_type.i == b->per_type.i;
        case VT_FLOAT:
            return a->per_type.f == b->per_type.f;
        case VT_STR:
            if (a->per_type.s.len != b->per_type.s.len)
                return false;
            return memcmp(a->per_type.s.ptr, b->per_type.s.ptr, a->per_type.s.len) == 0;
        case VT_LIST:
            return lists_are_equal(a->per_type.lst, b->per_type.lst);
        case VT_DICT:
            return dicts_are_equal(a->per_type.dct, b->per_type.dct);

        // for other types, we should implement is_dict_equal() etc.
    }

    // we shouldn't get here...
    return false;
}

const char *variant_to_string(variant *v) {
    return variant_as_str(v);
}

STRONGLY_TYPED_FAILABLE_IMPLEMENTATION(variant);
