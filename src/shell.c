#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "interpreter/interpreter.h"
#include "utils/str_builder.h"

static bool get_command(char *buffer, int buffer_size) {
    if (fgets(buffer, buffer_size, stdin) == NULL)
        return false;

    if (buffer[strlen(buffer) - 1] == '\n')
        buffer[strlen(buffer) - 1] = 0;        
    
    return true;
}


static void execute_command(const char *code, dict *values, bool verbose, bool enable_debugger) {

    failable_variant execution = interpret_and_execute(code, "interactive", values, verbose, enable_debugger, false);
    if (execution.failed) {
        printf("Evaluation failed!\n");
        failable_print(&execution);
        printf("\n");
        return;
    }

    str_builder *sb = new_str_builder();
    variant_describe(execution.result, sb);
    printf("%s\n", str_builder_charptr(sb));
    str_builder_free(sb);
}

void interactive_shell(bool verbose, bool enable_debugger) {
    char buffer[128];
    dict *values = new_dict(variant_class);

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
