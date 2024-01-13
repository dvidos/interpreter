OUTPUT = interpreter
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
	code/lexer/token.c \
	code/lexer/tokenization.c \
	code/lexer/tokenization_tests.c \
	code/parser/operator.c \
	code/parser/expression.c \
	code/parser/parser.c \
	code/parser/parser_tests.c \
	code/runtime/built_in_funcs.c \
	code/runtime/execution.c \
	code/interpreter.c \
	code/interpreter_tests.c


$(OUTPUT): $(FILES)
	gcc -g -o $(OUTPUT) $(FILES)
