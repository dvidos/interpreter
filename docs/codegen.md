# code generation

How about, this interpreter, after parsing the code,
with all the high level abilities that it has,
(e.g. passing or returning anonymous functions to/from functions)
generates valid C code that deals with initialization,
memory management, error handling etc?

Of course, we could already build a ton of things,
on top of the new language, from `"hello %s".format()`,
to `foreach(...)`, or even `with_cleanup(...) { ... }`.

I have to find a way to link special code constructs 
(such as iterating, cleanup, decorators, or error handling) with 
user-created code.

For example, the following not only can free the memory,
but also call any `destruct()` method to close the file.

```
    with_cleanup(f = open("data.txt)) {
        print("{} lines", f.readlines().count())
    }
```

We could also track when a reference is being copied
to other owners and see when it is dropped from the last 
context that uses it, therefore, free the memory there and then.

Generating basic would be fun!!!
