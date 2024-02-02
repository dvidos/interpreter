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
	src/utils/containers/pair.c \
	src/utils/containers/contained_item.c \
	src/utils/containers/containers_tests.c \
	\
	src/utils/str_builder.c \
	src/utils/failable.c \
	src/utils/file.c \
	\
	src/entities/operator_type.c \
	src/entities/operator.c \
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
	src/runtime/exec_context.c \
	src/runtime/expression_execution.c \
	src/runtime/statement_execution.c \
	src/runtime/symbol_table.c \
	\
	src/interpreter/interpreter.c \
	src/interpreter/interpreter_tests.c


$(OUTPUT): $(FILES)
	gcc -g -o $(OUTPUT) $(FILES)
