#include "t_hash_table.h"
#include "../src/utils.h"
#include "test_utils.h"
#include "../src/hash_table.h"

#include <stdio.h>
#include <stdlib.h>

int t_hash_table() {
	int exp = 1;

	HashTable ht = {0};
	init_ht(&ht, exp, sizeof(String8));

	String8 hello = {
		.str = "Hello",
		.size = 5
	};

	String8 goodbye = {
		.str = "Goodbye",
		.size = 7
	};

	String8 *hibye = (String8 *)upsert_ht(&ht, hello);
	*hibye = goodbye;

	String8 *byehi = (String8 *)upsert_ht(&ht, goodbye);
	*byehi = hello;

	TEST(str8_match(hello, *(String8 *)find_ht(&ht, goodbye)));
	TEST(str8_match(goodbye, *(String8 *)find_ht(&ht, hello)));
	TEST(ht.size == 2);

	remove_ht(&ht, hello);

	TEST(ht.tomb == 1);
	TEST(ht.size == 2);

	String8 deleted = {
		.str = "(deleted)",
		.size = 5
	};

	String8 *del = (String8 *)upsert_ht(&ht, deleted);
	*del = deleted;

	TEST(str8_match(deleted, *(String8 *)find_ht(&ht, deleted)));

	TEST(ht.capacity == 1 << exp + 1);
	TEST(ht.tomb == 0);
	TEST(ht.size == 2);
	
	TEST(str8_match(deleted, *(String8 *)find_ht(&ht, deleted)));

	printf("%so Hash Table tests passed!%s\n", GREEN, WHITE);
	return 0;
}