#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "../class.h"
#include "../str_builder.h"
#include "pair.h"

typedef struct pair {
    class *class;
    class *left_item_class;
    class *right_item_class;
    void *left;
    void *right;
} pair;


pair *new_pair(class *left_item_class, void *left, class *right_item_class, void *right) {
    pair *p = malloc(sizeof(pair));
    p->class = pair_class;
    p->left_item_class = left_item_class;
    p->right_item_class = right_item_class;
    p->left = left;
    p->right= right;
    return p;
}

void *pair_get_left(pair *p) {
    return p->left;
}

void *pair_get_right(pair *p) {
    return p->right;
}

bool pairs_are_equal(pair *a, pair *b) {
    if (a == NULL && b == NULL)
        return true;
    if ((a == NULL && b != NULL) || (a != NULL && b == NULL))
        return false;
    if (a == b)
        return true;
    
    if (!classes_are_equal(a->left_item_class, b->left_item_class))
        return false;
    if (!classes_are_equal(a->right_item_class, b->right_item_class))
        return false;

    bool equal;

    if (a->left_item_class != NULL && a->left_item_class->are_equal != NULL)
        equal = a->left_item_class->are_equal(a->left, b->left);
    else
        equal = a->left == b->left;
    if (!equal)
        return false;
    
    if (a->right_item_class != NULL && a->right_item_class->are_equal != NULL)
        equal = a->right_item_class->are_equal(a->right, b->right);
    else
        equal = a->right == b->right;
    if (!equal)
        return false;

    return true;
}

const void pair_describe(pair *p, const char *separator, str_builder *sb) {
    if (p->left == NULL)
        str_builder_add(sb, "(null)");
    else if (p->left_item_class != NULL && p->left_item_class->describe != NULL)
        p->left_item_class->describe(p->left, sb);
    else
        str_builder_addf(sb, "@0x%p", p->left);

    str_builder_add(sb, separator);

    if (p->right == NULL)
        str_builder_add(sb, "(null)");
    else if (p->right_item_class != NULL && p->right_item_class->describe != NULL)
        p->right_item_class->describe(p->right, sb);
    else
        str_builder_addf(sb, "@0x%p", p->right);
}


const void default_pair_describe(pair *p, str_builder *sb) {
    pair_describe(p, "-", sb);
}


STRONGLY_TYPED_FAILABLE_PTR_IMPLEMENTATION(pair);

class *pair_class = &(class){
    .type_name = "pair",
    .are_equal = (are_equal_func)pairs_are_equal,
    .describe = (describe_func)default_pair_describe,
    .hash      = NULL
};
