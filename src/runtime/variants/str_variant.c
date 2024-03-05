#include "_internal.h"
#include <string.h>


typedef struct str_instance {
    BASE_VARIANT_FIRST_ATTRIBUTES;
    char *buffer;
    int capacity;
    int length;
} str_instance;

static void ensure_capacity(str_instance *obj, int capacity) {
    if (obj->capacity >= capacity)
        return;
    
    while (obj->capacity < capacity)
        obj->capacity *= 2;
    
    obj->buffer = realloc(obj->buffer, obj->capacity);
}

static void initialize(str_instance *obj, variant *args, variant *named_args) {
    obj->capacity = 16;
    obj->buffer = malloc(obj->capacity);
    obj->length = 0;
    obj->buffer[0] = 0;
}

static void destruct(str_instance *obj) {
    if (obj->buffer != NULL) {
        free(obj->buffer);
    }
}

static void copy_initialize(str_instance *obj, str_instance *original) {
    obj->capacity = original->capacity;
    obj->buffer = malloc(obj->capacity);
    obj->length = original->length;
    strcpy(obj->buffer, original->buffer);
}

static variant *stringify(str_instance *obj) {
    return variant_clone((variant *)obj);
}

static unsigned hash(str_instance *obj) {
    unsigned long long_value = 0;
    unsigned char *data_ptr = (unsigned char *)obj->buffer;
    int data_bytes = obj->length;
    while (data_bytes-- > 0) {
        long_value = (long_value << 5) + *data_ptr;
        long_value = ((long_value & 0xFFFFFFFF00000000) >> 32) ^ (long_value & 0xFFFFFFFF);
        data_ptr++;
    }
    return (unsigned)long_value;
}

static int compare(str_instance *a, str_instance *b) {
    if (a->length != b->length)
        return a->length - b->length;
    
    return strcmp(a->buffer, b->buffer);
}

static bool are_equal(str_instance *a, str_instance *b) {
    return strcmp(a->buffer, b->buffer) == 0;
}

variant_type *str_type = &(variant_type){
    // ._type = type_of_types,
    .name = "str",
    .parent_type = NULL,
    .instance_size = sizeof(str_instance),

    .initializer = (initialize_func)initialize,
    .destructor = (destruct_func)destruct,
    .copy_initializer = (copy_initializer_func)copy_initialize,
    .stringifier = (stringifier_func)stringify,
    .hasher = (hashing_func)hash,
    .comparer = (compare_func)compare,
    .equality_checker = (equals_func)are_equal
};

variant *new_str_variant(const char *value) {
    str_instance *s = (str_instance *)variant_create(str_type, NULL, NULL);
    if (value != NULL) {
        ensure_capacity(s, strlen(value) + 1);
        strcpy(s->buffer, value);
        s->length = strlen(value);
    }
    return (variant *)s;
}

const char *str_variant_as_str(variant *v) {
    if (!variant_is(v, str_type))
        return NULL;
    return ((str_instance *)v)->buffer;
}
