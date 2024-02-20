#include "class_info.h"

class_info *get_instance_class(void *instance) {
    // the first pointer of the pointed instance should point to a class_info
    if (instance == NULL) return NULL;
    class_info *c = *((class_info **)instance);
    if (c == NULL) return NULL;
    return (c->_class_info_magic_number == CLASS_INFO_MAGIC_NUMBER) ? c : NULL;
}

const char *class_name(void *instance) {
    if (instance == NULL)
        return NULL;
    class_info *c = get_instance_class(instance);
    if (c == NULL || c->name == NULL)
        return NULL;
    return c->name;
}

bool is_of_class_or_subclass(void *instance, class_info *c) {
    class_info *actual = get_instance_class(instance);
    // check if instance is a class_info, or subclass of.
    while (actual != NULL) {
        if (actual == c)
            return true;
        actual = actual->parent;
    }
    return false;
}

void class_describe(void *instance, str_builder *sb) {
    if (instance == NULL) {
        str_builder_add(sb, "(null)");
        return;
    }
    class_info *c = get_instance_class(instance);
    if (c != NULL && c->describe != NULL) {
        c->describe(instance, sb);
        return;
    }
    str_builder_addf(sb, "0x%p", instance);
}

bool class_equals(void *instance_a, void *instance_b) {
    class_info *c = get_instance_class(instance_a);
    if (c != NULL && c->equals != NULL)
        return c->equals(instance_a, instance_b);
    return (instance_a == instance_b);
}

unsigned class_hash(void *instance) {
    class_info *c = get_instance_class(instance);
    if (c != NULL && c->hash != NULL)
        return c->hash(instance);
    return (unsigned)(((long)instance >> 32) ^ ((long)instance & 0xFFFFFFFF));
}

void *class_clone(void *instance) {
    class_info *c = get_instance_class(instance);
    if (c != NULL && c->clone != NULL)
        return c->clone(instance);
    return NULL;
}

int class_compare(void *instance_a, void *instance_b) {
    class_info *c = get_instance_class(instance_a);
    if (c != NULL && c->compare != NULL)
        return c->compare(instance_a, instance_b);
    return (int)(((long)instance_a) - ((long)instance_b));
}

void class_destruct(void *instance) {
    class_info *c = get_instance_class(instance);
    if (c != NULL && c->destruct != NULL)
        c->destruct(instance);
}

unsigned class_generic_hash(unsigned hash_value, const void *data_ptr, int data_bytes) {
    unsigned long long_value = hash_value;
    while (data_bytes-- > 0) {
        long_value = (long_value << 5) + *(unsigned char *)data_ptr;
        long_value = ((long_value & 0xFFFFFFFF00000000) >> 32) ^ (long_value & 0xFFFFFFFF);
        data_ptr++;
    }
    return (unsigned)long_value;
}
