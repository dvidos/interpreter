# interpreter

It started as a simple mental exercise, to see if we could parse and calculate
an expression in the style of `1+2*3+4`, or `8-4-2`.

The initial idea was either to execute an Excel style formula,
or apply a calculation to big data, or something.
It seems to be evolving to a full blown interpreter of a C-like language,
with built-in lists, dictionaries and simple objects.

## work description

Process for reading the script into an Abstract Syntax Tree:

* A **lexer** is converting characters into tokens. We create a trie for fast lookup.
* A **statement parser** is parsing statements (e.g. if, while, for, etc)
* An **expression parser** parses expressions (e.g. a=1, func() etc)

Process for executing the Absract Syntax Tree:

* A **statement executor** is executing the statements (blocks, loops, break, continue, return)
* An **expression executor** is executing the expressions (assignments, math, comparisons, function calls)
* In order to load and save values of variables we use a **symbol table**. One is created for every function we enter. If the function was anonymous member of a dictionary, the `this` symbol points to that dictionary. This emulates objects, similar to javascript.
* There are three types of functions supported:
  * **built in** functions: strpos(), strlen() etc.
  * **user defined** functions, as a statement, at a script body
  * **anonymous functions**, as an expression operand

What becomes evident is:

* parsing statements and expressions works in recursive descend (one can call the other)
* executing statements and expressions works in recursive descend too.

## entities (objects? classes?) in the code

Objects for reading and executing the scipt / code

* `token` - a semantic item in the scipt: PLUS, NUMBER, BREAK_KEYWORD etc.
* `operator` - operation of an expression: ADD, XOR, FUNC_CALL etc
* `expression` - unary or binary, with an operator and subexpression(s) as operand(s)
* `statement` - a flow control thing: IF, WHILE, BREAK, RETURN etc.
* `symbol_table` - represents the scope of a function, in a stack format
* `exec_context` - runtime environment a function runs under. Includes the symbol table.

General support objects

* `variant` - immutable values of various types: str, int, callable etc.
* `callable` - a function that can be called with args to return a variant
* `list` - a list of items, accessed by index
* `dict` - a hash map of items, accessed by name
* `stack` - a stack of items, with push, peek & pop operations
* `queue` - a queue of items, with put, peek & get operations
* `item_class` - item information for containers. Facilitates comparison and debugging
* `str_builder` - an expandable zero-terminated string buffer

## memory 

As this program was supposed to be short lived and execute and exit,
no attempt was made to free any allocated memory. 

---

# a few conventions

Here's a few things we applied in the project. 
Mainly driven out of the nature of the C language.

### file organization

Each C file is accompanied by a header file, which exposes only the external
functionality. Things are hidden (e.g. declared `static`) as much as possible.
Modules of closely related files are clustered in a directory (e.g. `token.c`
and `tokenization.c` in the `lexer` folder).

The idea of a directory-level header file, exposing the module functionality,
akin to Python's `__init__.py` or to Go's `go.mod` files, was entertained.

### structures and functions

Most "objects" have a declared public pointer without body (e.g. `typedef struct token token;`)
This allows outsiders to use the pointer, without knowing the innards.
For publicly available properties, getters are created (e.g. `token_get_type();`)

Most functions related to the object start with the object type and take the object
as a first argument. e.g. `list_add()`, `list_get()`. 
If polymorphism is neded (as in the iterator), the object exposes 
pointers to functions, who take the object as the first argument. e.g.
`iter->next(iter);` and `iter->valid(iter)`. 

### contained items info

In order to contain the items in the various containers, the convention is that the `class` struct is exposed in a public variable with the name `[type]_class`. For example `token_class`. This creates the syntactic sugar: `list *l = new_list(token_class);`.

Unit tests rely heavily on comparing a computed data structure against an expected
data structure. Also, for debugging, it is very helpful to be able to print 
the values in a container (e.g. a stack).

### returning status & values

In order to return failure status and results at the same time, the `failable`
mechanism was created. This creates strongly typed structures that contain
both the returned type (be it `bool`, `int` or pointer to something), and 
failure flag and information. By convention, these are called `failable_[type]`
and the two methods for returning are called `ok_[type]` and `failed_[type]`.
For example:

* `failable_variant` - is the type returned by functions
* `ok_variant(variant *v);` - for returning success with a value
* `failed_variant(err_msg);` - for returning failure information.

## Error control

In C there are various ways for error control, when calling functions:

* returning true for success, false for failure
* return int, with >=0 for success, <0 for failure.
* get the value from a pointer, get success/failure in returned value
* use setjmp()/longjmp(), exactly as exceptions are defined.

### Failable

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
    return failed(NULL, "could not understand option %d", index);
}
```

When calling that function, it's really easy to check, propagate the error, or continue:

```c
failable calculation = calculate_something();
if (calculation.failed)
    return failed(NULL, "calculation failed: %s", calculation.err_msg);
thing_needed = calculation.result;
````

The module is refined now, and results in highly informative error messages:

```bash
Executing script3.scr...
    interpret_and_execute() -> "Tokenization failed", at interpreter.c:33
    parse_code_into_tokens() -> "Cannot get token", at tokenization.c:233
    get_token_at_code_position() -> "Unrecognized character '@' at script3.scr:27:1", at tokenization.c:218
```

## Containers

I have found some basic containers invaluable, especially a list. These are generic, 
in the sense that they store pointers to anything `void *`. 
Each provides an iterator, along with extra functionality for containables.

### List

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
const char *list_describe(list *l);
iterator *list_iterator(list *l);
```

### Dictionary

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
const char *dict_describe(dict *l, const char *key_value_separator, const char *entries_separator);
iterator *dict_iterator(dict *d);
```

### Stack

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
const char *stack_describe(stack *s, const char *separator);
// stacks_are_equal() not implemented yet
```

### Iterator

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

### Contained item

Contained_item is a way to unlock polymorphic behavior for contained items, without the 
containers knowing what they are containing. This allows having `are_equal` and `describe` 
functionality, without knowing the item contents.

Each entity (e.g. token, expression etc) are declaring a global pointer to such a struct,
holding the relevant functions. By convention, these are usually named `containing_####`,
where #### is the name of the entity. This creates the syntactic sugar below:

```
list *l = new_list(having_tokens);
dict *d = new_dict(having_operators);
```

## Variant

To emulate the ability to convey a value, the `variant` is provided.
It allows to store ints, floats, boolean literals, strings, lists and dictionaries.
It enables json format manipulation. It supports containable and can be contained
in the containers. The values of a variant are immutable.

To create a new variant:

```c
variant *new_variant();
variant *new_bool_variant(bool b);
variant *new_int_variant(int i);
variant *new_float_variant(float f);
variant *new_str_variant(const char *p);
variant *new_list_variant(list *l);
variant *new_dict_variant(dict *d);
variant *new_callable_variant(callable *c);
```

To verify the encapsulated value type:

```c
bool variant_is_null(variant *v);
bool variant_is_bool(variant *v);
bool variant_is_int(variant *v);
bool value_is_float(variant *v);
bool value_is_str(variant *v);
bool variant_is_list(variant *v);
bool variant_is_dict(variant *v);
bool variant_is_callable(variant *v);
```

To retrieve the encapsulated value, with auto conversion, if the requested type 
is not the same as the encapsulated type:

```c
bool variant_as_bool(variant *v);
int variant_as_int(variant *v);
float variant_as_float(variant *v);
const char *variant_as_str(variant *v);
list *variant_as_list(variant *v);
dict *variant_as_dict(variant *v);
callable *variant_as_callable(variant *v);
```

## String working

Similar to `StringBuilder`, found in other languages, this struct allows
for adding to a string, without caring about buffer size: it auto expands
to always have sufficient capacity. Functions include:

```c
str_builder *new_str_builder();
void str_builder_cat(str_builder *sb, const char *str);
void str_builder_catc(str_builder *sb, char chr);
void str_builder_catf(str_builder *sb, char *fmt, ...);

const char *str_builder_charptr(str_builder *sb); // get a strz pointer
```
