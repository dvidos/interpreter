# runtime

More than just the exercise of parsing the code into an Abstract Syntax Tree,
the runtime environment upon which the execution is taking place is important.

## variants

Everything is a variant in our world. A variant can a base type or a composition of the following types:

* boolean
* int
* float
* string
* list
* dictionary
* callable

Each variant can optionally support the following operiations:

* initialization
* copy initialization (clone)
* string representation
* equality check
* hash value generation
* sortable comparison
* get or set an element (e.g. variant is a container)
* iterator generation (and iterators support getting the next item)
* call placement (e.g. variant is a callable closure)

Additionally, the variants support the following object-oriented features:

* attributes, existence check, get value, set value
* methods, existence check, place call

## call targets

We have the following target types for a call:

* built-in function call, e.g. `log()`
* built-in method call, e.g. `list.filter()`
* user-defined, a named, declared function call
* user-defined, an anonymous, expression function call
* user-defined, class method function call on a user class instance

## calls crossing worlds

Given that we have both user functions in AST nodes, and built-in functions (e.g. `list_filter()`), we have the following possible call patterns:

* user code calls user callable
* user code calls built-in function (e.g. `list.filter()`)
* built-in code calls user code (e.g. filter condition function)
* built-in code calls built-in code

## callables

In any case, the `callable` is a first class citizen in our scripts,
that is allowed to be stored, passed and returned to functions etc.

A callable is essentially the address of a call handler, along
with various data, including:

At callable **creation time**, the following are provided:

* which C function pointer to call (built-in method or an AST call handler),
* payload to pass in (e.g. the AST node, statement or expression)
* optional captured variables, in cases of closures
* optional function owner, the value of `this` in the execution

At **call time**, the following are provided:

* arguments to pass to the function
* origin of the call (filename, line, column)
* optional function owner binding, the value of `this` in the execution

The reason for the `this` value having this duality is twofold: 

* methods are defined at class level, while the method calls are related to specific instances (hence other pointers)
* we may promote a method to a callable, but binding it to a specific
`this` instance, then pass it around.


## calls pattern

At the minimum, each call has the following args and return value:

```c
execution_outcome some_function(exec_context *c);
```

This pattern has to be propagated and observed everywhere. The reasons are:

* `execution_outcome` supports error reporting and exception throwing, as well as returning a variant value.
  * we can denote interpreter bug detection or failure
  * we can denote exception thrown in the interpreted code
  * we can denote success, and (optional) result returned to caller
* `exec_context` represents interpreter state. It allows
  * to resolve or update symbol values
  * to make further calls to other functions


