#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include "value.h"


typedef enum value_type {
    VT_NULL,
    VT_BOOL,
    VT_INT,
    VT_FLOAT,
    VT_STR,
    // other: array of, dict of, function
} value_type;

struct value {
    value_type type;
    union {
        bool b;
        int i;
        float f;
        struct {
            char *ptr;
            int len;
        } s;
    } per_type;
    char *str_repr;
};

value *new_value() {
    value *v = malloc(sizeof(value));
    memset(v, 0, sizeof(value));
    v->type = VT_NULL;
    return v;
}

value *new_bool_value(bool b) {
    value *v = new_value();
    v->type = VT_BOOL;
    v->per_type.b = b;
    return v;
}

value *new_int_value(int i) {
    value *v = new_value();
    v->type = VT_INT;
    v->per_type.i = i;
    return v;
}

value *new_float_value(float f) {
    value *v = new_value();
    v->type = VT_FLOAT;
    v->per_type.f = f;
    return v;
}

value *new_str_value(char *p) {
    value *v = new_value();
    v->type = VT_STR;
    v->per_type.s.len = strlen(p);
    v->per_type.s.ptr = malloc(v->per_type.s.len + 1);
    strcpy(v->per_type.s.ptr, p);
    return v;
}

bool value_is_null(value *v) {
    return v->type == VT_NULL;
}

bool value_is_bool(value *v) {
    return v->type == VT_BOOL;
}

bool value_is_int(value *v) {
    return v->type == VT_INT;
}

bool value_is_float(value *v) {
    return v->type == VT_FLOAT;
}

bool value_is_str(value *v) {
    return v->type == VT_STR;
}

bool value_as_bool(value *v) {
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
        default:
            return false;
    }
}

int value_as_int(value *v) {
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
        default:
            return false;
    }
}

float value_as_float(value *v) {
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
        default:
            return false;
    }
}

const char *value_as_str(value *v) {
    switch (v->type) {
        case VT_NULL:
            return NULL;
        case VT_BOOL:
            return v->per_type.b ? "true" : "false";
        case VT_INT:
            if (v->str_repr == NULL) {
                v->str_repr = malloc(64);
                memset(v->str_repr, 0, 64);
                snprintf(v->str_repr, 64, "%d", v->per_type.i);
            }
            return v->str_repr;
        case VT_FLOAT:
            if (v->str_repr == NULL) {
                v->str_repr = malloc(64);
                memset(v->str_repr, 0, 64);
                snprintf(v->str_repr, 64, "%f", v->per_type.f);
            }
            return v->str_repr;
        case VT_STR:
            return v->per_type.s.ptr;
        default:
            return NULL;
    }
}

bool values_are_same(value *a, value *b) {
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
        // for other types, we should implement is_dict_equal() etc.
    }

    // we shouldn't get here...
    return false;
}

STRONGLY_TYPED_FAILABLE_IMPLEMENTATION(value);
