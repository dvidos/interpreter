#ifndef _ITERATOR_H
#define _ITERATOR_H

typedef struct iterator iterator;

struct iterator {
    void *(*reset)(iterator *it);
    bool (*valid)(iterator *it);
    void *(*next)(iterator *it);
    void *private_data;
};


// iterator *it = list_iterator();
// for (token *tk = it->reset(it); it->valid(it); tk = it->next(it))

#define for_iterator(iter_var, item_type, item_var)   \
    for (item_type *item_var = iter_var->reset(iter_var); \
         iter_var->valid(iter_var); \
         item_var = iter_var->next(iter_var) \
    )

#define for_list(list_var, iter_var, item_type, item_var)  \
    iterator *iter_var = list_iterator(list_var); \
    for_iterator(iter_var, item_type, item_var)

#define for_dict(dict_var, iter_var, item_type, item_var)  \
    iterator *iter_var = dict_iterator(dict_var); \
    for_iterator(iter_var, item_type, item_var)

#endif
