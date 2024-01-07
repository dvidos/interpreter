Utilities for C programs:

# Error control

In C there are various ways for error control, when calling functions:

* returning true for success, false for failure
* return int, with >=0 for success, <0 for failure.
* get the value from a pointer, get success/failure in returned value
* use setjmp()/longjmp(), exactly as exceptions are defined.

## Failable

Inspired from how Go language is doing it, i wanted to return both the success/failure flag,
and the resulting value. In C one can return full struct contents, so I took advantage of this:

```c
struct failable {
    bool failed;
    const char *err_msg;
    void *result;
};
failable succeeded(void *result) {
    failable f = { false, NULL, result };
    return f;
}
failable failed(char *err_msg_fmt, ...) {
    vsprintf(err_msg ... /* based on format and var args */);
    failable f = { true, err_msg, NULL };
    return f;
}
```

One can declare a function as returning the `failable` structure (not the pointer)
then check the value before acting on the result. In chains of 

```c
failable calculate_something() {
    return succeeded(ptr);
    // -- or --
    return failed("could not understand option %d", index);
}
```

When calling that function, it's really easy to check, propagate the error, or continue:

```c
failable calculation = calculate_something();
if (calculation.failed)
    return failed("calculation failed: %s", calculation.err_msg);
thing_needed = calculation.result;
````


# Containers

I have found some basic containers invaluable, especially a list. These are generic, 
in the sense that they store pointers to anything `void *`. 
Each provides an iterator, along with extra functionality for containables.

## List

Expandable list of items. Implemented using a single linked list for now. 
Finding an item in O(n). Functions include:

```c
list *l = new_list();
bool list_is_empty(list *l);
int list_length(list *l);
void list_add(list *l);
void *list_get(list *lint index);
void list_set(list *lint index, void *item);
```

And then the extra functionality:

```c
bool lists_are_equal(list *a, list *b);
const char *list_to_string(list *l);
iterator *list_iterator(list *l);
```

## Dictionary

Expandable hashtable of items, based on a `char *` key. Implemented using an array and small linked lists for items with the same hash result. Finding an item in O(1). Functions include:

```c
dict *d = new_dict(int capacity);
bool dict_is_empty(dict *d);
int dict_count(dict *d);
bool dict_has(dict *d, const char *key);
void *dict_get(dict *d, const char *key);
void dict_set(dict *d, const char *key, void *item);
```

And then the extra functionality:

```c
bool dicts_are_equal(dict *a, dict *b);
const char *dict_to_string(dict *l, const char *key_value_separator, const char *entries_separator);
iterator *dict_iterator(dict *d);
```

## Stack

A simple dynamic stack, using a single linked list same as the list. Functions:

```c
stack *s = new_stack();
bool stack_empty(stack *s);
void stack_push(stack *s, void *item);
void *stack_peek(stack *s);
void *stack_pop(stack *s);
```

Extra functionality here as well:

```c
iterator *stack_iterator(stack *s);
const char *stack_to_string(stack *s, const char *separator);
// stacks_are_equal() not implemented yet
```

## Iterator

In order to have a stream of tokens (or other items), an iterator allows 
to traverse a container, without knowing it's internals. It also is faster
than traversing a list using an index, as the access time n grows for larger vlaues of n.

```c
struct iterator {
    void *(*reset)(iterator *it); // for: reset to first and return that first item
    bool (*valid)(iterator *it);  // for: was last reset()/next() value valid?
    void *(*next)(iterator *it);  // for: advance to next and get that next item
    void *(*curr)(iterator *it);  // return the current (last returned) item
    void *(*peek)(iterator *it);  // return the next, without advancing to it.
    void *private_data;
};
```

To traverse an iterator, and with strongly typed items, use the `for_iterator` macro:

```c
for_iterator(iterator_var, item_type, item_var) {
    // do something with item_var
}
```

To get an iterator, use the following functions:

```c
iterator *list_iterator(list *l);
iterator *dict_iterator(dict *d);
iterator *stack_iterator(stack *s);
```



## Containable

Containable is a way to unlock polymorphic behavior for contained items, without the 
containers knowing what they are containing. This allows having `compare` and `to_string` 
functionality, without knowing the item contents.

For an item to be considered containable, the first item in the structure must be a pointer
to a `containable` structure, one with a magic number for verification. 
To make things easy, the following functions are all is needed:

```c
// in the factory function of an item
containable *new_containable(const char *struct_name, 
    bool (*are_equal_func)(void *pointer_a, void *pointer_b), 
    const char *(*to_string_func)(void *pointer)
);

// in the containers, to perform functionality
bool is_containable_instance(void *pointer);
bool containables_are_equal(void *pointer_a, void *pointer_b);
const char *containable_to_string(void *pointer);
```

# String working

Similar to `StringBuilder`, found in other languages, this struct allows
for adding to a string, without caring about buffer size: it auto expands
to always have sufficient capacity. Functions include:

```c
strbuff *new_strbuff();
void strbuff_cat(strbuff *s, const char *str);
void strbuff_catc(strbuff *s, char chr);
void strbuff_catf(strbuff *s, char *fmt, ...);

const char *strbuff_charptr(strbuff *s); // get a strz pointer
```
