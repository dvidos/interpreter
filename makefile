OUTPUT = ipret
FILES = \
	main.c \
	utils/testing.c \
	utils/failable.c \
	utils/containers/contained_item.c \
	utils/containers/dict.c \
	utils/containers/list.c \
	utils/containers/stack.c \
	utils/str_builder.c \
	utils/file.c \
	utils/variant.c \
	utils/variant_tests.c \
	utils/callable.c \
	interpreter/lexer/token.c \
	interpreter/lexer/tokenization.c \
	interpreter/lexer/tokenization_tests.c \
	interpreter/parser_expr/operator.c \
	interpreter/parser_expr/expression.c \
	interpreter/parser_expr/expression_parser.c \
	interpreter/parser_expr/expression_parser_tests.c \
	interpreter/parser_stmt/statement.c \
	interpreter/parser_stmt/statement_parser.c \
	interpreter/parser_stmt/statement_parser_tests.c \
	interpreter/runtime/built_in_funcs.c \
	interpreter/runtime/exec_context.c \
	interpreter/runtime/expression_execution.c \
	interpreter/runtime/statement_execution.c \
	interpreter/interpreter.c \
	interpreter/interpreter_tests.c


$(OUTPUT): $(FILES)
	gcc -g -o $(OUTPUT) $(FILES)
