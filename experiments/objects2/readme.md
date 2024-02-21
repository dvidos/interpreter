
# objects2

In this folder we try to make a framework for supporting 
a hierarchy of classes, without having to have strong C typing.

We aim to use them in the interpreter, so they don't have to code in C nicely.
For example, to set a value, we may call a `set_attribute(obj, 'name', value);`.
Therefore, they don't have to have strongly typed attributes, methods, vtables or otherwise.

This is heavily inspired / influenced by the way CPython is implementing 
the internal object hierarchy that is used in the python language.

See https://web.mit.edu/18.417/doc/pydocs/api.pdf

## Framework

The framework consists of the declaration of the following:


| Name        | Description
|-------------|----
| object      | a generic structure that has a type declaration
| type_object | a struct for type declaration instances
| type_type   | an instance of the type_object, for the type of all type declaration instances

Given a `object *` pointer, we can check it's class, see if it is a specific
class, which we can cast it to (e.g. a `str_object *`).


## To declare a class / object type

See list.c for implementation example

Steps:

* Create the `struct` that contains the data of the type.
  * It's declaration **must** start with the `BASE_OBJECT_ATTRIBUTES`, to contain the same members as the object class.
* Create static initializer and a deallocator methods.
* Create methods using the contract: `static object *list_insert(object *self, object *index, object *item)`
* Create a static array of `type_method_definition` structures, to hold method definitions.
  * The last entry of the array must have a NULL name.
* Create a static array of `type_attribute_definition` structures, to hold attribute definitions.
  * Each can contain either a getter and (optional) setter, or an offset and primitive type.
  * The last entry of the array must have a NULL name.
* Create a type instance. Put all the information there:
  * Give the name of the class (e.g. `str`, `list` etc)
  * Give the size to be allocated
  * Give the initialize, destructor and other utility class methods
  * Give the array of methods and array of attributes.

## Dependencies

* base object depends on type declaration
* base object functions depend on err_object for errors
* err object depends on str_object for the error message

