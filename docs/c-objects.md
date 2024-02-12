# c objects

C usually has this all-or-nothing in terms of visibility,
i.e. either have something visible inside a file, or globally.
I am missing the more flexible visibility model that OOP provides.

Therefore, i am trying to find various techniques to improve
the code organization in a C project, described below.

Let's assume an entity named `asset` here.

## strongly typedef'd names

By doing `typedef struct x x`, we make pointers to `x` visible,
and can have all the benefits of validations when we accept or 
returns pointers to this structure. 

Careful when converting to/from `void *`, as there is no warning.

## fixed factory functions (constructors)

Each structure type (object? class?) has one constructor
named `new_x()`. This naming schema helps with intellisense.

More constructors can be built, e.g.

* `new_list()`
* `new_list_from_array(...)`

## pointer to vtable

Good for namespace, does not polute the public namespace with functions.

Allows for polymorphism, e.g. in iterators, we can hide implementation
for traversing an array, a binary tree, or a hashtable.

## pointer to class information

Good for containers, allows them to compare, sort, hash etc
their contents, while staying generic (e.g. store `void *` items)

The convention is that the first element of a structure is either a pointer
to a class struct instance, or a pointer to a vtable, whose first member
is a pointer to a class struct instance.

```c
typedef struct class {
    const char *name;
    int (*hash)(void *obj);
    bool (*equals)(void *a, void *b);
    void (*to_debug_str)(void *obj, string_builder *sb);
    // also possible: clone, compare, serialize, unserialize, destruct etc
} class;
```

### having getters vs open visible structure

We are debating if getters are better, or whether 
the structure should be open for anyone to use.

Open structure is faster to access, but makes for tighter coupling
between modules of code. Getters are more safe, but too verbose.

```c
struct item {
    bool is_valid;
};

// -- vs --

struct item {
    struct item_vtable {
        bool (*is_valid)(item *i);
    } *vtable;
};

// -- vs --

bool item_is_valid(item *i);
```

### polymorphism

We often have to treat things in a polymorphic way.
In this project, a statement can be a simple `continue` statement,
but it can also be a function declaration, with all the metadata
it contains, along with the statements of its body.

Usually implemented using an enum and a union. 
Unfortunately, in C enum values are global, 
so prefixes are needed to avoid collision with other enum values.

```c
typedef struct expression {
    enum expr_type { ET_BINARY, ET_FUNC_CALL } type;
    union {
        struct {
            operand *op1;
            operand *op2;
        } binary;
        struct {
            operand *func;
            list *arguments;
        } func_call;
    };
} expression;
```

### class info and failable

For each "object" we create in this project,
we also define two important things:

1. Its "class info", i.e. a structure that defines 
how containers can work with such an item, and

2. A strongly-typed failable structure and methods,
to allow us to build functions that may return a pointer
to the structure, but they can also fail, hence carry an error message.

## reference

Bringing it all together, assuming an entity of person, 
we'd have the following:

**person.h**

```c
// type definitions
typedef struct person person;
extern class *person_class;

// constructors
person *new_person();
person *new_person_with_args(...);

// other possible methods, outside of the vtable
bool person_is_working(person *p);
const char *person_get_name(person *p);
int person_get_age(person *p);

// strongly typed pair of success flag and return value
typedef struct { bool failed; person *result; char *err_msg } failable_person;
failable_person ok_person(person *p);
failable_person failed_person(const char *err_fmt, ...);
```

**person.c**

```c
// constructors
person *new_person() { ... }
person *new_person_with_args(...) { ... }

// static functions, for class info and vtable
static int person_hash(person *p) { ... }
static bool person_equals(person *a, person *b) { ... }
static void person_to_debug_str(person *a, string_builder *sb) { ... }

// class info, possible vtable as well
class *person_class = &(class){
    .name = "person",
    .hash = person_hash,
    .equals = person_equals,
    .to_debug_str = person_to_debug_str,
};

// strongly typed pair of success flag and return value
failable_person ok_person(person *p) { ... }
failable_person failed_person(const char *err_fmt, ...) { ... }
```
