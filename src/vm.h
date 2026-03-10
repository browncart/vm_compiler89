#ifndef VM_H
#define VM_H

#include "models.h"
#include "common.h"
#include "arena_allocator.h"
#include "arrays/instruction_array.h"
#include "arrays/value_array.h"

typedef struct VMState VMState;
struct VMState {
	InstructionArray   *code;
	Arena			   *string_storage;
	Arena 	  		   *scratch;
	ValueArray		   *globals;
	int 				locals_count;
	int					globals_count;
};

void run_VM(VMState vm_state);

#endif