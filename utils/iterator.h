#ifndef _ITERATOR_H
#define _ITERATOR_H

typedef struct iterator iterator;

struct iterator {
    void *(*reset)(iterator *it); // for: reset to first and return that first item
    bool (*valid)(iterator *it);  // for: was last reset()/next() value valid?
    void *(*next)(iterator *it);  // for: advance to next and get that next item
    void *(*curr)(iterator *it);  // return the current (last returned) item
    void *(*peek)(iterator *it);  // return the next, without advancing to it.
    void *private_data;
};


// iterator *it = list_iterator();
// for (token *tk = it->reset(it); it->valid(it); tk = it->next(it))

#define for_iterator(iter_var, item_type, item_var)   \
    for (item_type *item_var = iter_var->reset(iter_var); \
         iter_var->valid(iter_var); \
         item_var = iter_var->next(iter_var) \
    )


#endif
