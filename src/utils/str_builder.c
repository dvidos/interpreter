#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "str_builder.h"

struct str_builder {
    char *buffer;
    int capacity;
    int length;
};

str_builder *new_str_builder() {
    str_builder *sb = malloc(sizeof(str_builder));
    sb->length = 0;
    sb->capacity = 16;
    sb->buffer = malloc(sb->capacity);
    sb->buffer[0] = '\0';
    return sb;
}

void str_builder_clear(str_builder *sb) {
    // no matter what capacity exists.
    sb->length = 0;
    sb->buffer[0] = '\0';
}

static void ensure_capacity(str_builder *sb, int len_needed) {
    if (sb->capacity < sb->length + len_needed + 1) {
        while (sb->capacity < sb->length + len_needed + 1)
            sb->capacity *= 2;
        sb->buffer = realloc(sb->buffer, sb->capacity);
    }
}

void str_builder_add(str_builder *sb, const char *s) {
    ensure_capacity(sb, strlen(s));
    strcat(sb->buffer, s);
    sb->length += strlen(s);
}

void str_builder_addc(str_builder *sb, char chr) {
    ensure_capacity(sb, 1);
    sb->buffer[sb->length++] = chr;
    sb->buffer[sb->length] = '\0';
}

void str_builder_addf(str_builder *sb, char *fmt, ...) {
    char tmp[128];

    va_list args;
    va_start(args, fmt);
    vsnprintf(tmp, sizeof(tmp), fmt, args);
    va_end(args);

    str_builder_add(sb, tmp);
}

const char *str_builder_charptr(str_builder *sb) {
    return sb->buffer;
}

void str_builder_free(str_builder *sb) {
    if (sb != NULL) {
        if (sb->buffer != NULL)
            free(sb->buffer);
        sb->buffer = NULL;
        free(sb);
    }
}
