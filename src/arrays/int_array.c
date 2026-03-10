#include "int_array.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int init_int_array(IntArray *i, usize capacity) {
	i->size 		= 0;
	i->capacity 	= capacity;
	i->data 		= (int *)malloc(i->capacity * sizeof(int));
	return i->data ? 0 : -1;
}

int resize_int_array(IntArray *i) {
	int new_capacity 	= i->capacity * 2 + 1;
	int *new_data 		= (int *)realloc(i->data, new_capacity * sizeof(int));
	if (!new_data) return -1;
	i->data 			= new_data;
	i->capacity 		= new_capacity;
}

int push_back_int_array(IntArray *i, int num) {
	if (i->size == i->capacity) {
		resize_int_array(i);
	}
	i->data[i->size] = num;
	i->size++;
	return 0;
}

void pop_back_int_array(IntArray *i) {
	if (i->size > 0) i->size--;
}

int get_int_array(IntArray *i, usize idx) {
	assert(idx < i->size && idx >= 0);
	return i->data[idx];
}

void clear_int_array(IntArray *i) {
	i->size = 0;
}

void free_int_array(IntArray *i) {
	free(i->data);
	i->data 		= NULL;
	i->size 		= 0;
	i->capacity 	= 0;
}