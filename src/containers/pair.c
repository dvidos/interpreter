#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "../utils/str_builder.h"
#include "pair.h"

typedef struct pair {
    contained_item_info *item_info;
    contained_item_info *left_item_info;
    contained_item_info *right_item_info;
    void *left;
    void *right;
} pair;


pair *new_pair(contained_item_info *left_item_info, void *left, contained_item_info *right_item_info, void *right) {
    pair *p = malloc(sizeof(pair));
    p->item_info = pair_item_info;
    p->left_item_info = left_item_info;
    p->right_item_info = right_item_info;
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
    
    if (!item_infos_are_equal(a->left_item_info, b->left_item_info))
        return false;
    if (!item_infos_are_equal(a->right_item_info, b->right_item_info))
        return false;

    bool equal;

    if (a->left_item_info != NULL && a->left_item_info->are_equal != NULL)
        equal = a->left_item_info->are_equal(a->left, b->left);
    else
        equal = a->left == b->left;
    if (!equal)
        return false;
    
    if (a->right_item_info != NULL && a->right_item_info->are_equal != NULL)
        equal = a->right_item_info->are_equal(a->right, b->right);
    else
        equal = a->right == b->right;
    if (!equal)
        return false;

    return true;
}

const void pair_describe(pair *p, const char *separator, str_builder *sb) {
    if (p->left == NULL)
        str_builder_add(sb, "(null)");
    else if (p->left_item_info != NULL && p->left_item_info->describe != NULL)
        p->left_item_info->describe(p->left, sb);
    else
        str_builder_addf(sb, "@0x%p", p->left);

    str_builder_add(sb, separator);

    if (p->right == NULL)
        str_builder_add(sb, "(null)");
    else if (p->right_item_info != NULL && p->right_item_info->describe != NULL)
        p->right_item_info->describe(p->right, sb);
    else
        str_builder_addf(sb, "@0x%p", p->right);
}


const void default_pair_describe(pair *p, str_builder *sb) {
    pair_describe(p, "-", sb);
}


STRONGLY_TYPED_FAILABLE_PTR_IMPLEMENTATION(pair);

contained_item_info *pair_item_info = &(contained_item_info){
    .item_info_magic = ITEM_INFO_MAGIC,
    .type_name = "pair",
    .are_equal = (items_equal_func)pairs_are_equal,
    .describe = (describe_item_func)default_pair_describe,
    .hash      = NULL
};
