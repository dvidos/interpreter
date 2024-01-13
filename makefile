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
	eval/token.c \
	eval/tokenization.c \
	eval/operator.c \
	eval/expression.c \
	eval/parser.c \
	eval/built_in_funcs.c \
	eval/execution.c \
	eval/eval.c \
	tests.c


evaluator: $(FILES)
	gcc -g -o evaluator $(FILES)
