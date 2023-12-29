

typedef struct token {
    enum token_type type;
    char *data; // e.g. identifier or number
} token;

token *new_token(token_type type);
token *new_token_data(token_type type, const char *data);

token_type token_get_type(token *t);
