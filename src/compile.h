#ifndef COMPILE_H
#define COMPILE_H

#include "models.h"
#include "arena_allocator.h"
#include "string.h"
#include "hash_table.h"
#include "arrays/labels_array.h"
#include "arrays/usize_array.h"
#include "arrays/symbol_array.h"
#include "arrays/label_fixup_array.h"
#include "arrays/instruction_array.h"
#include "arrays/value_array.h"

#define get_offset(type, member) ((usize)&(((type *)0)->member))

typedef struct StackFrame StackFrame;
struct StackFrame {
	int return_pc;
	int base_slot;
	int local_count;
};

extern char *op_codes[];
extern char *op_types[];

void print_compiler_instructions(InstructionArray *code);

typedef struct CompilerState CompilerState;
struct CompilerState {
	Arena 				*base_nodes_arena;
	Arena 				*scratch;
	AstNode 			*ast; 
	HashTable 			*globals;
	ValueArray			*global_data;
	SymbolArray			*locals;
	HashTable 			*local_indexes;
	UsizeArray 			*scope_markers;
	UsizeArray			*func_arity;
	InstructionArray 	*code;
	LabelsArray			*labels;
	LabelFixupArray		*fixups;
	HashTable 			*funcs;
	int					*max_locals;
	int					*max_globals;
	int					*global_slot_count;
	int 				*label_counter;
	int 				*local_slot_count;
	int 				*scope_depth;
};

void compile(CompilerState c_state);
void compile_ast(CompilerState c_state);

#endif