#include <stdlib.h>
#include <string.h>
#include "token.h"
#include "operator.h"
#include "../utils/containers/_module.h"
#include "../utils/str_builder.h"


contained_item *containing_operators = &(contained_item){
    .type_name = "operator",
    .are_equal = (are_equal_func)operators_are_equal,
    .to_string = (to_string_func)operator_to_string,
    .hash = NULL
};

struct operator {
    operator_type type;
    token *token;
};

operator *new_operator(operator_type type, token *token) {
    operator *o = malloc(sizeof(operator));
    o->type = type;
    o->token = token;
    return o;
}

operator_type inline operator_get_type(operator *o) {
    return o->type;
}

token *operator_get_token(operator *o) {
    return o->token;
}

const char *operator_to_string(operator *o) {
    return operator_type_to_string(o->type);
}

bool operators_are_equal(operator *a, operator *b) {
    if (a == NULL && b == NULL) return true;
    if ((a == NULL && b != NULL) || (a != NULL && b == NULL)) return false;
    if (a == b) return true;
    
    if (a->type != b->type)
        return false;
    if (a->token != b->token)
        return false;

    return true;
}

STRONGLY_TYPED_FAILABLE_PTR_IMPLEMENTATION(operator);
