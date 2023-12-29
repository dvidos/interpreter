FILES = \
	main.c \
	utils/value.c \
	utils/dict.c \
	utils/list.c \
	utils/stack.c \
	eval/token.c \
	eval/tokenization.c \
	eval/expression.c \
	eval/eval.c \
	tests.c


evaluator:
	gcc -g -o evaluator $(FILES)
