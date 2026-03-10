#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include "common.h"
#include "string.h"

typedef struct HashTable HashTable;
struct HashTable {
	String8 *keys;
	unsigned char *values;

	usize tomb;
	usize size;
	usize capacity;
	usize elem_size;
	u32 exp;
};

s64 hash64(String8 key);

void 	init_ht		(HashTable *ht, u32 exp, usize elem_size);
void 	destroy_ht	(HashTable *ht);
void 	resize_ht	(HashTable *ht);
void   *upsert_ht	(HashTable *ht, String8 key);
void   *find_ht		(HashTable *ht, String8 key);
int 	count_ht	(HashTable *ht, String8 key);
void 	remove_ht	(HashTable *ht, String8 key);

#endif