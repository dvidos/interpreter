#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "../utils/strbuff.h"
#include "dict.h"

typedef struct dict_entry {
    const char *key;
    void *item;
    struct dict_entry *next;
} dict_entry;

typedef struct dict {
    dict_entry **entries_array;
    int capacity;
    int count;
} dict;

dict *new_dict(int capacity) {
    dict *d = malloc(sizeof(dict));
    d->capacity = capacity;
    d->count = 0;
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

void dict_set(dict *d, const char *key, void *item) {
    dict_entry *entry = malloc(sizeof(dict_entry));
    entry->key = key;
    entry->item = item;
    entry->next = NULL;

    int slot = hash(key) % d->capacity;
    if (d->entries_array[slot] == NULL) {
        d->entries_array[slot] = entry;
        d->count += 1;
    } else {
        dict_entry *e = d->entries_array[slot];
        while (e->next != NULL)
            e = e->next;
        e->next = entry;
        d->count += 1;
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

void *dict_get(dict *d, const char *key) {
    int slot = hash(key) % d->capacity;
    dict_entry *e = d->entries_array[slot];
    while (e != NULL) {
        if (strcmp(e->key, key) == 0)
            return e->item;
        e = e->next;
    }
    return NULL;
}

int dict_count(dict *d) {
    return d->count;
}

bool dict_is_empty(dict *d) {
    return d->count == 0;
}


typedef struct dict_iterator_private_data {
    dict *dict;
    int last_slot;
    dict_entry *last_entry;
} dict_iterator_private_data;
static void *dict_iterator_reset(iterator *it) {
    dict_iterator_private_data *pd = (dict_iterator_private_data *)it->private_data;
    if (pd->dict->count == 0) {
        pd->last_slot = -1;
        pd->last_entry = NULL;
    } else {
        pd->last_slot = 0;
        while (pd->dict->entries_array[pd->last_slot] == NULL)
            pd->last_slot += 1;
        pd->last_entry = pd->dict->entries_array[pd->last_slot];
    }
    return pd->last_entry == NULL ? NULL : pd->last_entry->item;
}
static bool dict_iterator_valid(iterator *it) {
    dict_iterator_private_data *pd = (dict_iterator_private_data *)it->private_data;
    return pd->last_entry != NULL;
}
static void *dict_iterator_next(iterator *it) {
    dict_iterator_private_data *pd = (dict_iterator_private_data *)it->private_data;
    if (pd->last_entry->next != NULL) { // next in chain
        pd->last_entry = pd->last_entry->next;
    } else { // next slot in array
        pd->last_slot += 1;
        while (pd->last_slot < pd->dict->capacity && pd->dict->entries_array[pd->last_slot] == NULL)
            pd->last_slot += 1;
        
        pd->last_entry = (pd->last_slot >= pd->dict->capacity) ? NULL :
            pd->dict->entries_array[pd->last_slot];
    }
    return pd->last_entry == NULL ? NULL : pd->last_entry->item;
}
iterator *dict_iterator(dict *d) {
    dict_iterator_private_data *pd = malloc(sizeof(dict_iterator_private_data));
    pd->dict = d;
    pd->last_slot = -1;
    pd->last_entry = NULL;
    iterator *it = malloc(sizeof(iterator));
    it->reset = dict_iterator_reset;
    it->valid = dict_iterator_valid;
    it->next = dict_iterator_next;
    it->private_data = pd;
    return it;
}


bool dicts_are_equal(dict *a, dict *b) {
    if (a == NULL && b == NULL)
        return true;
    if ((a == NULL && b != NULL) || (a != NULL && b == NULL))
        return false;
    if (a == b)
        return true;
    
    if (a->count != b->count)
        return false;

    // we should walk now, but I'm tired for today..
    // TODO: implement this comparison.

    return true;
}

const char *dict_to_string(dict *l, const char *key_value_separator, const char *entries_separator) {
    strbuff *sb = new_strbuff();
    // we should also walk and I'm tired...
    // TODO: implement this walking.
    return strbuff_charptr(sb);
}




STRONGLY_TYPED_FAILABLE_IMPLEMENTATION(dict);
