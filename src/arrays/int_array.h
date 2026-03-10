#ifndef INT_ARRAY_H
#define INT_ARRAY_H

#include "../common.h"

#include <string.h>

typedef struct IntArray IntArray;
struct IntArray {
	int    *data;
	usize 	size;
	usize 	capacity;
};

int 	init_int_array					(IntArray *i, usize capacity);
int 	resize_int_array				(IntArray *i);
int 	push_back_int_array				(IntArray *i, int num);
void 	pop_back_int_array				(IntArray *i);
int 	get_int_array					(IntArray *i, usize idx);
void 	clear_int_array					(IntArray *i);
void 	free_int_array					(IntArray *i);

#endif