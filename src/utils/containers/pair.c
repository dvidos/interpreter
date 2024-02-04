#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "contained_item.h"
#include "../str_builder.h"
#include "pair.h"

typedef struct pair {
    contained_item *left_item;
    contained_item *right_item;
    void *left;
    void *right;
} pair;


pair *new_pair(contained_item *left_item, void *left, contained_item *right_item, void *right) {
    pair *p = malloc(sizeof(pair));
    p->left_item = left_item;
    p->right_item = right_item;
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
    
    if (!contained_item_info_are_equal(a->left_item, b->left_item))
        return false;
    if (!contained_item_info_are_equal(a->right_item, b->right_item))
        return false;

    bool equal;

    if (a->left_item != NULL && a->left_item->are_equal != NULL)
        equal = a->left_item->are_equal(a->left, b->left);
    else
        equal = a->left == b->left;
    if (!equal)
        return false;
    
    if (a->right_item != NULL && a->right_item->are_equal != NULL)
        equal = a->right_item->are_equal(a->right, b->right);
    else
        equal = a->right == b->right;
    if (!equal)
        return false;

    return true;
}

const char *pair_to_string(pair *p, const char *separator) {
    str_builder *sb = new_str_builder();

    if (p->left == NULL)
        str_builder_add(sb, "(null)");
    else if (p->left_item != NULL && p->left_item->to_string != NULL)
        str_builder_add(sb, p->left_item->to_string(p->left));
    else
        str_builder_addf(sb, "@0x%p", p->left);

    str_builder_add(sb, separator);

    if (p->right == NULL)
        str_builder_add(sb, "(null)");
    else if (p->right_item != NULL && p->right_item->to_string != NULL)
        str_builder_add(sb, p->right_item->to_string(p->right));
    else
        str_builder_addf(sb, "@0x%p", p->right);
    
    return str_builder_charptr(sb);
}


const char *default_pair_to_string(pair *p) {
    return pair_to_string(p, "-");
}


STRONGLY_TYPED_FAILABLE_PTR_IMPLEMENTATION(pair);

contained_item *containing_pairs = &(contained_item){
    .type_name = "pair",
    .are_equal = (are_equal_func)pairs_are_equal,
    .to_string = (to_string_func)default_pair_to_string,
    .hash      = NULL
};
