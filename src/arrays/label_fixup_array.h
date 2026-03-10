#ifndef LABEL_FIXUP_ARRAY_H
#define LABEL_FIXUP_ARRAY_H

#include "../common.h"
#include "../models.h"

#include <string.h>

typedef struct LabelFixupArray LabelFixupArray;
struct LabelFixupArray {
	LabelFixup *data;
	usize 		size;
	usize 		capacity;
};

int 			init_label_fixup_array			(LabelFixupArray *a, usize capacity);
int 			resize_label_fixup_array		(LabelFixupArray *a);
int 			push_back_label_fixup_array		(LabelFixupArray *a, LabelFixup fixup);
void 			pop_back_label_fixup_array		(LabelFixupArray *a);
LabelFixup 		get_label_fixup_array			(LabelFixupArray *a, usize idx);
void 			clear_label_fixup_array			(LabelFixupArray *a);
void 			free_label_fixup_array			(LabelFixupArray *a);

#endif