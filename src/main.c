#include "arena_allocator.h"
#include "lexer.h"
#include "parser.h"
#include "utils.h"
#include "string.h"
#include "compile.h"
#include "vm.h"

#include <stdlib.h>
#include <windows.h>

double current_time() {
	static LARGE_INTEGER freq;
	static int init;
	LARGE_INTEGER counter;
	
	if (!init) {
		QueryPerformanceFrequency(&freq);
		init = 1;
	}

	QueryPerformanceCounter(&counter);

	return (double)counter.QuadPart / (double)freq.QuadPart;
}

int main(void) {
	/*
		Source
	*/

	DEBUG_PRINT("%s************\nSOURCE:\n************\n%s", GREEN, WHITE);

	const char *src_path = "./scripts/mandelbrot.pinky";
	/*const char *src_path = "./scripts/dragon.pinky";*/

	FILE *fsrc = fopen(src_path, "rb");
	if (!fsrc) {
		fprintf(stderr, "Failed to open src file: %s", src_path);
		exit(1);
	}

	usize length = 0;
	usize *length_ptr = &length;
	if (get_file_size(fsrc, length_ptr) != 0) {
		fprintf(stderr, "Failed to get src size: %s", src_path);
		exit(1);
	}

	char *buffer = (char *)malloc(length + 1);
	buffer[length] = '\0';

	fread(buffer, 1, length, fsrc);
	fclose(fsrc);

	String8 src = (String8){
		.str 	= buffer,
		.size 	= length 
	};

	DEBUG_PRINT_SOURCE(buffer, length);

	/*
		Lexer
	*/

	DEBUG_PRINT("%s************\nLEXER:\n************\n%s", GREEN, WHITE);

	DArray tokens_arr = {0};
	init_darray(&tokens_arr, sizeof(Token), 2);

	LexerState ls = (LexerState){
		.source = src,
		.start 	= 0,
		.curr 	= 0,
		.line 	= 1,
		.tokens = tokens_arr
	};

	double lexer_time = current_time();
	tokenize(&ls);
	lexer_time = current_time() - lexer_time;

	int k;
	for (k = 0; k < ls.tokens.size; ++k) {
		Token token = *(Token *)get_darray(&ls.tokens, k);
		int token_size = token.lexeme.size;
		token_size = token_size > INT_MAX ? INT_MAX : (int)token_size;
		fprintf(stderr, "(%s, '%.*s', %d)\n", token_print[token.token], token_size, token.lexeme.str, token.line);
	}

	DEBUG_PRINT_TOKENS(ls);

	printf("\nLexer took %.17f seconds.\n", lexer_time);

	/*
		Parser / AST
	*/
	
	DEBUG_PRINT("%s************\nPARSER:\n************\n%s", GREEN, WHITE);

	Arena base_nodes_arena = {0};
	init_arena(&base_nodes_arena, MAX_NODES);

	Arena ast_nodes_arena = {0};
	init_arena(&ast_nodes_arena, MAX_NODES);

	ParserState ps = (ParserState){
		.tokens 			= ls.tokens,
		.curr 				= 0,
		.nodes_arena 		= &base_nodes_arena,
		.ast_nodes_arena 	= &ast_nodes_arena
	};

	double parser_time = current_time();
	AstNode *ast = parse(&ps);
	parser_time = current_time() - parser_time;

	DEBUG_PRINT_AST(ast, ps.nodes_arena);

	printf("\n\nParser took %.17f seconds.", parser_time);
	DEBUG_PRINT("\n\nAST Arena Memory Usage: %lu bytes", (unsigned long)base_nodes_arena.curr);
	DEBUG_PRINT("\nBase Nodes Arena Memory Usage: %lu bytes\n", (unsigned long)ast_nodes_arena.curr);

	/*
		Compiler
	*/

	DEBUG_PRINT("%s************\nCompiler:\n************\n%s", GREEN, WHITE);

	InstructionArray code = {0};
	init_instruction_array(&code, 10);
	
	Arena scratch_space = {0};
	init_arena(&scratch_space, MAX_NODES);

	HashTable globals = {0};
	init_ht(&globals, 10, sizeof(Value));

	ValueArray global_data = {0};
	init_value_array(&global_data, 10);

	SymbolArray locals = {0};
	init_symbol_array(&locals, 10);

	UsizeArray scope_markers = {0};
	init_usize_array(&scope_markers, 10);

	HashTable local_indexes = {0};
	init_ht(&local_indexes, 10, sizeof(int));

	UsizeArray func_arity = {0};
	init_usize_array(&func_arity, 10);

	LabelsArray labels = {0};
	init_labels(&labels, 8);

	LabelFixupArray fixups = {0};
	init_label_fixup_array(&fixups, 10);

	HashTable funcs = {0};
	init_ht(&funcs, 10, sizeof(int));

	HashTable new_funcs = {0};
	init_ht(&new_funcs, 10, sizeof(FunctionSymbol));

	int max_locals 			= 0;
	int max_globals 		= 0;
	int global_slot_count 	= 0;
	int label_counter		= 0;
	int local_slot_count	= 0;
	int scope_depth			= 0;

	CompilerState c_state;
	c_state.base_nodes_arena 	= &base_nodes_arena;
	c_state.scratch				= &scratch_space;
	c_state.ast 				= ast;
	c_state.globals				= &globals;
	c_state.global_data			= &global_data;
	c_state.locals				= &locals;
	c_state.scope_markers		= &scope_markers;
	c_state.local_indexes		= &local_indexes;
	c_state.func_arity			= &func_arity;
	c_state.code				= &code;
	c_state.labels				= &labels;
	c_state.fixups				= &fixups;
	c_state.funcs				= &new_funcs;
	c_state.max_locals			= &max_locals;
	c_state.max_globals			= &max_globals;
	c_state.global_slot_count	= &global_slot_count;
	c_state.label_counter		= &label_counter;
	c_state.local_slot_count	= &local_slot_count;
	c_state.scope_depth			= &scope_depth;

	double compile_time = current_time();
	compile(c_state);
	compile_time = current_time() - compile_time;

	DEBUG_PRINT_COMPILER(&code);

	printf("GLOBAL DATA COUNT: %lu\n", global_data.size);

	printf("\nCompiler took %.17f seconds.\n", compile_time);
	DEBUG_PRINT("\nScratch Space Arena Memory Usage: %lu bytes (Should be Zero)\n", (unsigned long)scratch_space.curr);

	/*
		VM
	*/

	DEBUG_PRINT("%s************\nVM:\n************\n%s", GREEN, WHITE);

	Arena string_storage = {0};
	init_arena(&string_storage, MAX_NODES);

	VMState vm_state;
	vm_state.code 				= &code;
	vm_state.scratch 			= &scratch_space;
	vm_state.string_storage		= &string_storage;
	vm_state.globals			= &global_data;
	vm_state.locals_count 		= max_locals;
	vm_state.globals_count 		= max_globals;

	printf("MAX LOCALS: %d\n", max_locals);
	printf("GLOBALS COUNT IS: %d\n\n", vm_state.globals_count);

	double vm_time = current_time();
	run_VM(vm_state);
	vm_time = current_time() - vm_time;

	printf("\nVM took %.17f seconds.\n", vm_time);
	DEBUG_PRINT("\nScratch Space Arena Memory Usage: %lu bytes (Should be Zero)\n", (unsigned long)scratch_space.curr);

	return 0;
}