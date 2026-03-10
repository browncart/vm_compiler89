#ifndef USIZE_ARRAY_H
#define USIZE_ARRAY_H

#include "../common.h"

#include <string.h>

typedef struct UsizeArray UsizeArray;
struct UsizeArray {
	usize  *data;
	usize 	size;
	usize 	capacity;
};

int 	init_usize_array				(UsizeArray *a, usize capacity);
int 	resize_usize_array				(UsizeArray *a);
int 	push_back_usize_array			(UsizeArray *a, usize num);
void 	pop_back_usize_array			(UsizeArray *a);
usize 	get_usize_array					(UsizeArray *a, usize idx);
void 	clear_usize_array				(UsizeArray *a);
void 	free_usize_array				(UsizeArray *a);

#endif