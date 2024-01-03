#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "strbuff.h"

struct strbuff {
    char *buffer;
    int capacity;
    int length;
};

strbuff *new_strbuff() {
    strbuff *s = malloc(sizeof(strbuff));
    s->length = 0;
    s->capacity = 16;
    s->buffer = malloc(s->capacity);
    s->buffer[0] = '\0';
    return s;
}

static void ensure_capacity(strbuff *s, int extra_length) {
    if (s->length + extra_length + 1 >= s->capacity) {
        s->capacity *= 2;
        s->buffer = realloc(s->buffer, s->capacity);
    }
}

void strbuff_cat(strbuff *s, const char *str) {
    ensure_capacity(s, strlen(str));
    strcat(s->buffer, str);
    s->length += strlen(str);
}

void strbuff_catc(strbuff *s, char chr) {
    ensure_capacity(s, 1);
    s->buffer[s->length++] = chr;
    s->buffer[s->length] = '\0';
}

void strbuff_catf(strbuff *s, char *fmt, ...) {
    char tmp[128];

    va_list args;
    va_start(args, fmt);
    vsnprintf(tmp, sizeof(tmp), fmt, args);
    va_end(args);

    strbuff_cat(s, tmp);
}

const char *strbuff_charptr(strbuff *s) {
    return s->buffer;
}

