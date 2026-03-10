#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H

#include "../common.h"

#include <string.h>

typedef struct DArray DArray;
struct DArray {
	void   *data;
	usize 	elem_size;
	usize 	size;
	usize 	capacity;
};

int 	init_darray					(DArray *d, usize elem_size, usize capacity);
int 	resize_darray				(DArray *d);
void 	insert_index_darray			(DArray *d, void *item, usize idx);
int 	push_back_darray			(DArray *d, void *item);
void   *push_back_and_get_darray	(DArray *d, void *item);
void   *emplace_back_darray			(DArray *d);
void 	pop_back_darray				(DArray *d);
void   *get_darray					(DArray *d, usize idx);
void 	clear_darray				(DArray *d);
void 	free_darray					(DArray *d);

#define INSERT_DARRAY(darray, item, idx, type) 				\
	do { 													\
		type _tmp = (item); 								\
		type *mem = (type *)insert_idx((darray), (idx));	\
		if (mem) *mem = (item);								\
	} while(0)

#define PUSH_DARRAY(darray, item, type) 	\
	do { 									\
		type _tmp = (item); 				\
		push_back_darray((darray), &_tmp); 	\
	} while(0)

#define PUSH_AND_GET_DARRAY(darray, item, type) 			\
	do { 													\
		type _tmp = (item); 								\
		return push_back_and_get_darray((darray), &_tmp); 	\
	} while(0)

#define POP_DARRAY(darray, out, type) 								\
	do { 															\
		type *pop_val = get_darray((darray), (darray)->size - 1);	\
		memcpy((out), pop_val, sizeof(type));						\
		pop_back_darray((darray));									\
	} while(0)

#endif