#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "strbld.h"

struct strbld {
    char *buffer;
    int capacity;
    int length;
};

strbld *new_strbld() {
    strbld *sb = malloc(sizeof(strbld));
    sb->length = 0;
    sb->capacity = 16;
    sb->buffer = malloc(sb->capacity);
    sb->buffer[0] = '\0';
    return sb;
}

static void ensure_capacity(strbld *sb, int len_needed) {
    if (sb->capacity < sb->length + len_needed + 1) {
        while (sb->capacity < sb->length + len_needed + 1)
            sb->capacity *= 2;
        sb->buffer = realloc(sb->buffer, sb->capacity);
    }
}

void strbld_cat(strbld *sb, const char *str) {
    ensure_capacity(sb, strlen(str));
    strcat(sb->buffer, str);
    sb->length += strlen(str);
}

void strbld_catc(strbld *sb, char chr) {
    ensure_capacity(sb, 1);
    sb->buffer[sb->length++] = chr;
    sb->buffer[sb->length] = '\0';
}

void strbld_catf(strbld *sb, char *fmt, ...) {
    char tmp[128];

    va_list args;
    va_start(args, fmt);
    vsnprintf(tmp, sizeof(tmp), fmt, args);
    va_end(args);

    strbld_cat(sb, tmp);
}

const char *strbld_charptr(strbld *sb) {
    return sb->buffer;
}

