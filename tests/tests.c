#include "../src/lexer.h"
#include "../src/utils.h"
#include "../src/parser.h"
#include "../src/string.h"
#include "../src/arena_allocator.h"
#include "../src/compile.h"
#include "../src/vm.h"

#include "t_hash_table.h"
#include "t_dynamic_array.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
	#define TTY_DEVICE "CONOUT$"
#else
	#define TTY_DEVICE "/dev/tty"
#endif

#ifndef UPDATE_GOLDENS
	#define UPDATE_GOLDENS 0
#endif

Arena base_nodes_arena 	= {0};
Arena ast_nodes_arena 	= {0};
Arena scratch_space 	= {0};
Arena string_storage 	= {0};

static const char *TMP_OUT = "tests/output/__tmp_stdout.out";

static int capture_begin(const char *path) {
	fflush(stdout);
	return freopen(path, "wb", stdout) ? 0 : -1;
}

static int capture_end() {
	fflush(stdout);
	return freopen(TTY_DEVICE, "w", stdout) ? 0 : -1;
}

static bool files_equal(const char* a, const char* b) {
	unsigned char buf1[4096];
	unsigned char buf2[4096];
	FILE *f1 = fopen(a, "rb");
	FILE *f2 = fopen(b, "rb");
	
	if (!f1 || !f2) { 
		if (f1) fclose(f1); 
		if (f2) fclose(f2);
		return false;
	}

	while (1) {
		size_t n1 = fread(buf1, 1, sizeof(buf1), f1);
		size_t n2 = fread(buf2, 1, sizeof(buf2), f2);
		if (n1 != n2 || memcmp(buf1, buf2, n1) != 0) { 
			fclose(f1); 
			fclose(f2); 
			return false;
		}

		if (n1 == 0) break;
	}

	fclose(f1); 
	fclose(f2);
	return true;
}

static int file_copy(const char *src, const char *dst) {
	FILE *in = fopen(src, "rb"); 
	if (!in) return -1;
	
	FILE *out = fopen(dst, "wb"); 
	if (!out) { 
		fclose(in); 
		return -2; 
	}

	unsigned char buf[4096]; 
	size_t n = fread(buf, 1, sizeof(buf), in);
	
	while (n > 0) {
		if (fwrite(buf, 1, n, out) != n) { 
			fclose(in); 
			fclose(out); 
			return -3;
		}
		n = fread(buf, 1, sizeof(buf), in);
	}

	fclose(in);
	fclose(out);
	return 0;
}

static void run_source_to_tmp(const char *src_path) {
	arena_clear(&base_nodes_arena);
	arena_clear(&ast_nodes_arena);
	arena_clear(&scratch_space);

	/*
		Source
	*/

	FILE *fsrc = fopen(src_path, "rb");
	if (!fsrc) {
		fprintf(stderr, "Failed to open src file: %s", src_path);
		exit(1);
	}

	size_t length = 0;
	size_t *length_ptr = &length;
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

	/*
		Lexer
	*/

	DArray tokens_arr = {0};
	init_darray(&tokens_arr, sizeof(Token), 2);

	LexerState ls = (LexerState){
		.source = src,
		.start 	= 0,
		.curr 	= 0,
		.line 	= 1,
		.tokens = tokens_arr
	};

	tokenize(&ls);

	/*
		Parser / AST
	*/

	ParserState ps = (ParserState){
		.tokens 			= ls.tokens,
		.curr 				= 0,
		.nodes_arena 		= &base_nodes_arena,
		.ast_nodes_arena 	= &ast_nodes_arena
	};

	AstNode *ast = parse(&ps);

	/*
		Compiler
	*/

	InstructionArray code = {0};
	init_instruction_array(&code, 10);

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

	compile(c_state);

	/*
		VM
	*/

	VMState vm_state;
	vm_state.code 				= &code;
	vm_state.scratch 			= &scratch_space;
	vm_state.string_storage		= &string_storage;
	vm_state.globals			= &global_data;
	vm_state.locals_count 		= max_locals;
	vm_state.globals_count 		= max_globals;

	capture_begin(TMP_OUT);
	run_VM(vm_state);
	capture_end();
}

static bool match_golden(const char *src, const char *golden_path) {
	run_source_to_tmp(src);

	if (UPDATE_GOLDENS) {
		if (file_copy(TMP_OUT, golden_path) == 0) {
			printf("[updated] %s\n", golden_path);
			remove(TMP_OUT);
			return true;
		}
		printf("[update failed] %s\n", golden_path);
		remove(TMP_OUT);
		return false;
	}
	
	bool equal = files_equal(TMP_OUT, golden_path);
	remove(TMP_OUT);
	return equal;
}

static int golden_test(const char *src, const char *golden_path, const char *test_name) {
	if (match_golden(src, golden_path)) {
		printf("%so %s%s\n", GREEN, test_name, WHITE);
		return 0;
	} else {
		printf("%sx %s [DIFF] %s vs %s%s\n", RED, test_name, TMP_OUT, golden_path, WHITE);
		return 1;
	}
}

/*
	Tests
*/

static int t_print() {
	return golden_test("tests/scripts/print.pinky", "tests/output/print.out", "Print");
}
static int t_println() {
	return golden_test("tests/scripts/println.pinky", "tests/output/println.out", "Println");
}
static int t_addition() {
	return golden_test("tests/scripts/addition.pinky", "tests/output/addition.out",  "Simple Addition");
}
static int t_group_math() {
	return golden_test("tests/scripts/group_math.pinky", "tests/output/group_math.out", "Grouping + Math");
}
static int t_concat_mixed() {
	return golden_test("tests/scripts/concat_mixed.pinky", "tests/output/concat_mixed.out", "Concatenate Mixed Values");
}
static int t_func_decl_call() {
	return golden_test("tests/scripts/func_decl_call.pinky", "tests/output/func_decl_call.out", "Function Declaration + Call");
}
static int t_if() {
	return golden_test("tests/scripts/if.pinky", "tests/output/if.out", "If Statement");
}
static int t_if_else() {
	return golden_test("tests/scripts/if_else.pinky", "tests/output/if_else.out", "If/Else Statement");
}
static int t_while() {
	return golden_test("tests/scripts/while.pinky", "tests/output/while.out", "While Statement");
}
static int t_for() {
	return golden_test("tests/scripts/nested_for.pinky", "tests/output/nested_for.out", "Nested For Statement");
}
static int t_for_neg() {
	return golden_test("tests/scripts/neg_for.pinky", "tests/output/neg_for.out", "Negative For Statement");
}
static int t_var_scopes() {
	return golden_test("tests/scripts/var_scopes.pinky", "tests/output/var_scopes.out", "Variable Scopes");
}
static int t_recursion() {
	return golden_test("tests/scripts/recursion.pinky", "tests/output/recursion.out", "Simple Recursion");
}
static int t_mandelbrot() {
	return golden_test("tests/scripts/mandelbrot.pinky", "tests/output/mandelbrot.out", "Mandelbrot");
}
static int t_dragon() {
	return golden_test("tests/scripts/dragon.pinky", "tests/output/dragon.out", "Dragon");
}

int main() {
	init_arena(&base_nodes_arena, MAX_NODES);
	init_arena(&ast_nodes_arena, MAX_NODES);
	init_arena(&scratch_space, MAX_NODES);
	init_arena(&string_storage, MAX_NODES);

	printf("%sScript Tests\n%s", CYAN, WHITE);

	int script_fails = 0;

	/*Test Scripts*/
	script_fails += t_print();
	script_fails += t_println();
	script_fails += t_addition();
	script_fails += t_group_math();
	script_fails += t_concat_mixed();
	script_fails += t_func_decl_call();
	script_fails += t_if();
	script_fails += t_if_else();
	script_fails += t_while();
	script_fails += t_for();
	script_fails += t_for_neg();
	script_fails += t_var_scopes();
	script_fails += t_recursion();
	script_fails += t_mandelbrot();
	script_fails += t_dragon();

	if (script_fails) {
		printf("%s\nFailure! %d test(s) failed.%s\n", RED, script_fails, WHITE);
	} else {
		printf("%s\nSuccess! All script tests passed.%s\n", GREEN, WHITE);
	}

	printf("%s\nCode Tests\n%s", CYAN, WHITE);

	int code_fails = 0;

	/*Compiler Code*/
	code_fails += t_hash_table();
	code_fails += t_dynamic_array();

	if (code_fails) {
		printf("%s\nFailure! %d test(s) failed.%s\n", RED, code_fails, WHITE);
	} else {
		printf("%s\nSuccess! All code tests passed.%s\n", GREEN, WHITE);
	}

	return 0;
}