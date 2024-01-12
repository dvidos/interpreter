#ifndef _TESTING_H
#define _TESTING_H

#include <stdbool.h>

#ifdef assert
#undef assert
#endif

#define assert(x)            __testing_assert(x, #x, NULL, __FILE__, __LINE__)
#define assert_msg(x, msg)   __testing_assert(x, #x, msg, __FILE__, __LINE__)
#define fail_test(msg)       __testing_failed(msg, __FILE__, __LINE__)


void __testing_assert(bool passed, const char *condition, const char *message, const char *file, int line);
void __testing_failed(const char *message, const char *file, int line);

bool testing_outcome(bool verbose);


#endif
