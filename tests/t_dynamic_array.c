#include "t_dynamic_array.h"

#include "test_utils.h"
#include "../src/arrays/dynamic_array.h"

#include <stdio.h>
#include <stdlib.h>

int t_dynamic_array() {
	DArray d = {0};
	init_darray(&d, sizeof(int), 2);

	int one = 1;
	int two = 2;
	int thr = 3;

	push_back_darray(&d, &one);

	TEST(*(int *)get_darray(&d, 0) == one);

	int *new_int = (int *)emplace_back_darray(&d);
	
	*new_int = 40;

	TEST(*(int *)get_darray(&d, 1) == 40);
	TEST(d.size == 2 && d.capacity == 2);

	push_back_darray(&d, &two);

	TEST(d.capacity == 5);
	TEST(d.size == 3);

	TEST(*(int *)get_darray(&d, 2) == two);
	pop_back_darray(&d);

	TEST(get_darray(&d, 2) == NULL);
	TEST(d.size == 2);

	clear_darray(&d);

	TEST(d.size == 0);

	free_darray(&d);
	init_darray(&d, sizeof(int), 2);

	int i;
	for (i = 0; i < 1000; ++i) {
		int *i_val = (int *)emplace_back_darray(&d);
		*i_val = i;
	}

	for (i = 0; i < 1000; ++i) {
		int *i_val = (int *)get_darray(&d, i);
		TEST(*i_val == i);
	}
	TEST(d.size == 1000);

	clear_darray(&d);

	for (i = 0; i < 1000; ++i) {
		push_back_darray(&d, &i);
	}

	for (i = 0; i < 1000; ++i) {
		int *i_val = (int *)get_darray(&d, i);
		TEST(*i_val == i);
	}
	TEST(d.size == 1000);

	printf("%so Dynamic Array tests passed!%s\n", GREEN, WHITE);
	return 0;
}