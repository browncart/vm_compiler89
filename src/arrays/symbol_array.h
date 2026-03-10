#ifndef SYMBOL_ARRAY_H
#define SYMBOL_ARRAY_H

#include "../common.h"
#include "../models.h"

#include <string.h>

typedef struct SymbolArray SymbolArray;
struct SymbolArray {
	Symbol *data;
	usize 	size;
	usize 	capacity;
};

int 	init_symbol_array			(SymbolArray *a, usize capacity);
int 	resize_symbol_array			(SymbolArray *a);
int 	push_back_symbol_array		(SymbolArray *a, Symbol num);
void 	pop_back_symbol_array		(SymbolArray *a);
Symbol 	get_symbol_array			(SymbolArray *a, usize idx);
void 	clear_symbol_array			(SymbolArray *a);
void 	free_symbol_array			(SymbolArray *a);

#endif