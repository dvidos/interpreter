
#include <string.h>
#include "../../utils/mem.h"
#include "class_execution.h"
#include "statement_execution.h"
#include "expression_execution.h"
#include "function_execution.h"


// special method names
#define CONSTRUCTOR_METHOD_NAME   "construct"
#define DESTRUCTOR_METHOD_NAME    "destruct"
#define TO_STRING_METHOD_NAME     "toString"
#define HASH_METHOD_NAME          "hash"

typedef struct class_instance {
    BASE_VARIANT_FIRST_ATTRIBUTES;
    variant *first_attribute;
    variant *second_attribute; // given for debugging, not for actual storage
    variant *third_attribute;
} class_instance;


#define INSTANCE_SIZE(attributes_count)  \
        (offsetof(class_instance, first_attribute) + (sizeof(variant *) * (attributes_count)))

#define ATTRIBUTE_OFFSET(attribute_num)  \
        (offsetof(class_instance, first_attribute) + (sizeof(variant *) * (attribute_num)))

#define ATTRIBUTE_ADDRESS(instance, attribute_num)  \
        ((variant **)(((void *)(instance)) + ATTRIBUTE_OFFSET(attribute_num)))

#define GET_ATTRIBUTE(instance, attribute_num)  \
        (*(ATTRIBUTE_ADDRESS((instance), (attribute_num))))

#define SET_ATTRIBUTE(instance, attribute_num, value)  \
        (*(ATTRIBUTE_ADDRESS((instance), (attribute_num)))) = (value)


static execution_outcome instance_initializer(class_instance *instance, variant *args_list, exec_context *ctx) {
    if (instance == NULL)
        return failed_outcome("no instance given");
    
    struct statement_class_info *ci = &((statement *)instance->_type->ast_node)->per_type.class;
    execution_outcome ex;

    // first set all variables to eiter result of expression, or to void variant.
    int index = 0;
    for_list(ci->attributes, it, class_attribute, ca) {
        variant *value = void_singleton;

        if (ca->init_value != NULL) {
            ex = execute_expression(ca->init_value, ctx);
            if (ex.failed || ex.excepted) return ex;
            value = ex.result;
        }

        SET_ATTRIBUTE(instance, index, value);
        index++;
    }
    
    // then, if there is an explicit constructor, use it
    // actually, if constructor was not public, maybe we could forbid instantiation
    if (variant_has_method((variant *)instance, CONSTRUCTOR_METHOD_NAME, VIS_SAME_CLASS_CODE)) {
        list *al = args_list == NULL ? NULL : list_variant_as_list(args_list);
        variant_call_method((variant *)instance, CONSTRUCTOR_METHOD_NAME, VIS_SAME_CLASS_CODE, al, internal_origin(), ctx);
    }

    return ok_outcome(NULL);
}

static execution_outcome instance_to_string(class_instance *instance) {
    if (instance == NULL)
        return failed_outcome("no instance given");

    struct statement_class_info *ci = &((statement *)instance->_type->ast_node)->per_type.class;
    
    // it seems if we want exec_context here, we have to make all variant functions
    // to accept it, and pass it to "variant->stringify"
    // but that means all our unit tests need the context!!!!!
    // or, otherwise, to kick off our unit tests, we need to start an interpreter session.
    // i.e. variants() cannot live outside of an intepreter bubble.
    
    variant *s = NULL;
    // if (variant_has_method(instance, TO_STRING_METHOD_NAME, VIS_SAME_CLASS_CODE)) {
    //     list *args = list_of(variant_item_info, 1, instance);
    //     execution_outcome ex = variant_call_method(
    //         (variant *)instance, 
    //         TO_STRING_METHOD_NAME, 
    //         VIS_SAME_CLASS_CODE, 
    //         args, 
    //         internal_origin(), 
    //         ctx
    //     );
    //     list_free(args);
    //     if (ex.excepted || ex.failed) return ex;
    //     s = ex.result;
    // } else {
        s = new_str_variant("(%s @ %p)", instance->_type->name, instance);
    // }

    return ok_outcome(s);
}

static execution_outcome class_method_call_handler(variant *this, variant_method_definition *method, list *arg_values, origin *call_origin, exec_context *ctx) {
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
    int attribs_count = list_length(stmt->per_type.class.attributes);
    variant_attrib_definition *attribs_table = malloc(sizeof(variant_attrib_definition) * (attribs_count + 1));

    int index = 0;
    for_list(stmt->per_type.class.attributes, it, class_attribute, ca) {
        variant_attrib_definition *attr = &attribs_table[index];

        attr->name = ca->name;
        attr->vaf_flags = 0;
        if (ca->public)
            attr->vaf_flags += VAF_PUBLIC;
        attr->offset = ATTRIBUTE_OFFSET(index);

        index++;
    }

    attribs_table[attribs_count].name = NULL; // signal the last attribute
    return attribs_table;
}

static variant_method_definition *prepare_method_definitions(statement *stmt) {
    int methods_count = list_length(stmt->per_type.class.methods);
    variant_method_definition *methods_table = malloc(sizeof(variant_method_definition) * (methods_count + 1));

    int index = 0;
    for_list(stmt->per_type.class.methods, it, class_method, cm) {
        variant_method_definition *method = &methods_table[index];

        method->name = cm->name;
        method->handler = class_method_call_handler;
        if (cm->public)
            method->vmf_flags += VMF_PUBLIC;
        method->ast_node = cm->function;

        index++;
    }

    methods_table[methods_count].name = NULL; // signal the last method
    return methods_table;
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
    t->instance_size = INSTANCE_SIZE(list_length(ci->attributes));
    t->attributes = prepare_attrib_definitions(stmt);
    t->methods = prepare_method_definitions(stmt);
    t->initializer = (initialize_func)instance_initializer;
    t->stringifier = (stringifier_func)instance_to_string;
    
    return t;
}
