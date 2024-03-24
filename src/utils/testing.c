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

static int asserts_passed;
static int asserts_failed;

void testing_initialize(const char *tests_type) {
    asserts_passed = 0;
    asserts_failed = 0;
    fprintf(stderr, "Running %s tests...\n", tests_type);
}

void __testing_passed() {
    asserts_passed++;
}

void __testing_failed(const char *use_case, const char *extra, const char *file, int line) {
    asserts_failed++;
    fprintf(stderr, "%sTest failed%s: \"%s\"%s%s, at %s:%d\n", 
        TERM_COLOR_YELLOW,
        TERM_COLOR_NORMAL,
        use_case,
        extra == NULL ? "" : ": ",
        extra == NULL ? "" : extra,
        file, line
    );
}

void __testing_assert(bool passed, const char *use_case, const char *extra, const char *file, int line) {
    if (passed)
        __testing_passed();
    else
        __testing_failed(use_case, extra, file, line);
}

bool testing_outcome() {
    fprintf(stderr, "Tests %s%s%s, %d assertions, %d passed, %d failed.\n", 
        asserts_failed > 0 ? TERM_COLOR_RED : TERM_COLOR_GREEN,
        asserts_failed > 0 ? "FAILED" : "PASSED!",
        TERM_COLOR_NORMAL,
        asserts_passed + asserts_failed,
        asserts_passed,
        asserts_failed
    );

    return (asserts_failed == 0);
}
