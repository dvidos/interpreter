// experiment to see if uniform objects is meaningful.
// we must distinguish calling inside the C code,
// from calling inside the interpreter.
// method calls are using a name, not an offset.
// objects are 1st class citizens, 
//   used in args, returned from functions, participating in operations, etc
// the below constructs maybe better for the interpreted language.

typedef struct object object; // "everything is an object"

struct object {
    object *type;  // points to the type object. types point to the type-type object, which points to itself.
};

struct string_object {
    object *type;
    const char *ptr;
};

struct int_object {
    object *type;
    int value;
};

struct list_object {
    object *type;
    struct list_node *head;
    struct list_node *tail;
    int length;
};

object *class_class = &(object){
    .type = class_class
    // 10 this represents the type of a class. 
    // 20 a metaclass. 
    // 30 GOTO 10.
};

object *list_class = &(object){
    .type = NULL
    // ???
    // methods for add, get, length, etc? a dictionary of members
    // each method is also a callable object...
};

struct class_object {
    object *type;
    object *(*to_string)(object *o);
    bool (*equals)(object *o);
    int (*hash)(object *o);
    dict *attributes; // has/get/set...attr() ???
}

// but it's good for the interpreted code.
// we cannot use these from inside C code, it's too complex
object *a = new_int_object(1);
object *b = new_int_object(2);
object *add_method = object_get_attr(a, "add");
object *args = new_list_object_with(2, a, b);
object *c = object_make_call(add_method, args);

