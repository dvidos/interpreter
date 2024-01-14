OUTPUT = ipret
FILES = \
	main.c \
	utils/testing.c \
	utils/failable.c \
	utils/containers/contained_item.c \
	utils/containers/dict.c \
	utils/containers/list.c \
	utils/containers/stack.c \
	utils/strbld.c \
	utils/file.c \
	utils/variant.c \
	utils/callable.c \
	interpreter/lexer/token.c \
	interpreter/lexer/tokenization.c \
	interpreter/lexer/tokenization_tests.c \
	interpreter/parser/operator.c \
	interpreter/parser/expression.c \
	interpreter/parser/expression_parser.c \
	interpreter/parser/expression_parser_tests.c \
	interpreter/parser/statement.c \
	interpreter/parser/statement_parser.c \
	interpreter/runtime/built_in_funcs.c \
	interpreter/runtime/execution.c \
	interpreter/interpreter.c \
	interpreter/interpreter_tests.c


$(OUTPUT): $(FILES)
	gcc -g -o $(OUTPUT) $(FILES)
