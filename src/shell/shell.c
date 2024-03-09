#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "../interpreter/interpreter.h"
#include "../utils/str_builder.h"

static bool get_command(char *buffer, int buffer_size) {
    // would be nice to have even a single line of history...
    if (fgets(buffer, buffer_size, stdin) == NULL)
        return false;

    if (buffer[strlen(buffer) - 1] == '\n')
        buffer[strlen(buffer) - 1] = 0;        
    
    return true;
}


static void execute_command(const char *code, dict *values, bool verbose, bool enable_debugger) {

    execution_outcome ex = interpret_and_execute(code, "interactive", values, verbose, enable_debugger, false);
    if (ex.failed) {
        printf("Evaluation failed: %s\n", ex.failure_message);

    } else if (ex.excepted) {
        variant *s = variant_to_string(ex.exception_thrown);
        printf("Uncaught exception: %s\n", str_variant_as_str(s));
        variant_drop_ref(s);
    } else {
        variant *s = variant_to_string(ex.result);
        printf("%s\n", str_variant_as_str(s));
        variant_drop_ref(s);
    }
}

void interactive_shell(bool verbose, bool enable_debugger) {
    char buffer[128];
    dict *values = new_dict(variant_item_info);

    printf("Interactive shell. Enter 'q' to quit.\n");
    while (true) {
        fputs("(expression) > ", stdout);
        if (!get_command(buffer, sizeof(buffer)))
            break;
        if (strcmp(buffer, "q") == 0)
            break;

        execute_command(buffer, values, verbose, enable_debugger);
    }
}
