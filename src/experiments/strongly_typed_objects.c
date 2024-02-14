
// if we try for C friendly objects, for our interpreter code,
// we want strongly typed things, but polymorphism for values
// * first item will always be a pointer to class info,
//   to allow operations in containers etc.
// * a strongly typed vtable can be pointed using a same name pointer,
//   in a subsequent member, other members following
// * child classes shall replicate the contents of the base class
//   (including base methods and properties), but class_info
//   will point to the child class. 
//   maybe we must agree 1st pointer -> class
//                       2nd pointer -> vtable

/*
    Hierarchies:

    - variant
      - str_variant
      - int_variant
      - bool_variant
      - list_variant
      - dict_variant
      - callable_variant
    
    - statement
      - if_statement
      - for_statement
      - while_statement
      - break_statement
      - continue_statement
      - func_decl_statement
      - class_decl_statement
      - expression_statement

    - expression
      - identifier_expression
      - num_literal_expression
      - str_literal_expression
      - bool_literal_expression
      - unary_expression
      - binary_expression
      - func_decl_expression
      - class_decl_expression

    #define if_expression_ptr(e)    ((if_expression *)e)
    #define for_expression_ptr(e)   ((for_expression *)e)

    expression *e;
    if_expression_ptr(e)->else;
    for_expression_ptr(e)->next;
*/

typedef struct class_info class_info;
struct class_info { // one instance for each class
    const char *name;
    long magic_number;
    struct class_info *parent_class; // is_subclass_of(instance, class)

    void *(*clone)(void *instance);
    void *(*to_string)(void *instance);
    bool (*equals)(void *instance_a, void *instance_b);
    int (*hash)(void *instance);
    int (*compare)(void *instance_a, void *instance_b);
    void (*destruct)(void *instance);
};

/*

// base class (variant):

#define BASE_VARIANT_MEMBERS  class_info *class; \
                              variant_type type
#define BASE_VARIANT_METHODS  variant_type (*get_type)(variant *v)

typedef struct variant_vtable variant_vtable;
typedef struct variant variant;
struct variant {
    BASE_VARIANT_MEMBERS; // what if this has a vtable????
};
struct variant_vtable {
    BASE_VARIANT_METHODS;
};

// --- str: ---

typedef struct str str;
typedef struct str_vtable str_vtable;

struct str { // one for each instance
    BASE_VARIANT_MEMBERS;
    str_vtable *vtable; // always first member
    const char *ptr;
};
struct str_vtable { // one instance for each polymorphic method set
    BASE_VARIANT_METHODS;
    int (*length)(str *s);
    bool (*starts_with)(str *s, str *chunk);
    str *(*to_upper)(str *s);
};

// --- list: ---

typedef struct list list;
typedef struct list_vtable list_vtable;

struct list {
    BASE_VARIANT_MEMBERS;
    list_vtable *vtable; // always first member
    void *head;
    void *tail;
    int length;
};
struct list_vtable { // one for each class
    BASE_VARIANT_METHODS;
    int (*length)(list *l);
    void (*add)(list *l, void *item);
};
*/


// how to implement polymorphism?
// * same level polymorphism is easy (e.g. an iterator), it's just 
//   one struct with pointers to functions.
// * but having a parent and a child, having diff number of properties & methods
//   is really challenging. mainly child encapsulates the parent data.

typedef struct statement       statement; // common methods, e.g. "token"
typedef struct if_statement    if_statement; // special methods, e.g. "has_else"
typedef struct while_statement while_statement;
typedef struct for_statement   for_statement;

#define if_statement_ptr(s)     ((if_statement *)s)
#define while_statement_ptr(s)  ((while_statement *)s)
#define for_statement_ptr(s)    ((for_statement *)s)

struct statement {
    class_info *class;
    statment_vtable *vtable; // less we agree 2nd argument is always vtable...
    int statement_type; // ? 
};
struct statement_vtable {
    failable execute(statment *s);
};
struct if_statement {
    // we need a strongly typed vtable here...
    if_statement_vtable *vtable; // collides with parent vtable member...
};
struct if_statement_vtable {
    failable execute(if_statment *s);
};
struct while_statement {
    while_statement_vtable *vtable; // breaks polymorphism...
};
struct while_statement_vtable {
    while_statement_vtable *vtable; // breaks polymorphism...
};

void example(statement *s) {
}