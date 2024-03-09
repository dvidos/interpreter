#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "../utils/str.h"
#include "../utils/str_builder.h"
#include "../utils/data_types/_module.h"
#include "dict.h"
#include "list.h"

typedef struct dict_entry {
    const char *key;
    void *item;
    struct dict_entry *next;
} dict_entry;

typedef struct dict {
    item_info *item_info;
    dict_entry **entries_array;
    int capacity;
    int count;
} dict;

dict *new_dict(item_info *item_info) {
    dict *d = malloc(sizeof(dict));
    d->item_info = dict_item_info;
    d->capacity = 32;
    d->count = 0;
    d->entries_array = malloc(sizeof(dict_entry *) * d->capacity);
    memset(d->entries_array, 0, d->capacity * sizeof(dict_entry *));
    d->item_info = item_info;
    return d;
}

dict *dict_of(item_info *item_info, int pairs_count, ...) {
    dict *d = new_dict(item_info);
    va_list args;
    va_start(args, pairs_count);
    while (pairs_count-- > 0) {
        const char *key = va_arg(args, const char *);
        void *item = va_arg(args, void *);
        dict_set(d, key, item);
    }
    va_end(args);
    return d;
}

static unsigned hash(const char *str) {
    unsigned result = 5381;
    while (*str) {
        // result = (result * 33) + char
        result = ((result << 5) + result) + *str++;
    }
    return result;
}

void dict_set(dict *d, const char *key, void *item) {
    dict_entry *entry = malloc(sizeof(dict_entry));
    char *p = malloc(strlen(key) + 1);
    strcpy(p, key);
    entry->key = p;
    entry->item = item;
    entry->next = NULL;

    unsigned slot = hash(key) % (unsigned)d->capacity;
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

bool dict_del(dict *d, const char *key) {
    if (!dict_has(d, key))
        return false;
    int slot = hash(key) % d->capacity;

    // if it's the only one in chain..
    if (strcmp(d->entries_array[slot]->key, key) == 0 && d->entries_array[slot]->next == NULL) {
        d->entries_array[slot] = NULL;
        d->count -= 1;
        return true;
    }

    // else find trailing node to reset theh "next" pointer
    dict_entry *trailing = d->entries_array[slot];
    while (trailing->next != NULL) {
        if (strcmp(trailing->next->key, key) == 0) {
            trailing->next = trailing->next->next;
            d->count -= 1;
            return true;
        }
        trailing = trailing->next;
    }
    
    return false;
}

int dict_count(dict *d) {
    return d->count;
}

bool dict_is_empty(dict *d) {
    return d->count == 0;
}


typedef struct dict_keys_iterator_private_data {
    dict *dict;
    int curr_slot;
    dict_entry *curr_entry;
} dict_keys_iterator_private_data;

static void dict_keys_iterator_find_next_entry(dict_keys_iterator_private_data *pd, int *slot, dict_entry **entry) {
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
static void *dict_keys_iterator_reset(iterator *it) {
    dict_keys_iterator_private_data *pd = (dict_keys_iterator_private_data *)it->private_data;
    pd->curr_slot = -1;
    pd->curr_entry = NULL;
    dict_keys_iterator_find_next_entry(pd, &pd->curr_slot, &pd->curr_entry);
    return pd->curr_entry == NULL ? NULL : (char *)pd->curr_entry->key; // we lose const here
}
static bool dict_keys_iterator_valid(iterator *it) {
    dict_keys_iterator_private_data *pd = (dict_keys_iterator_private_data *)it->private_data;
    return pd->curr_entry != NULL;
}
static void *dict_keys_iterator_next(iterator *it) {
    dict_keys_iterator_private_data *pd = (dict_keys_iterator_private_data *)it->private_data;
    if (pd->curr_slot == -1 || pd->curr_slot >= pd->dict->capacity)
        return NULL;
    dict_keys_iterator_find_next_entry(pd, &pd->curr_slot, &pd->curr_entry);
    return pd->curr_entry == NULL ? NULL : (char *)pd->curr_entry->key; // we lose const here
}
static void *dict_keys_iterator_curr(iterator *it) {
    dict_keys_iterator_private_data *pd = (dict_keys_iterator_private_data *)it->private_data;
    return pd->curr_entry == NULL ? NULL : (char *)pd->curr_entry->key; // we lose const here
}
static void *dict_keys_iterator_peek(iterator *it) {
    dict_keys_iterator_private_data *pd = (dict_keys_iterator_private_data *)it->private_data;
    if (pd->curr_slot == -1 || pd->curr_slot >= pd->dict->capacity)
        return NULL;
    int slot = pd->curr_slot;
    dict_entry *entry = pd->curr_entry;
    dict_keys_iterator_find_next_entry(pd, &slot, &entry);
    return entry == NULL ? NULL : (char *)entry->key; // we lose const here
}
iterator *dict_keys_iterator(dict *d) {
    dict_keys_iterator_private_data *pd = malloc(sizeof(dict_keys_iterator_private_data));
    pd->dict = d;
    pd->curr_slot = -1;
    pd->curr_entry = NULL;
    iterator *it = malloc(sizeof(iterator));
    it->reset = dict_keys_iterator_reset;
    it->valid = dict_keys_iterator_valid;
    it->next = dict_keys_iterator_next;
    it->curr = dict_keys_iterator_curr;
    it->peek = dict_keys_iterator_peek;
    it->private_data = pd;
    return it;
}

list *dict_get_keys(dict *d) {
    list *keys = new_list(str_item_info);
    iterator *it = dict_keys_iterator(d);
    for_iterator(it, str, key)
        list_add(keys, (void *)key); // we lose 'const' here
    return keys;
}

list *dict_get_values(dict *d) {
    list *values = new_list(d->item_info);
    iterator *it = dict_keys_iterator(d);
    for_iterator(it, str, key)
        list_add(values, dict_get(d, key));
    return values;
}

bool dicts_are_equal(dict *a, dict *b) {
    if (a == NULL && b == NULL)
        return true;
    if ((a == NULL && b != NULL) || (a != NULL && b == NULL))
        return false;
    if (a == b)
        return true;
    
    if (!item_infos_are_equal(a->item_info, b->item_info))
        return false;
    if (a->count != b->count)
        return false;

    iterator *it_a = dict_keys_iterator(a);
    iterator *it_b = dict_keys_iterator(b);
    const char *key_a = it_a->reset(it_a);
    const char *key_b = it_b->reset(it_b);
    while (it_a->valid(it_a)) {
        if (!it_b->valid(it_b)) return false;
        if (strcmp(key_a, key_b) != 0) return false;
        
        void *value_a = dict_get(a, key_a);
        void *value_b = dict_get(b, key_b);
        bool values_equal;
        if (a->item_info != NULL && a->item_info->are_equal != NULL)
            values_equal = a->item_info->are_equal(value_a, value_b);
        else
            values_equal = value_a == value_b;
        if (!values_equal)
            return false;

        key_a = it_a->next(it_a);
        key_b = it_b->next(it_b);
    }

    return true;
}

const void dict_describe(dict *d, const char *key_value_separator, const char *entries_separator, str_builder *sb) {
    iterator *it = dict_keys_iterator(d);
    bool first = true;
    for_iterator(it, const_char, key) {
        str_builder_add(sb, first ? "" : entries_separator);
        first = false;

        str_builder_addf(sb, "%s%s", key, key_value_separator);
        void *value = dict_get(d, key);
        if (d->item_info != NULL && d->item_info->describe != NULL)
            d->item_info->describe(value, sb);
        else
            str_builder_addf(sb, "@0x%p", value);
    }
}

static const void dict_describe_default(dict *d, str_builder *sb) {
    dict_describe(d, ": ", ", ", sb);
}

void dict_free(dict *d) {
    for (int i = 0; i < d->capacity; i++) {
        dict_entry *e = d->entries_array[i];
        dict_entry *next;
        while (e != NULL) {
            next = e->next;
            free((void *)e->key);
            free(e);
            e = next;
        }
    }
    free(d->entries_array);
    free(d);
}



item_info *dict_item_info = &(item_info) {
    .item_info_magic = ITEM_INFO_MAGIC,
    .type_name = "dict",
    .are_equal = (items_equal_func)dicts_are_equal,
    .describe = (describe_item_func)dict_describe_default
};

STRONGLY_TYPED_FAILABLE_PTR_IMPLEMENTATION(dict);
