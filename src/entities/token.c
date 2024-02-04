#include <stdlib.h>
#include <string.h>
#include "token.h"
#include "../utils/containers/_module.h"
#include "../utils/str_builder.h"


contained_item *containing_tokens = &(contained_item){
    .type_name = "token",
    .are_equal = (are_equal_func)tokens_are_equal,
    .to_string = (to_string_func)token_to_string,
    .hash = NULL
};

struct token {
    token_type type;
    const char *data; // e.g. identifier or number
    const char *filename;
    int line_no;
    int column_no;
};

token *new_token(token_type type, const char *filename, int line_no, int column_no) {
    token *t = malloc(sizeof(token));
    t->type = type;
    t->data = NULL;
    t->filename = filename;
    t->line_no = line_no;
    t->column_no = column_no;
    return t;
}

token *new_data_token(token_type type, const char *data, const char *filename, int line_no, int column_no) {
    token *t = malloc(sizeof(token));
    t->type = type;
    t->data = data;
    t->filename = filename;
    t->line_no = line_no;
    t->column_no = column_no;
    return t;
}

token_type inline token_get_type(token *t) {
    return t->type;
}

inline const char *token_get_data(token *t) {
    return t->data;
}

const char *token_get_file_name(token *t) {
    return t->filename;
}

int token_get_file_line_no(token *t) {
    return t->line_no;
}

int token_get_file_col_no(token *t) {
    return t->column_no;
}


void token_print(token *t, FILE *stream, char *prefix) {
    token_type tt = token_get_type(t);
    const char *data = token_get_data(t);
    bool has_data = (tt == T_IDENTIFIER || tt == T_STRING_LITERAL || tt == T_NUMBER_LITERAL || tt == T_BOOLEAN_LITERAL);

    fprintf(stream, "%s%s%s%s%s", 
        prefix,
        token_type_str(tt), 
        has_data ? " \"" : "",
        has_data ? data : "",
        has_data ? "\"" : ""
    );
}

const char *token_to_string(token *t) {
    str_builder *sb = new_str_builder();

    token_type tt = token_get_type(t);
    const char *data = token_get_data(t);
    bool has_data = (tt == T_IDENTIFIER || tt == T_STRING_LITERAL || tt == T_NUMBER_LITERAL || tt == T_BOOLEAN_LITERAL);
    
    str_builder_add(sb, token_type_str(tt));
    if (has_data)
        str_builder_addf(sb, "(\"%s\")", t->data);
    
    return str_builder_charptr(sb);
}

void token_print_list(list *tokens, FILE *stream, char *prefix, char *separator) {
    int i = 0;
    for_list(tokens, it, token, t) {
        if (i++ > 0)
            fprintf(stream, "%s", separator);
        token_print(t, stream, prefix);
    }
}

bool tokens_are_equal(token *a, token *b) {
    if (a == NULL && b == NULL) return true;
    if ((a == NULL && b != NULL) || (a != NULL && b == NULL)) return false;
    if (a == b) return true;
    
    if (a->type != b->type)
        return false;
    if (strcmp(a->data, b->data) != 0)
        return false;

    return true;
}

STRONGLY_TYPED_FAILABLE_PTR_IMPLEMENTATION(token);
