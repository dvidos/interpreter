#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "dict.h"

typedef struct dict_entry {
    const char *key;
    value *val;
    struct dict_entry *next;
} dict_entry;

typedef struct dict {
    dict_entry **entries_array;
    int capacity;
} dict;

dict *new_dict(int capacity) {
    dict *d = malloc(sizeof(dict));
    d->capacity = capacity;
    d->entries_array = malloc(sizeof(dict_entry *) * capacity);
    memset(d->entries_array, 0, d->capacity * sizeof(dict_entry *));
    return d;
}

static int hash(const char *str) {
    int result = 5381;
    while (*str) {
        // result = (result * 33) + char
        result = ((result << 5) + result) + *str++;
    }
    return result;
}

void dict_set(dict *d, const char *key, value *v) {
    dict_entry *entry = malloc(sizeof(dict_entry));
    entry->key = key;
    entry->val = v;
    entry->next = NULL;

    int slot = hash(key) % d->capacity;
    if (d->entries_array[slot] == NULL) {
        d->entries_array[slot] = entry;
    } else {
        dict_entry *e = d->entries_array[slot];
        while (e->next != NULL)
            e = e->next;
        e->next = entry;
    }
}

bool dict_has(dict *d, const char *key) {
    int slot = hash(key) % d->capacity;
    dict_entry *e = d->entries_array[slot];
    while (e != NULL) {
        if (strcmp(e->key, key) == 0)
            return true;
        e = e->next;
    }
    return false;
}

value *dict_get(dict *d, const char *key) {
    int slot = hash(key) % d->capacity;
    dict_entry *e = d->entries_array[slot];
    while (e != NULL) {
        if (strcmp(e->key, key) == 0)
            return e->val;
        e = e->next;
    }
    return NULL;
}

STRONGLY_TYPED_FAILABLE_IMPLEMENTATION(dict);
