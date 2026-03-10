#ifndef UTILS_H
#define UTILS_H

#include "models.h"
#include "common.h"
#include <stdio.h>

#ifndef DEBUG
#define DEBUG 0
#endif

#if DEBUG
	#define DEBUG_PRINT_SOURCE(buffer, length) \
		do { \
			usize i; \
			for (i = 0; i < length; ++i) { \
				fprintf(stderr, "%c", buffer[i]); \
			} \
				fprintf(stderr, "\n"); \
		} while (0)
	
	#define DEBUG_PRINT_TOKENS(ls) \
		do { \
				int i; \
				for (i = 0; i < ls.tokens.size; ++i) { \
					Token token = ((Token *)ls.tokens.data)[i]; \
					int token_size = token.lexeme.size; \
					token_size = token_size > INT_MAX ? INT_MAX : (int)token_size; \
					fprintf(stderr, "(%s, '%.*s', %d)\n", token_print[token.token], token_size, token.lexeme.str, token.line); \
				} \
		} while (0)
	
	#define DEBUG_PRINT_AST(ast, arena) \
		do { \
			print_ast_node(ast, arena, 0); \
		} while (0)

	#define DEBUG_PRINT_COMPILER(code) \
		do { \
			print_compiler_instructions(code); \
		} while (0)
	
	#define DEBUG_PRINT(str, ...) \
		fprintf(stderr, str, ##__VA_ARGS__)
#else
	#define DEBUG_PRINT_SOURCE(buffer, length) do {} while (0)
	#define DEBUG_PRINT_TOKENS(ls) do {} while (0)
	#define DEBUG_PRINT_AST(ast, arena) do {} while (0)
	#define DEBUG_PRINT_COMPILER(code) do {} while (0)
	#define DEBUG_PRINT(str, ...) do {} while (0)
#endif

#define MAX(a, b) ((a) > (b) ? (a) : (b))

extern const char *WHITE;
extern const char *BLUE;
extern const char *CYAN;
extern const char *GREEN;
extern const char *YELLOW;
extern const char *RED;

void lexer_error				(const char *msg, int line_number);
void parse_error				(const char *msg);
void parse_error_with_line		(const char *msg, int line_number);
void compile_error				(const char *msg);
void compile_error_with_line	(const char *msg, int line_number);
void vm_runtime_error			(const char *msg, usize pc);
void runtime_error				(const char *msg);
void runtime_error_with_line	(const char *msg, int line_number);
void runtime_op_error			(NodeType node_type, const char *lexeme, int line_number, int args_count, ...);

int get_file_size				(FILE *f, size_t *out);

#endif