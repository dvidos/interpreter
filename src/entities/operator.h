#ifndef _OPERATOR_H
#define _OPERATOR_H

#include <stdio.h>
#include "../utils/failable.h"
#include "../utils/containers/_module.h"
#include "token_type.h"
#include "operator_type.h"


typedef struct operator operator;

operator *new_operator(operator_type type, token *token);

operator_type operator_get_type(operator *o);
token *operator_get_token(operator *o);

const char *operator_to_string(operator *o);
bool operators_are_equal(operator *a, operator *b);

extern contained_item *containing_operators;


STRONGLY_TYPED_FAILABLE_PTR_DECLARATION(operator);

#endif
