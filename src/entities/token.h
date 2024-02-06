#ifndef _TOKEN_H
#define _TOKEN_H

#include <stdio.h>
#include "../utils/failable.h"
#include "../utils/containers/_module.h"
#include "token_type.h"


typedef struct token token;

token *new_token(token_type type, const char *filename, int line_no, int column_no);
token *new_data_token(token_type type, const char *data, const char *filename, int line_no, int column_no);

token_type token_get_type(token *t);
const char *token_get_data(token *t);
const char *token_get_file_name(token *t);
int token_get_file_line_no(token *t);
int token_get_file_col_no(token *t);

void token_print(token *t, FILE *stream, char *prefix);
void token_print_list(list *tokens, FILE *stream, char *prefix, char *separator);

const void token_describe(token *t, str_builder *sb);
bool tokens_are_equal(token *a, token *b);

STRONGLY_TYPED_FAILABLE_PTR_DECLARATION(token);
#define failed_token(inner, fmt, ...)  __failed_token(inner, __func__, __FILE__, __LINE__, fmt, ##__VA_ARGS__)


extern class *token_class;


#endif
