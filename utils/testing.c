#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "testing.h"

#define TERM_COLOR_NORMAL  "\x1b[0m"
#define TERM_COLOR_RED     "\x1b[1;31m"  // 31 is normal, 91 is light
#define TERM_COLOR_GREEN   "\x1b[1;32m"  // 1 is bold, 3 is italics
#define TERM_COLOR_YELLOW  "\x1b[1;33m"

static int asserts_done;
static int asserts_failed;

void __testing_assert(bool passed, const char *condition, const char *message, const char *file, int line) {
    asserts_done++;
    if (!passed) {
        asserts_failed++;
        fprintf(stderr, "%sAssertion failed%s: \"%s\"%s%s, at %s:%d\n", 
            TERM_COLOR_YELLOW,
            TERM_COLOR_NORMAL,
            condition,
            message == NULL ? "" : ", ",
            message == NULL ? "" : message,
            file, line
        );
    }
}

void __testing_passed(const char *file, int line) {
    asserts_done++;
}

void __testing_failed(const char *message, const char *file, int line) {
    asserts_done++;
    asserts_failed++;
    fprintf(stderr, "Test failed: %s, at %s:%d\n", message, file, line);
}

bool testing_outcome(bool verbose) {
    if (verbose) {
        fprintf(stderr, "Tests %s%s%s, %d assertions, %d failed.\n", 
            asserts_failed > 0 ? TERM_COLOR_RED : TERM_COLOR_GREEN,
            asserts_failed > 0 ? "FAILED" : "PASSED!",
            TERM_COLOR_NORMAL,
            asserts_done,
            asserts_failed
        );
    }
    return asserts_failed == 0;
}
