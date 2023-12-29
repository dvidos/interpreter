FILES = \
	main.c \
	value.c \
	dict.c \
	list.c \
	token.c \
	expression.c \
	eval.c


evaluator:
	gcc -g -o evaluator $(FILES)
