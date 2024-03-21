#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "str.h"
#include "mem.h"

struct str {
    char *buffer;
    int capacity;
    int length;
};

str *new_str() {
    str *s = malloc(sizeof(str));
    s->length = 0;
    s->capacity = 16;
    s->buffer = malloc(s->capacity);
    s->buffer[0] = '\0';
    return s;
}

void str_clear(str *s) {
    // no matter what capacity exists.
    s->length = 0;
    s->buffer[0] = '\0';
}

static void ensure_capacity(str *s, int free_space_needed) {
    if (s->capacity < s->length + free_space_needed + 1) {
        while (s->capacity < s->length + free_space_needed + 1)
            s->capacity *= 2;
        s->buffer = realloc(s->buffer, s->capacity);
    }
}

void str_add(str *s, const char *cstr) {
    ensure_capacity(s, strlen(cstr));
    strcat(s->buffer, cstr);
    s->length += strlen(cstr);
}

void str_addc(str *s, char chr) {
    ensure_capacity(s, 1);
    s->buffer[s->length++] = chr;
    s->buffer[s->length] = '\0';
}

void str_addf(str *s, char *fmt, ...) {
    char tmp[128];

    va_list args;
    va_start(args, fmt);
    vsnprintf(tmp, sizeof(tmp), fmt, args);
    va_end(args);

    str_add(s, tmp);
}

const char *str_cstr(str *s) {
    return s->buffer;
}

void str_free(str *s) {
    if (s != NULL) {
        if (s->buffer != NULL)
            free(s->buffer);
        s->buffer = NULL;
        free(s);
    }
}
