#include "dynamic_array.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int init_darray(DArray *d, usize elem_size, usize capacity) {
	d->elem_size 	= elem_size;
	d->size 		= 0;
	d->capacity 	= capacity;
	d->data 		= malloc(d->capacity * elem_size);
	return d->data ? 0 : -1;
}

int resize_darray(DArray *d) {
	int new_capacity 	= d->capacity * 2 + 1;
	void *new_data 		= realloc(d->data, new_capacity * d->elem_size);
	if (!new_data) return -1;
	d->data 			= new_data;
	d->capacity 		= new_capacity;
}

void insert_index_darray(DArray *d, void *item, usize idx) {
	if (d->capacity <= idx) {
		resize_darray(d);
	}
	void *old = get_darray(d, idx);
	memcpy(old, item, d->elem_size);
}

int push_back_darray(DArray *d, void *item) {
	if (d->size == d->capacity) {
		resize_darray(d);
	}
	memcpy((unsigned char *)d->data + (d->size * d->elem_size), item, d->elem_size);
	d->size++;
	return 0;
}

void *push_back_and_get_darray(DArray *d, void *item) {
	if (d->size == d->capacity) {
		resize_darray(d);
	}
	unsigned char *update_addr = (unsigned char *)d->data + (d->size * d->elem_size);
	memcpy(update_addr, item, d->elem_size);
	d->size++;
	return update_addr;
}

void *emplace_back_darray(DArray *d) {
	if (d->size == d->capacity) {
		resize_darray(d);
	}
	void *mem = (unsigned char *)d->data + (d->size * d->elem_size);
	d->size++;
	return mem;
}

void pop_back_darray(DArray *d) {
	if (d->size > 0) d->size--;
}

void *get_darray(DArray *d, usize idx) {
	if (idx >= d->size) return NULL;
	return (unsigned char *)d->data + (idx * d->elem_size);
}

void clear_darray(DArray *d) {
	d->size = 0;
}

void free_darray(DArray *d) {
	free(d->data);
	d->data 		= NULL;
	d->size 		= 0;
	d->capacity 	= 0;
	d->elem_size 	= 0;
}