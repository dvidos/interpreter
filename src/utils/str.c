#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "str.h"
#include "mem.h"
#include "../containers/contained_item_info.h"

#define at_least(value, threshold)   ((value) >= (threshold) ? (value) : (threshold))
#define at_most(value, threshold)    ((value) <= (threshold) ? (value) : (threshold))
#define between(value, low, high)    at_most(at_least(value, low), high)

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

str *new_str_of(const char *value) {
    str *s = new_str();
    str_adds(s, value);
    return s;
}

bool str_empty(str *s) {
    return s->length == 0;
}

int str_length(str *s) {
    return s->length;
}

void str_clear(str *s) {
    // no matter what capacity exists.
    s->length = 0;
    s->buffer[0] = '\0';
}

static void ensure_capacity(str *s, int extra_space_needed) {
    if (s->capacity < s->length + extra_space_needed + 1) {
        while (s->capacity < s->length + extra_space_needed + 1)
            s->capacity *= 2;
        s->buffer = realloc(s->buffer, s->capacity);
    }
}

void str_add(str *s, str *other) {
    ensure_capacity(s, other->length);
    strcat(s->buffer, other->buffer);
    s->length += other->length;
}

void str_adds(str *s, const char *cstr) {
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

    str_adds(s, tmp);
}

bool str_equals(str *s, const char *value) {
    return s->length == strlen(value) 
        && strcmp(s->buffer, value) == 0;
}

bool str_starts_with(str *s, const char *part) {
    return s->length >= strlen(part) 
        && strncmp(s->buffer, part, strlen(part)) == 0;
}

bool str_ends_with(str *s, const char *part) {
    return s->length >= strlen(part) 
        && strncmp(s->buffer + s->length - strlen(part), part, strlen(part)) == 0;
}

bool str_contains(str *s, const char *part) {
    return s->length >= strlen(part)
        && strstr(s->buffer, part) != NULL;
}

str *str_substr(str *s, int start, int length) {

    int actual_start = start >= 0 ? start : s->length - (-start);
    actual_start = between(actual_start, 0, s->length);

    int actual_length = length >= 0 ? length : strlen(s->buffer + actual_start) - (-length);
    actual_length = between(actual_length, 0, strlen(s->buffer + actual_start));

    char *temp = malloc(actual_length + 1);
    memcpy(temp, s->buffer + actual_start, actual_length);
    temp[actual_length] = 0;

    str *result = new_str_of(temp);
    free(temp);
    return result;
}

str *str_trim(str *s, const char *delimiters) {
    int start = 0;
    while (start < s->length && strchr(delimiters, s->buffer[start]) != NULL)
        start++;
    
    int length = s->length - start;
    while (length > 0 && strchr(delimiters, s->buffer[start+length-1]) != NULL)
        length--;

    return str_substr(s, start, length);
}

#define IS_WHITESPACE(c)  ((c)==' ' || (c)=='\t' || (c)=='\n' || (c)=='\r')
#define IS_DIGIT(c)       ((c)>='0' && (c)<='9')
#define IS_ALPHA(c)       ((c)>='a' && (c)<='z') || ((c)>='A'&&(c)<='Z')
#define IS_ALPHA_NUM(c)   (IS_ALPHA(c)||IS_DIGIT(c) || ((c)=='_'))

str *str_get_token(str *s, int *start) {
    if (*start < 0 || *start >= s->length)
        return NULL; // no more tokens.
    
    char c = s->buffer[*start];

    // skip whitespace
    while (IS_WHITESPACE(c) && *start < s->length) {
        c = s->buffer[++(*start)];
    }
    if (*start >= s->length)
        return NULL; // exhausted

    str *token = new_str();

    if (IS_DIGIT(c)) { // grab whole number
        while (IS_DIGIT(c) && *start < s->length) {
            str_addc(token, c);
            c = s->buffer[++(*start)];
        }
    } else if (IS_ALPHA(c)) { // grab alphanum
        while (IS_ALPHA_NUM(c) && *start < s->length) {
            str_addc(token, c);
            c = s->buffer[++(*start)];
        }
    } else if (c == '\'' || c == '"') {
        bool double_quote = (c == '"');
        str_addc(token, c);
        c = s->buffer[++(*start)];
        while (true) {
            str_addc(token, c);
            (*start)++;
            if (c == (double_quote ? '"' : '\'') || *start >= s->length)
                break;
            c = s->buffer[*start];
        }
    } else {
        str_addc(token, c);
        (*start)++;
    }

    return token;
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


static bool strs_are_equal(str *a, str *b) {
    if (a == b) return true;
    if (a == NULL && b != NULL) return false;
    if (a != NULL && b == NULL) return false;
    if (a->length != b->length) return false;
    if (strcmp(a->buffer, b->buffer) != 0) return false;

    return true;
}

static const void str_describe(str *s, str *str) {
    str_add(str, s);
}

contained_item_info *str_item_info = &(contained_item_info){
    .item_info_magic = ITEM_INFO_MAGIC,
    .type_name = "str",
    .are_equal = (items_equal_func)strs_are_equal,
    .describe = (describe_item_func)str_describe
};
