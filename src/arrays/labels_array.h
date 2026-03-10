#ifndef LABELS_ARRAY_H
#define LABELS_ARRAY_H

#include "../common.h"

typedef struct LabelsArray LabelsArray;
struct LabelsArray {
	int    *data;
	usize 	size;
	usize 	capacity;
};

int 	init_labels		(LabelsArray *l, usize capacity);
int 	resize_labels	(LabelsArray *l, usize new_capacity);
void 	insert_label	(LabelsArray *l, int item, usize idx);
int    	get_label		(LabelsArray *l, usize idx);
void 	clear_labels	(LabelsArray *l);
void 	free_labels		(LabelsArray *l);

#define LABEL_UNDEF (-1)

#endif