#include "label_fixup_array.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int init_label_fixup_array(LabelFixupArray *a, usize capacity) {
	a->size 		= 0;
	a->capacity 	= capacity;
	a->data 		= (LabelFixup *)malloc(a->capacity * sizeof(LabelFixup));
	return a->data ? 0 : -1;
}

int resize_label_fixup_array(LabelFixupArray *a) {
	int new_capacity 		= a->capacity * 2 + 1;
	LabelFixup *new_data 	= (LabelFixup *)realloc(a->data, new_capacity * sizeof(LabelFixup));
	if (!new_data) return -1;
	a->data 				= new_data;
	a->capacity 			= new_capacity;
}

int push_back_label_fixup_array(LabelFixupArray *a, LabelFixup fixup) {
	if (a->size == a->capacity) {
		resize_label_fixup_array(a);
	}
	a->data[a->size] = fixup;
	a->size++;
	return 0;
}

void pop_back_label_fixup_array(LabelFixupArray *a) {
	if (a->size > 0) a->size--;
}

LabelFixup get_label_fixup_array(LabelFixupArray *a, usize idx) {
	assert(idx < a->size && idx >= 0);
	return a->data[idx];
}

void clear_label_fixup_array(LabelFixupArray *a) {
	a->size = 0;
}

void free_label_fixup_array(LabelFixupArray *a) {
	free(a->data);
	a->data 		= NULL;
	a->size 		= 0;
	a->capacity 	= 0;
}