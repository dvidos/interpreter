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
        while (e != NULL) {
            if (strcmp(e->key, key) == 0) {
                e->item = item;
                return;
            }
            if (e->next == NULL) {
                e->next = entry;
                d->count += 1;
                return;
            }
            e = e->next;
        }
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
    int curr_slot;
    dict_entry *curr_entry;
} dict_iterator_private_data;

static void dict_iterator_find_next_entry(dict_iterator_private_data *pd, int *slot, dict_entry **entry) {
    if (*slot != -1 && *entry != NULL && (*entry)->next != NULL) {
        // only if already in some slot and entry
        *entry = (*entry)->next;
    } else {
        // we need to move to a new slot, possibly the first
        *slot += 1;
        while (*slot < pd->dict->capacity && pd->dict->entries_array[*slot] == NULL)
            *slot += 1;
        *entry = *slot >= pd->dict->capacity ? NULL : pd->dict->entries_array[*slot];
    }
}
static void *dict_iterator_reset(iterator *it) {
    dict_iterator_private_data *pd = (dict_iterator_private_data *)it->private_data;
    pd->curr_slot = -1;
    pd->curr_entry = NULL;
    dict_iterator_find_next_entry(pd, &pd->curr_slot, &pd->curr_entry);
    return pd->curr_entry == NULL ? NULL : pd->curr_entry->item;
}
static bool dict_iterator_valid(iterator *it) {
    dict_iterator_private_data *pd = (dict_iterator_private_data *)it->private_data;
    return pd->curr_entry != NULL;
}
static void *dict_iterator_next(iterator *it) {
    dict_iterator_private_data *pd = (dict_iterator_private_data *)it->private_data;
    if (pd->curr_slot == -1 || pd->curr_slot >= pd->dict->capacity)
        return NULL;
    dict_iterator_find_next_entry(pd, &pd->curr_slot, &pd->curr_entry);
    return pd->curr_entry == NULL ? NULL : pd->curr_entry->item;
}
static void *dict_iterator_curr(iterator *it) {
    dict_iterator_private_data *pd = (dict_iterator_private_data *)it->private_data;
    return pd->curr_entry == NULL ? NULL : pd->curr_entry->item;
}
static void *dict_iterator_peek(iterator *it) {
    dict_iterator_private_data *pd = (dict_iterator_private_data *)it->private_data;
    if (pd->curr_slot == -1 || pd->curr_slot >= pd->dict->capacity)
        return NULL;
    int slot = pd->curr_slot;
    dict_entry *entry = pd->curr_entry;
    dict_iterator_find_next_entry(pd, &slot, &entry);
    return entry == NULL ? NULL : entry->item;
}
iterator *dict_iterator(dict *d) {
    dict_iterator_private_data *pd = malloc(sizeof(dict_iterator_private_data));
    pd->dict = d;
    pd->curr_slot = -1;
    pd->curr_entry = NULL;
    iterator *it = malloc(sizeof(iterator));
    it->reset = dict_iterator_reset;
    it->valid = dict_iterator_valid;
    it->next = dict_iterator_next;
    it->curr = dict_iterator_curr;
    it->peek = dict_iterator_peek;
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
