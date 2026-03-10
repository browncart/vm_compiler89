#ifndef INSTRUCTION_ARRAY_H
#define INSTRUCTION_ARRAY_H

#include "../common.h"
#include "../models.h"

#include <string.h>

typedef struct InstructionArray InstructionArray;
struct InstructionArray {
	Instruction    *data;
	usize 			size;
	usize 			capacity;
};

int 			init_instruction_array			(InstructionArray *i, usize capacity);
int 			resize_instruction_array		(InstructionArray *i);
int 			push_back_instruction_array		(InstructionArray *i, Instruction instruction);
void 			pop_back_instruction_array		(InstructionArray *i);
Instruction    *get_pointer_instruction_array	(InstructionArray *i, usize idx);
Instruction 	get_instruction_array			(InstructionArray *i, usize idx);
void 			clear_instruction_array			(InstructionArray *i);
void 			free_instruction_array			(InstructionArray *i);

#endif