#include "instruction_array.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int init_instruction_array(InstructionArray *s, usize capacity) {
	s->size 		= 0;
	s->capacity 	= capacity;
	s->data 		= (Instruction *)malloc(s->capacity * sizeof(Instruction));
	return s->data ? 0 : -1;
}

int resize_instruction_array(InstructionArray *s) {
	int new_capacity 	= s->capacity * 2 + 1;
	Instruction *new_data 	= (Instruction *)realloc(s->data, new_capacity * sizeof(Instruction));
	if (!new_data) return -1;
	s->data 			= new_data;
	s->capacity 		= new_capacity;
}

int push_back_instruction_array(InstructionArray *s, Instruction instruction) {
	if (s->size == s->capacity) {
		resize_instruction_array(s);
	}
	s->data[s->size] = instruction;
	s->size++;
	return 0;
}

void pop_back_instruction_array(InstructionArray *s) {
	if (s->size > 0) s->size--;
}

Instruction *get_pointer_instruction_array(InstructionArray *s, usize idx) {
	assert(idx < s->size && idx >= 0);
	return &s->data[idx];
}

Instruction get_instruction_array(InstructionArray *s, usize idx) {
	assert(idx < s->size && idx >= 0);
	return s->data[idx];
}

void clear_instruction_array(InstructionArray *s) {
	s->size = 0;
}

void free_instruction_array(InstructionArray *s) {
	free(s->data);
	s->data 		= NULL;
	s->size 		= 0;
	s->capacity 	= 0;
}