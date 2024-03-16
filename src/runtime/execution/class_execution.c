
#include <string.h>
#include "../../utils/mem.h"
#include "class_execution.h"
#include "statement_execution.h"

/*  we do not create a strongly typed instance structure
    because the size of it depends on the number of attributes
    that a class instance will have.

    it is implied that the instance has the following structure:
        - BASE_VARIANT_MEMBERS (type ptr, ref_count)
        - variant *first_attribute;
        - variant *second_attribute;
        - variant *third_attribute; 
        etc
*/

// special method names
#define CONSTRUCTOR_METHOD_NAME   'constructor'
#define DESTRUCTOR_METHOD_NAME    'destructor'
#define TO_STRING_METHOD_NAME     'toString'
#define HASH_METHOD_NAME          'hash'




static void class_initializer(variant *instance, variant *args_list, variant *named_args) {
    struct statement_class_info *ci = &((statement *)instance->_type->ast_node)->per_type.class;

    // first set all variables to eiter result of expression, or to void variant.
    
    // when we implement inherited classes, maybe call parent constructor?

    // then, if there is an explicit constructor, use it
}

static execution_outcome class_to_string(variant *instance) {
    struct statement_class_info *ci = &((statement *)instance->_type->ast_node)->per_type.class;

    // if there is a specific toString method, call it.
    return ok_outcome(new_str_variant("(%s @ %p)", instance->_type->name, instance));
}

static execution_outcome method_call_handler(variant *this, list *args, dict *named_args, exec_context *ctx) {
    // this handler is called when a method of this class is called.
    // we somehow must find a way to tell which method this was!
    return failed_outcome("Not implemented yet!");
}


static struct variant_attrib_definition *prepare_attrib_definitions(statement *stmt) {
    int len = list_length(stmt->per_type.class.attributes);
    struct variant_attrib_definition *attribs = malloc((len + 1) * sizeof(struct variant_attrib_definition *));

    int index = 0;
    int offset = BASE_VARIANT_FIRST_ATTRIBUTES_SIZE;
    for_list(stmt->per_type.class.attributes, it, class_attribute, ca) {
        attribs[index].name = ca->name;
        attribs[index].vaf_flags = 0;
        if (ca->public)
            attribs[index].vaf_flags += VAF_PUBLIC;
        attribs[index].offset = offset;
        offset += sizeof(variant *);
    }
    attribs[index].name = NULL; // signal the last attribute

    return attribs;
}

static struct variant_method_definition *prepare_method_definitions(statement *stmt) {
    int len = list_length(stmt->per_type.class.methods);
    struct variant_method_definition *methods = malloc((len + 1) * sizeof(struct variant_attrib_definition *));

    int index = 0;
    for_list(stmt->per_type.class.methods, it, class_method, cm) {
        methods[index].name = cm->name;

        methods[index].handler = method_call_handler;
        if (cm->public)
            methods[index].vmf_flags += VMF_PUBLIC;
    }
    methods[index].name = NULL; // signal the last attribute

    return methods;
}

variant_type *class_statement_create_variant_type(statement *stmt) {
    struct statement_class_info *ci = &stmt->per_type.class;

    variant_type *t = malloc(sizeof(variant_type));
    t->_type = type_of_types;
    t->_references_count = 1;
    
    char *p = malloc(strlen(stmt->per_type.class.name) + 1);
    strcpy(p, stmt->per_type.class.name);
    t->name = p;
    t->ast_node = stmt;
    t->instance_size = BASE_VARIANT_FIRST_ATTRIBUTES_SIZE 
            + (list_length(ci->attributes) * sizeof(variant *));
    
    t->attributes = prepare_attrib_definitions(stmt);
    t->methods = prepare_method_definitions(stmt);
    t->initializer = (initialize_func)class_initializer;
    t->stringifier = (stringifier_func)class_to_string;
    
    // find special methods, if any.
    // then...
    // register it as a symbol
    // register it with the exec_context
    // and make a built in "new" function that takes this type as the first argument
    // it should resolve the first argument, find the variant type,
    // check if it's a type_type, and instantiate it, passing the rest of the arguments 
    // to the constructor.

    // then...
    // have all methods work on this class, but calling custom handlers.

    // then...
    // do it all over again with the anonymous expression class!

    return t;
}
