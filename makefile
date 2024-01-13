OUTPUT = ipret
FILES = \
	main.c \
	utils/testing.c \
	utils/failable.c \
	utils/containers/containable.c \
	utils/containers/dict.c \
	utils/containers/list.c \
	utils/containers/stack.c \
	utils/strbld.c \
	utils/variant.c \
	utils/callable.c \
	interpreter/lexer/token.c \
	interpreter/lexer/tokenization.c \
	interpreter/lexer/tokenization_tests.c \
	interpreter/parser/operator.c \
	interpreter/parser/expression.c \
	interpreter/parser/parser.c \
	interpreter/parser/parser_tests.c \
	interpreter/runtime/built_in_funcs.c \
	interpreter/runtime/execution.c \
	interpreter/interpreter.c \
	interpreter/interpreter_tests.c


$(OUTPUT): $(FILES)
	gcc -g -o $(OUTPUT) $(FILES)
