#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include "../src/utils.h"

#define TEST(cond) do { 																							\
	if (!(cond)) { 																									\
		printf("%sx Test failed: (%s) \n    File: %s \n    Line: %d%s\n", RED, #cond, __FILE__, __LINE__, WHITE); 	\
			exit(1); 																								\
	} 																												\
} while (0)

#endif