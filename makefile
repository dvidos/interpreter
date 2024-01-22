OUTPUT = ipret
FILES = \
	main.c \
	utils/testing.c \
	\
	utils/data_types/str.c \
	utils/data_types/variant.c \
	utils/data_types/variant_tests.c \
	utils/data_types/callable.c \
	\
	utils/containers/dict.c \
	utils/containers/list.c \
	utils/containers/stack.c \
	utils/containers/contained_item.c \
	\
	utils/str_builder.c \
	utils/failable.c \
	utils/file.c \
	\
	interpreter/entities/operator.c \
	interpreter/entities/expression.c \
	interpreter/entities/statement.c \
	interpreter/entities/token.c \
	\
	interpreter/lexer/tokenization.c \
	interpreter/lexer/tokenization_tests.c \
	\
	interpreter/parser/expression_parser.c \
	interpreter/parser/expression_parser_tests.c \
	interpreter/parser/statement_parser.c \
	interpreter/parser/statement_parser_tests.c \
	\
	interpreter/runtime/built_in_funcs.c \
	interpreter/runtime/exec_context.c \
	interpreter/runtime/expression_execution.c \
	interpreter/runtime/statement_execution.c \
	interpreter/runtime/symbol.c \
	interpreter/runtime/symbol_table.c \
	\
	interpreter/interpreter.c \
	interpreter/interpreter_tests.c


$(OUTPUT): $(FILES)
	gcc -g -o $(OUTPUT) $(FILES)
