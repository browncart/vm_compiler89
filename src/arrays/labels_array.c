#include "labels_array.h"

#include <stdio.h>
#include <stdlib.h>

int init_labels(LabelsArray *l, usize capacity) {	
	l->capacity 	= capacity;
	l->data 		= (int *)malloc(l->capacity * sizeof(int));
	int i;
	for (i = 0; i < capacity; ++i) {
		l->data[i] = LABEL_UNDEF;
	}
	return l->data ? 0 : -1;
}

int resize_labels(LabelsArray *l, usize new_cap) {
	usize old_cap 	= l->capacity;
	int *new_data 	= (int *)realloc(l->data, new_cap * sizeof(int));
	if (!new_data) return -1;
	int i = old_cap;
	for (; i < new_cap; ++i) {
		new_data[i] = LABEL_UNDEF;
	}	
	l->data 		= new_data;
	l->capacity 	= new_cap;
}

void insert_label(LabelsArray *l, int item, usize idx) {
	if (l->capacity <= idx) {
		resize_labels(l, idx + 1);
	}
	l->data[idx] = item;
}

int get_label(LabelsArray *l, usize idx) {
	if (idx >= l->capacity || l->data[idx] == -1) return -1;
	return l->data[idx];
}

void clear_labels(LabelsArray *l) {
	int i;
	for (i = 0; i < l->capacity; ++i) {
		l->data[i] = LABEL_UNDEF;
	}
}

void free_labels(LabelsArray *l) {
	free(l->data);
	l->data 		= NULL;
	l->capacity 	= 0;
}