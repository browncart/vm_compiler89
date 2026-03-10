#include "utils.h"
#include "string.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

const char *WHITE 	= "\033[0m";
const char *BLUE 	= "\033[94m";
const char *CYAN 	= "\033[96m";
const char *GREEN 	= "\033[92m";
const char *YELLOW 	= "\033[93m";
const char *RED 	= "\033[91m";

void lexer_error(const char *msg, int line_number) {
	printf("%sLexer Error [Line %d]: %s%s\n", RED, line_number, msg, WHITE);
	exit(1);
}

void parse_error(const char *msg) {
	printf("%sParser Error: %s%s\n", RED, msg, WHITE);
	exit(1);
}

void parse_error_with_line(const char *msg, int line_number) {	
	printf("%sParser Error [Line %d]: %s%s\n", RED, line_number, msg, WHITE);
	exit(1);
}

void compile_error(const char *msg) {
	printf("%sCompiler Error: %s%s\n", RED, msg, WHITE);
	exit(1);
}

void compile_error_with_line(const char *msg, int line_number) {
	printf("%sCompiler Error [Line %d]: %s%s\n", RED, line_number, msg, WHITE);
	exit(1);
}

void vm_runtime_error(const char *msg, usize pc) {
	printf("%sVM Runtime Error [PC %lu]: %s%s\n", RED, pc, msg, WHITE);
	exit(1);
}

void runtime_error(const char *msg) {
	printf("%sRuntime Error: %s%s\n", RED, msg, WHITE);
	exit(1);
}

void runtime_error_with_line(const char *msg, int line_number) {
	printf("%sRuntime Error [Line %d]: %s%s\n", RED, line_number, msg, WHITE);
	exit(1);
}

void runtime_op_error(NodeType node_type, const char *lexeme, int line_number, int args_count, ...) {
	va_list args;
	va_start(args, args_count);

	int buffer_size = 256;
	char buffer[buffer_size];

	switch (node_type) {
		case _BinOp: {
			Value left_value 		= va_arg(args, Value);
			Value right_value 		= va_arg(args, Value);

			const char *left_str 	= value_strings[left_value.type];
			const char *right_str 	= value_strings[right_value.type];

			snprintf(buffer, buffer_size, "Unsupported operation [%s] between %s & %s!", lexeme, left_str, right_str);
			break;
		}
		case _UnOp: {
			Value operand 			= va_arg(args, Value);
			const char *operand_str = value_strings[operand.type];

			snprintf(buffer, buffer_size, "Unsupported operation [%s] on %s!", lexeme, operand_str);
			break;
		}
	}
	
	printf("%sRuntime Error [Line %d]: %s%s\n", RED, line_number, buffer, WHITE);
	exit(1);
}

int get_file_size(FILE *f, size_t *out) {
	size_t curr = ftell(f);
	if (curr < 0) return -1;

	if (fseek(f, 0, SEEK_END) != 0) return -1;
	size_t size = ftell(f);
	if (size < 0) return -1;

	if (fseek(f, curr, SEEK_SET) != 0) return -1;
	*out = size;
	return 0;
}