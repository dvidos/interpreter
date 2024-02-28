OUTPUT = ipret
FILES = \
	src/main.c \
	src/utils/testing.c \
	\
	src/utils/data_types/variant.c \
	src/utils/data_types/variant_tests.c \
	src/utils/data_types/callable.c \
	src/utils/data_types/exec_context.c \
	src/utils/data_types/stack_frame.c \
	\
	src/utils/containers/dict.c \
	src/utils/containers/list.c \
	src/utils/containers/stack.c \
	src/utils/containers/queue.c \
	src/utils/containers/pair.c \
	src/utils/containers/containers_tests.c \
	\
	src/utils/class.c \
	src/utils/str.c \
	src/utils/str_builder.c \
	src/utils/failable.c \
	src/utils/file.c \
	src/utils/listing.c \
	\
	src/entities/operator_type.c \
	src/entities/expression.c \
	src/entities/statement.c \
	src/entities/token_type.c \
	src/entities/token.c \
	\
	src/lexer/tokenization.c \
	src/lexer/tokenization_tests.c \
	\
	src/parser/expression_parser.c \
	src/parser/expression_parser_tests.c \
	src/parser/statement_parser.c \
	src/parser/statement_parser_tests.c \
	\
	src/runtime/built_in_funcs.c \
	src/runtime/built_in_funcs_tests.c \
	src/runtime/expression_execution.c \
	src/runtime/execution_result.c \
	src/runtime/statement_execution.c \
	\
	src/debugger/debugger.c \
	src/debugger/breakpoint.c \
	\
	src/interpreter/interpreter.c \
	src/interpreter/interpreter_tests.c \
	\
	src/shell/shell.c


$(OUTPUT): $(FILES)
	gcc -g -o $(OUTPUT) $(FILES)
