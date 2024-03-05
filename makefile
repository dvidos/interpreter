OUTPUT = ipret
FILES = \
	src/main.c \
	src/utils/testing.c \
	\
	src/utils/data_types/callable.c \
	src/utils/data_types/exec_context.c \
	src/utils/data_types/stack_frame.c \
	\
	src/containers/dict.c \
	src/containers/list.c \
	src/containers/stack.c \
	src/containers/queue.c \
	src/containers/pair.c \
	src/containers/containers_tests.c \
	\
	src/utils/class.c \
	src/utils/str.c \
	src/utils/str_builder.c \
	src/utils/failable.c \
	src/utils/file.c \
	src/utils/listing.c \
	src/utils/mem.c \
	src/utils/error.c \
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
	src/runtime/variants/variant_original.c \
	src/runtime/variants/variant_tests.c \
	src/runtime/variants/variant_type.c \
	src/runtime/variants/variant_funcs.c \
	src/runtime/variants/str_variant.c \
	\
	src/runtime/execution/execution_outcome.c \
	src/runtime/execution/expression_execution.c \
	src/runtime/execution/statement_execution.c \
	\
	src/runtime/built_ins/built_in_funcs.c \
	src/runtime/built_ins/built_in_funcs_tests.c \
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
