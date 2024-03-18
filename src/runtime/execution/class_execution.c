
#include <string.h>
#include "../../utils/mem.h"
#include "class_execution.h"
#include "statement_execution.h"
#include "expression_execution.h"
#include "function_execution.h"

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
#define CONSTRUCTOR_METHOD_NAME   "construct"
#define DESTRUCTOR_METHOD_NAME    "destruct"
#define TO_STRING_METHOD_NAME     "toString"
#define HASH_METHOD_NAME          "hash"


#define ADDRESS_OF_ATTRIBUTE(instance, index)  \
    (variant **)(                              \
        ((void *)instance)                     \
        + BASE_VARIANT_FIRST_ATTRIBUTES_SIZE   \
        + ((index) * sizeof(void *))           \
    )


static execution_outcome instance_initializer(variant *instance, variant *args_list, exec_context *ctx) {
    struct statement_class_info *ci = &((statement *)instance->_type->ast_node)->per_type.class;
    execution_outcome ex;

    // first set all variables to eiter result of expression, or to void variant.
    int index = 0;
    for_list(ci->attributes, it, class_attribute, ca) {
        variant **attr_ptr = ADDRESS_OF_ATTRIBUTE(instance, index);

        if (ca->init_value == NULL) {
            *attr_ptr = void_singleton;
        } else {
            ex = execute_expression(ca->init_value, ctx);
            if (ex.failed || ex.excepted) return ex;
            *attr_ptr = ex.result;
        }
    }
    
    // then, if there is an explicit constructor, use it
    // actually, if constructor was not public, maybe we could forbid instantiation
    if (variant_has_method(instance, CONSTRUCTOR_METHOD_NAME, VIS_SAME_CLASS_CODE)) {
        list *al = args_list == NULL ? NULL : list_variant_as_list(args_list);
        variant_call_method(instance, CONSTRUCTOR_METHOD_NAME, VIS_SAME_CLASS_CODE, al, internal_origin(), ctx);
    }

    return ok_outcome(NULL);
}

static execution_outcome instance_to_string(variant *instance) {
    struct statement_class_info *ci = &((statement *)instance->_type->ast_node)->per_type.class;

    // if there is a specific toString method, call it.
    return ok_outcome(new_str_variant("(%s @ %p)", instance->_type->name, instance));
}

static execution_outcome method_call_handler(variant *this, variant_method_definition *method, list *arg_values, origin *call_origin, exec_context *ctx) {
    // this handler is called when a method of this class is called.
    statement *stmt = (statement *)method->ast_node;

    return execute_user_function(
        method->name,
        stmt->per_type.function.statements,
        stmt->per_type.function.arg_names,
        arg_values,
        this,
        call_origin,
        ctx);
}

static variant_attrib_definition *prepare_attrib_definitions(statement *stmt) {
    int len = list_length(stmt->per_type.class.attributes);
    variant_attrib_definition *attribs = malloc((len + 1) * sizeof(variant_attrib_definition));

    int index = 0;
    int offset = BASE_VARIANT_FIRST_ATTRIBUTES_SIZE;
    for_list(stmt->per_type.class.attributes, it, class_attribute, ca) {
        attribs[index].name = ca->name;
        attribs[index].vaf_flags = 0;
        if (ca->public)
            attribs[index].vaf_flags += VAF_PUBLIC;
        attribs[index].offset = offset;

        offset += sizeof(variant *);
        index++;
    }
    attribs[index].name = NULL; // signal the last attribute

    return attribs;
}

static variant_method_definition *prepare_method_definitions(statement *stmt) {
    int len = list_length(stmt->per_type.class.methods);
    variant_method_definition *methods = malloc((len + 1) * sizeof(variant_attrib_definition));

    int index = 0;
    for_list(stmt->per_type.class.methods, it, class_method, cm) {
        methods[index].name = cm->name;

        methods[index].handler = method_call_handler;
        if (cm->public)
            methods[index].vmf_flags += VMF_PUBLIC;
        methods[index].ast_node = cm->function;

        index++;
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
    t->initializer = (initialize_func)instance_initializer;
    t->stringifier = (stringifier_func)instance_to_string;
    
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
