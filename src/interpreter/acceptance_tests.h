

typedef struct acceptance_test acceptance_test;

acceptance_test *new_acceptance_test();
void acceptance_test_expect_result(acceptance_test *t, const char *text);
void acceptance_test_expect_log(acceptance_test *t, const char *text);
void acceptance_test_add_code_line(acceptance_test *t, const char *text);
void acceptance_test_run();
void acceptance_test_free(acceptance_test *t);


bool run_acceptance_tests_from_dir(const char *dirpath, const char *extension, bool with_debugger);
bool run_acceptance_tests_from_text(const char *text, const char *filename, bool with_debugger);
