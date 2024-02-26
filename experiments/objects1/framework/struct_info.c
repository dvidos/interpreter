#include "struct_info.h"

struct_info *get_instance_info(void *instance) {
    // the first pointer of the pointed instance should point to a struct_info
    if (instance == NULL)
        return NULL;
    
    struct_info *c = *((struct_info **)instance);
    if (c == NULL)
        return NULL;
    if (c->_struct_info_magic_number != STRUCT_INFO_MAGIC_NUMBER)
        return NULL;
    
    return c;
}

const char *instance_struct_name(void *instance) {
    if (instance == NULL)
        return NULL;
    
    struct_info *c = get_instance_info(instance);
    if (c == NULL || c->struct_name == NULL)
        return NULL;
    
    return c->struct_name;
}

bool is_or_encloses_struct(void *instance, struct_info *c) {
    struct_info *actual = get_instance_info(instance);
    // check if instance is a struct_info, or subclass of.
    while (actual != NULL) {
        if (actual == c)
            return true;
        actual = actual->enclosed;
    }
    return false;
}

void instance_describe(void *instance, str_builder *sb) {
    if (instance == NULL) {
        str_builder_add(sb, "(null)");
        return;
    }
    struct_info *c = get_instance_info(instance);
    if (c != NULL && c->describe != NULL) {
        c->describe(instance, sb);
        return;
    }
    str_builder_addf(sb, "0x%p", instance);
}

bool instances_are_equal(void *instance_a, void *instance_b) {
    struct_info *c = get_instance_info(instance_a);
    if (c != NULL && c->equals != NULL)
        return c->equals(instance_a, instance_b);
    
    return (instance_a == instance_b);
}

unsigned instance_hash(void *instance) {
    struct_info *c = get_instance_info(instance);
    if (c != NULL && c->hash != NULL)
        return c->hash(instance);
    return (unsigned)(((long)instance >> 32) ^ ((long)instance & 0xFFFFFFFF));
}

void *instance_clone(void *instance) {
    struct_info *c = get_instance_info(instance);
    if (c != NULL && c->clone != NULL)
        return c->clone(instance);
    return NULL;
}

int instances_compare(void *instance_a, void *instance_b) {
    struct_info *c = get_instance_info(instance_a);
    if (c != NULL && c->compare != NULL)
        return c->compare(instance_a, instance_b);
    return (int)(((long)instance_a) - ((long)instance_b));
}

void instance_destruct(void *instance) {
    struct_info *c = get_instance_info(instance);
    if (c != NULL && c->destruct != NULL)
        c->destruct(instance);
}

unsigned instance_generic_hash(unsigned hash_value, const void *data_ptr, int data_bytes) {
    unsigned long long_value = hash_value;
    while (data_bytes-- > 0) {
        long_value = (long_value << 5) + *(unsigned char *)data_ptr;
        long_value = ((long_value & 0xFFFFFFFF00000000) >> 32) ^ (long_value & 0xFFFFFFFF);
        data_ptr++;
    }
    return (unsigned)long_value;
}
