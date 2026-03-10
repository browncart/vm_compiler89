#ifndef FUNCTION_SYMBOL_ARRAY_H
#define FUNCTION_SYMBOL_ARRAY_H

#include "../common.h"
#include "../models.h"

#include <string.h>

typedef struct FunctionSymbolArray FunctionSymbolArray;
struct FunctionSymbolArray {
	FunctionSymbol *data;
	usize 	size;
	usize 	capacity;
};

int 			init_function_symbol_array			(FunctionSymbolArray *a, usize capacity);
int 			resize_function_symbol_array		(FunctionSymbolArray *a);
int 			push_back_function_symbol_array		(FunctionSymbolArray *a, FunctionSymbol num);
void 			pop_back_function_symbol_array		(FunctionSymbolArray *a);
FunctionSymbol 	get_function_symbol_array			(FunctionSymbolArray *a, usize idx);
void 			clear_function_symbol_array			(FunctionSymbolArray *a);
void 			free_function_symbol_array			(FunctionSymbolArray *a);

#endif