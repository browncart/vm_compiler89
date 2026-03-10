SRC = 	src/arena_allocator.c 				\
		src/tokens.c          				\
		src/lexer.c           				\
		src/parser.c          				\
		src/utils.c           				\
		src/models.c          				\
		src/string.c          				\
		src/hash_table.c					\
		src/arrays/dynamic_array.c			\
		src/arrays/labels_array.c			\
		src/arrays/int_array.c				\
		src/arrays/usize_array.c			\
		src/arrays/ast_node_array.c			\
		src/arrays/symbol_array.c			\
		src/arrays/function_symbol_array.c	\
		src/arrays/instruction_array.c		\
		src/arrays/label_fixup_array.c		\
		src/arrays/value_array.c			\
		src/compile.c						\
		src/vm.c

TESTS = tests/tests.c 			\
		tests/t_hash_table.c	\
		tests/t_dynamic_array.c	

MAIN 	= src/main.c
OUT 	= bin/main
CFLAGS 	= -std=c89
FLAGS 	= -DDEBUG=1 

run:
	gcc $(CFLAGS) $(FLAGS) $(TESTS) $(SRC) -o $(OUT)