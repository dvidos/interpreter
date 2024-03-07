#include "_skeleton.h"

/*
    At a glance:
    ------------------
    * All static methods in the vtable
    * All supported methods in the class info
    * Public variable pointing to struct_info
    * Private variable pointing to vtable
    * Public constructor
*/

static void skeleton_method_1(skeleton *p) {
    // ...
}

static long skeleton_method_2(skeleton *p, long number) {
    // ...
}

static void skeleton_method_3(skeleton *p, const char *ptr) {
    // ...
}

static void struct_describe(skeleton *p, str_builder *sb) {
    // ...
}

static bool struct_equals(skeleton *a, skeleton *b) {
    // ...
}

static unsigned struct_hash(skeleton *p) {
    // ...
}

static void *struct_clone(skeleton *p) {
    // ...
}

static int struct_compare(skeleton *a, skeleton *b) {
    // ...
}

static void struct_destruct(skeleton *p) {
    // ...
}

// public instance of class info
struct_info *skeleton_struct = &(struct_info){
    .struct_name = "skeleton",
    .enclosed = NULL,
    ._struct_info_magic_number = STRUCT_INFO_MAGIC_NUMBER,
    
    .describe = (describe_item_func *)struct_describe,
    .equals   = (instances_are_equal_func *)struct_equals,
    .hash     = (hash_instance_func *)struct_hash,
    .clone    = (clone_instance_func *)struct_clone,
    .compare  = (compare_instances_func *)struct_compare,
    .destruct = (destruct_instance_func *)struct_destruct,
};

// private instance of the vtable
static skeleton_vtable *skeleton_vt = &(skeleton_vtable){
    .skeleton_method_1 = skeleton_method_1,
    .skeleton_method_2 = skeleton_method_2,
    .skeleton_method_3 = skeleton_method_3,
};

// public constructor(s)
skeleton *new_skeleton() {
    skeleton *p = malloc(sizeof(skeleton));
    p->_info = skeleton_struct;
    p->_vt = skeleton_vt;
    p->skeleton_attr_1 = NULL;
    p->skeleton_attr_2 = 12;
    p->skeleton_attr_3 = true;
    return p;
}

