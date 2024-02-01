#ifndef _EXPRESSION_TYPE_H
#define _EXPRESSION_TYPE_H

typedef enum expression_type {
    ET_IDENTIFIER,
    ET_NUMERIC_LITERAL,
    ET_STRING_LITERAL,
    ET_BOOLEAN_LITERAL,
    ET_UNARY_OP,   // e.g. a + b
    ET_BINARY_OP,  // e.g. !validated
    ET_LIST_DATA,  // e.g. fruits = ['apple', 'orange'];
    ET_DICT_DATA,  // e.g. person = { name:"John", age:99 };
    ET_FUNC_DECL,  // e.g. person.fullname = function() { return this.first + " " + this.last; }
} expression_type;


#endif
