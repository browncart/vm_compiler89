#ifndef VALUE_ARRAY_H
#define VALUE_ARRAY_H

#include "../common.h"
#include "../models.h"

#include <string.h>

typedef struct ValueArray ValueArray;
struct ValueArray {
	Value *data;
	usize 	size;
	usize 	capacity;
};

int 	init_value_array			(ValueArray *a, usize capacity);
int 	resize_value_array			(ValueArray *a);
int 	push_back_value_array		(ValueArray *a, Value num);
void 	pop_back_value_array		(ValueArray *a);
Value 	get_value_array				(ValueArray *a, usize idx);
Value  *get_pointer_value_array		(ValueArray *s, usize idx);
void 	clear_value_array			(ValueArray *a);
void 	free_value_array			(ValueArray *a);

#endif