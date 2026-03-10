#include "hash_table.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static char *tombstone = "(deleted)";

s64 hash64(String8 key) {
	u64 hash = 0xcbf29ce484222325;
	int i;
	for (i = 0; i < key.size; ++i) {
		hash ^= key.str[i] & 255;
		hash *= 0x100000001b3;
	}
	return hash;
}

void init_ht(HashTable *ht, u32 exp, usize elem_size) {
	ht->capacity 	= 1 << exp;
	ht->elem_size 	= elem_size;
	ht->keys 		= (String8 *)calloc(ht->capacity, sizeof(String8));
	ht->values 		= (unsigned char *)calloc(ht->capacity, elem_size);
	ht->tomb 		= 0;
	ht->size 		= 0;
	ht->exp 		= exp;
}

void destroy_ht(HashTable *ht) {
	free(ht->keys);
	free(ht->values);
}

void resize_ht(HashTable *ht) {
	double full = (double)ht->size / ht->capacity;
	if (full >= .7) {
		String8 	   *old_keys 	= ht->keys;
		unsigned char  *old_vals 	= ht->values;
		usize 			old_cap		= ht->capacity;

		init_ht(ht, ht->exp + 1, ht->elem_size);

		usize i;
		for (i = 0; i < old_cap; ++i) {
			String8 key = old_keys[i];
			if (key.str && key.str != tombstone) {
				void *val = upsert_ht(ht, key);
				memcpy(val, &old_vals[i], ht->elem_size);
			}
		}

		free(old_keys);
		free(old_vals);
	}
}

void *upsert_ht(HashTable *ht, String8 key) {
	resize_ht(ht);

	s64 hash = hash64(key);
	int mask = (1 << ht->exp) - 1;
	int step = (hash >> (64 - ht->exp)) | 1;

	int tombstone_idx = -1;
	int i = hash & mask;

	for (;;) {
		if (!ht->keys[i].str) {
			if (tombstone_idx >= 0) {
				i = tombstone_idx;
			} else {
				ht->size++;
			}
			ht->keys[i] = key;
			return (unsigned char *)(ht->values + i * ht->elem_size);
		}

		if (str8_match(ht->keys[i], key)) {
			return (unsigned char *)(ht->values + i * ht->elem_size);
		}

		if (ht->keys[i].str == tombstone && tombstone_idx < 0) {
			tombstone_idx = i;
		}

		i = (i + step) & mask;
	}
	return NULL;
}

void *find_ht(HashTable *ht, String8 key) {
	s64 hash = hash64(key);
	int mask = (1 << ht->exp) - 1;
	int step = (hash >> (64 - ht->exp)) | 1;

	int i = hash & mask;
	for (;;) {
		if (!ht->keys[i].str) {
			return NULL;
		}

		if (str8_match(ht->keys[i], key)) {
			return ht->values + (i * ht->elem_size);
		}
		
		i = (hash + step) & mask;
	}
	return NULL;
}

int count_ht(HashTable *ht, String8 key) {
	s64 hash = hash64(key);
	int mask = (1 << ht->exp) - 1;
	int step = (hash >> (64 - ht->exp)) | 1;

	int i = hash & mask;
	for (;;) {
		if (!ht->keys[i].str) {
			return 0;
		}

		if (str8_match(ht->keys[i], key)) {
			return 1;
		}

		i = (hash + step) & mask;
	}
}

void remove_ht(HashTable *ht, String8 key) {
	s64 hash = hash64(key);
	int mask = (1 << ht->exp) - 1;
	int step = (hash >> (64 - ht->exp)) | 1;

	int i = hash & mask;
	for (;;) {
		if (!ht->keys[i].str) {
			return;
		}

		if (str8_match(ht->keys[i], key)) {		
			ht->keys[i].str = tombstone;
			ht->tomb++;
			return;
		}

		i = (hash + step) & mask;
	}
}