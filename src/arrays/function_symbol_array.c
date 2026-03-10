#include "function_symbol_array.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int init_function_symbol_array(FunctionSymbolArray *a, usize capacity) {
	a->size 		= 0;
	a->capacity 	= capacity;
	a->data 		= (FunctionSymbol *)malloc(a->capacity * sizeof(FunctionSymbol));
	return a->data ? 0 : -1;
}

int resize_function_symbol_array(FunctionSymbolArray *a) {
	int new_capacity 	= a->capacity * 2 + 1;
	FunctionSymbol *new_data 	= (FunctionSymbol *)realloc(a->data, new_capacity * sizeof(FunctionSymbol));
	if (!new_data) return -1;
	a->data 			= new_data;
	a->capacity 		= new_capacity;
}

int push_back_function_symbol_array(FunctionSymbolArray *a, FunctionSymbol symbol) {
	if (a->size == a->capacity) {
		resize_function_symbol_array(a);
	}
	a->data[a->size] = symbol;
	a->size++;
	return 0;
}

void pop_back_function_symbol_array(FunctionSymbolArray *a) {
	if (a->size > 0) a->size--;
}

FunctionSymbol get_function_symbol_array(FunctionSymbolArray *a, usize idx) {
	assert(idx < a->size && idx >= 0);
	return a->data[idx];
}

void clear_function_symbol_array(FunctionSymbolArray *a) {
	a->size = 0;
}

void free_function_symbol_array(FunctionSymbolArray *a) {
	free(a->data);
	a->data 		= NULL;
	a->size 		= 0;
	a->capacity 	= 0;
}