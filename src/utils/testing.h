#ifndef _TESTING_H
#define _TESTING_H

#include <stdbool.h>

#ifdef assert
#undef assert
#endif

#define assert(x)                  __testing_assert(x, #x, NULL, __FILE__, __LINE__)
#define assert_msg(x, msg)         __testing_assert(x, #x, msg, __FILE__, __LINE__)
#define assertion_passed(msg)      __testing_passed(__FILE__, __LINE__)
#define assertion_failed(msg)      __testing_failed(msg, __FILE__, __LINE__)

// for better visibility:
// assert_str_equals(given, expected)
// assert_str_not_equals(given, expected)
// equal_to, not_equal_to, null, not_null, larger_than, larger_equal_than, less_than, larger_equal_than
// etc.
#define assert_str_builder_equals(sb, str)   assert(strcmp(str_builder_charptr(sb), str) == 0)




void __testing_assert(bool passed, const char *condition, const char *message, const char *file, int line);
void __testing_passed(const char *file, int line);
void __testing_failed(const char *message, const char *file, int line);

bool testing_outcome(bool verbose);



#endif
