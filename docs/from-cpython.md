# python notes

Based on CPython's `PyObject` information ([link](https://github.com/python/cpython/blob/d93605de7232da5e6a182fd1d5c220639e900159/Include/object.h#L122))

The subclassing is done by having a base struct, the contents
defined by a macro, and including (declaring) those contents 
of the base class as the first entry in each subclass structure.

Each object instance has a pointer to the object _type_ and a refcount;
Each type (e.g. str, int, list, dict) are also objects,
that contain poiners to their type, which is the `type` type object.
That type object's type is pointing to itself.

The type info (what we call `class` in this repo)
may have uniform methods (e.g. initialize, hash, to_string, get_iterator, 
destructor, rich_comparator, etc)
It also has has_attr/get_attr/set_attr etc,
for getting and setting other objects, based on name.

> I think what CPython calls `PyObject`, is what we call a `variant`

For example, the way CPython does objects:

```c
#define BASE_STATEMENT_MEMBERS   object *class; \
                                 int type
struct statement { // base class
    BASE_STATEMENT_MEMBERS;
};
struct if_statement { // child class
    BASE_STATEMENT_MEMBERS;
    object *condition;
    object *true_statements;
    object *false_statements;
};
struct while_statement { // another child class
    BASE_STATEMENT_MEMBERS;
    object *condition;
    object *loop_statements;
};
```

