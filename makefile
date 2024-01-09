FILES = \
	main.c \
	utils/testing.c \
	utils/failable.c \
	utils/containable.c \
	utils/strbld.c \
	utils/variant.c \
	utils/dict.c \
	utils/list.c \
	utils/stack.c \
	eval/token.c \
	eval/tokenization.c \
	eval/operator.c \
	eval/expression.c \
	eval/parser.c \
	eval/execution.c \
	eval/eval.c \
	tests.c


evaluator: $(FILES)
	gcc -g -o evaluator $(FILES)
