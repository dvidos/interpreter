OUTPUT = ipret
FILES = \
	src/main.c \
	src/utils/testing.c \
	\
	src/utils/data_types/str.c \
	src/utils/data_types/variant.c \
	src/utils/data_types/variant_tests.c \
	src/utils/data_types/callable.c \
	\
	src/utils/containers/dict.c \
	src/utils/containers/list.c \
	src/utils/containers/stack.c \
	src/utils/containers/queue.c \
	src/utils/containers/contained_item.c \
	src/utils/containers/containers_tests.c \
	\
	src/utils/str_builder.c \
	src/utils/failable.c \
	src/utils/file.c \
	\
	src/interpreter/entities/operator.c \
	src/interpreter/entities/expression.c \
	src/interpreter/entities/statement.c \
	src/interpreter/entities/token.c \
	\
	src/interpreter/lexer/tokenization.c \
	src/interpreter/lexer/tokenization_tests.c \
	\
	src/interpreter/parser/expression_parser.c \
	src/interpreter/parser/expression_parser_tests.c \
	src/interpreter/parser/statement_parser.c \
	src/interpreter/parser/statement_parser_tests.c \
	\
	src/interpreter/runtime/built_in_funcs.c \
	src/interpreter/runtime/exec_context.c \
	src/interpreter/runtime/expression_execution.c \
	src/interpreter/runtime/statement_execution.c \
	src/interpreter/runtime/symbol_table.c \
	\
	src/interpreter/interpreter.c \
	src/interpreter/interpreter_tests.c


$(OUTPUT): $(FILES)
	gcc -g -o $(OUTPUT) $(FILES)
