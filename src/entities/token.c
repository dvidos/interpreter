#include <stdlib.h>
#include <string.h>
#include "token.h"
#include "../containers/_module.h"
#include "../utils/str_builder.h"


item_info *token_item_info = &(item_info){
    .item_info_magic = ITEM_INFO_MAGIC,
    .type_name = "token",
    .are_equal = (items_equal_func)tokens_are_equal,
    .describe = (describe_item_func)token_describe,
    .hash = NULL
};

token *new_token(token_type type, const char *filename, int line_no, int column_no) {
    token *t = malloc(sizeof(token));
    t->item_info = token_item_info;
    t->type = type;
    t->data = NULL;
    t->filename = filename;
    t->line_no = line_no;
    t->column_no = column_no;
    t->origin = new_origin(filename, line_no, column_no);
    return t;
}

token *new_data_token(token_type type, const char *data, const char *filename, int line_no, int column_no) {
    token *t = malloc(sizeof(token));
    t->item_info = token_item_info;
    t->type = type;
    t->data = data;
    t->filename = filename;
    t->line_no = line_no;
    t->column_no = column_no;
    return t;
}

void token_print(token *t, FILE *stream, char *prefix) {
    token_type tt = t->type;
    const char *data = t->data;
    bool has_data = (tt == T_IDENTIFIER || tt == T_STRING_LITERAL || tt == T_NUMBER_LITERAL || tt == T_BOOLEAN_LITERAL);

    fprintf(stream, "%s%s%s%s%s", 
        prefix,
        token_type_str(tt), 
        has_data ? " \"" : "",
        has_data ? data : "",
        has_data ? "\"" : ""
    );
}

const void token_describe(token *t, str_builder *sb) {
    token_type tt = t->type;
    const char *data = t->data;
    bool has_data = (tt == T_IDENTIFIER || tt == T_STRING_LITERAL || tt == T_NUMBER_LITERAL || tt == T_BOOLEAN_LITERAL);
    
    str_builder_add(sb, token_type_str(tt));
    if (has_data)
        str_builder_addf(sb, "(\"%s\")", t->data);
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
