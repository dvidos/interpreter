#ifndef _DEBUGGER_H
#define _DEBUGGER_H

#include <stdbool.h>
#include "../entities/statement.h"
#include "../entities/expression.h"
#include "../utils/data_types/exec_context.h"


enum debugger_anticipation_mode {
    DAM_NEXT_STATEMENT_OR_EXPRESSION, // single step
    DAM_NEXT_LINE_NO, // needs a last_line_no
    DAM_RETURN_STATEMENT, // execute till the next return statement
    DAM_QUIT, // quit execution completely
    DAM_NONE, // run completely


};

enum debugger_exit_mode {
    DEM_SINGLE_STEP,
    DEM_NEXT_LINE,
    DEM_CONTINUE,
    DEM_QUIT,
};

bool should_start_debugger(statement *curr_stmt, expression *curr_expr, exec_context *ctx);
failable run_debugger(statement *curr_stmt, expression *curr_expr, exec_context *ctx);


#endif
