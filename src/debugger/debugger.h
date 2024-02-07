#ifndef _DEBUGGER_H
#define _DEBUGGER_H


enum debugger_exit_mode {
    DEM_SINGLE_STEP,
    DEM_NEXT_LINE,
    DEM_CONTINUE,
    DEM_QUIT,
};

void run_debugger();


#endif
