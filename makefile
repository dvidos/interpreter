FILES = \
	main.c \
	value.c \
	dict.c \
	list.c \
	stack.c \
	token.c \
	tokenization.c \
	expression.c \
	eval.c


evaluator:
	gcc -g -o evaluator $(FILES)
