#include <stdlib.h>
#include "token.h"

struct token {
    token_type type;
    const char *data; // e.g. identifier or number
};

token *new_token(token_type type) {
    token *t = malloc(sizeof(token));
    t->type = type;
    t->data = NULL;
    return t;
}

token *new_data_token(token_type type, const char *data) {
    token *t = malloc(sizeof(token));
    t->type = type;
    t->data = data;
    return t;
}

token_type inline token_get_type(token *t) {
    return t->type;
}

inline const char *token_get_data(token *t) {
    return t->data;
}

STRONGLY_TYPED_FAILABLE_IMPLEMENTATION(token);
