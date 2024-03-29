#ifndef _STATEMENT_TYPE_H
#define _STATEMENT_TYPE_H

typedef enum statement_type {
    ST_EXPRESSION,
    ST_IF,
    ST_WHILE,
    ST_FOR_LOOP,
    ST_CONTINUE,
    ST_BREAK,
    ST_RETURN,
    ST_FUNCTION,
    ST_TRY_CATCH,
    ST_THROW,
    ST_BREAKPOINT,
    ST_CLASS,
} statement_type;


#endif
