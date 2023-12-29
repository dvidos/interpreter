FILES = \
	main.c \
	value.c \
	dict.c \
	eval.c


evaluator:
	gcc -o evaluator $(FILES)
