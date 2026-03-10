#include "usize_array.h"

#include <assert.h>
#include <stdlib.h>

int init_usize_array(UsizeArray *a, usize capacity) {
	a->size 		= 0;
	a->capacity 	= capacity;
	a->data 		= (usize *)malloc(a->capacity * sizeof(usize));
	return a->data ? 0 : -1;
}

int resize_usize_array(UsizeArray *a) {
	usize new_capacity 	= a->capacity * 2 + 1;
	usize *new_data 	= (usize *)realloc(a->data, new_capacity * sizeof(usize));
	if (!new_data) return -1;
	a->data 			= new_data;
	a->capacity 		= new_capacity;
}

int push_back_usize_array(UsizeArray *a, usize num) {
	if (a->size == a->capacity) {
		resize_usize_array(a);
	}
	a->data[a->size] = num;
	a->size++;
	return 0;
}

void pop_back_usize_array(UsizeArray *a) {
	if (a->size > 0) a->size--;
}

usize get_usize_array(UsizeArray *a, usize idx) {
	assert(idx < a->size && idx >= 0);
	return a->data[idx];
}

void clear_usize_array(UsizeArray *a) {
	a->size = 0;
}

void free_usize_array(UsizeArray *a) {
	free(a->data);
	a->data 		= NULL;
	a->size 		= 0;
	a->capacity 	= 0;
}