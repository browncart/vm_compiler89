#include "symbol_array.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

int init_symbol_array(SymbolArray *a, usize capacity) {
	a->size 		= 0;
	a->capacity 	= capacity;
	a->data 		= (Symbol *)malloc(a->capacity * sizeof(Symbol));
	return a->data ? 0 : -1;
}

int resize_symbol_array(SymbolArray *a) {
	int new_capacity 	= a->capacity * 2 + 1;
	Symbol *new_data 	= (Symbol *)realloc(a->data, new_capacity * sizeof(Symbol));
	if (!new_data) return -1;
	a->data 			= new_data;
	a->capacity 		= new_capacity;
}

int push_back_symbol_array(SymbolArray *a, Symbol symbol) {
	if (a->size == a->capacity) {
		resize_symbol_array(a);
	}
	a->data[a->size] = symbol;
	a->size++;
	return 0;
}

void pop_back_symbol_array(SymbolArray *a) {
	if (a->size > 0) a->size--;
}

Symbol get_symbol_array(SymbolArray *a, usize idx) {
	assert(idx < a->size && idx >= 0);
	return a->data[idx];
}

void clear_symbol_array(SymbolArray *a) {
	a->size = 0;
}

void free_symbol_array(SymbolArray *a) {
	free(a->data);
	a->data 		= NULL;
	a->size 		= 0;
	a->capacity 	= 0;
}