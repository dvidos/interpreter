#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "failable.h"


failable ok() {
    return (failable){ false };
}

const char *__fail_message(const char *fmt, va_list vl) {
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), fmt, vl);
    char *p = malloc(strlen(buffer) + 1);
    strcpy(p, buffer);
    return p;
}

failable *__fail_clone(void *some_failable_ptr) {
    failable *clone = malloc(sizeof(failable));
    memcpy(clone, some_failable_ptr, sizeof(failable));
    return clone;
}

failable __failed(void *inner, const char *func, const char *file, int line, const char *fmt, ...) {
    failable f = { true, NULL, func, file, line, NULL };
    if (inner != NULL) {
        // stack objects will be clobbered
        failable *inner_copy = malloc(sizeof(failable));
        memcpy(inner_copy, inner, sizeof(failable));
        f.inner = inner_copy;
    }
    if (fmt != NULL) {
        va_list args;
        va_start(args, fmt);
        f.err_msg = __fail_message(fmt, args);
        va_end(args);
    }
    return f;
}

STRONGLY_TYPED_FAILABLE_VAL_IMPLEMENTATION(bool);
STRONGLY_TYPED_FAILABLE_VAL_IMPLEMENTATION(int);
STRONGLY_TYPED_FAILABLE_PTR_IMPLEMENTATION(const_char);


static void failable_print_reverse(void *some_failable) {
    failable *f = (failable *)some_failable;
    if (f == NULL)
        return;
    failable_print_reverse(f->inner);

    const char *basename = strrchr(f->file, '/') == NULL ? f->file : strrchr(f->file, '/') + 1;
    if (f->err_msg == NULL)
        printf("    %s(), at %s:%d\n", f->func, basename, f->line);
    else
        printf("    %s() -> \"%s\", at %s:%d\n", f->func, f->err_msg, basename, f->line);
}

void failable_print(void *some_failable) {
    // failable *f = (failable *)some_failable;
    // while (f != NULL) {
    //     const char *basename = strrchr(f->file, '/') == NULL ? f->file : strrchr(f->file, '/') + 1;
    //     if (f->err_msg == NULL)
    //         printf("    %s(), at %s:%d\n", f->func, basename, f->line);
    //     else
    //         printf("    %s() -> \"%s\", at %s:%d\n", f->func, f->err_msg, basename, f->line);
    //     f = f->inner;
    // }
    failable_print_reverse(some_failable);
}