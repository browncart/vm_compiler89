#include "value_array.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int init_value_array(ValueArray *a, usize capacity) {
	a->size 		= 0;
	a->capacity 	= capacity;
	a->data 		= (Value *)malloc(a->capacity * sizeof(Value));
	return a->data ? 0 : -1;
}

int resize_value_array(ValueArray *a) {
	int new_capacity 	= a->capacity * 2 + 1;
	Value *new_data 	= (Value *)realloc(a->data, new_capacity * sizeof(Value));
	if (!new_data) return -1;
	a->data 			= new_data;
	a->capacity 		= new_capacity;
}

int push_back_value_array(ValueArray *a, Value value) {
	if (a->size == a->capacity) {
		resize_value_array(a);
	}
	a->data[a->size] = value;
	a->size++;
	return 0;
}

void pop_back_value_array(ValueArray *a) {
	if (a->size > 0) a->size--;
}

Value get_value_array(ValueArray *a, usize idx) {
	assert(idx < a->size && idx >= 0);
	return a->data[idx];
}

Value *get_pointer_value_array(ValueArray *s, usize idx) {
	assert(idx < s->size && idx >= 0);
	return &s->data[idx];
}

void clear_value_array(ValueArray *a) {
	a->size = 0;
}

void free_value_array(ValueArray *a) {
	free(a->data);
	a->data 		= NULL;
	a->size 		= 0;
	a->capacity 	= 0;
}