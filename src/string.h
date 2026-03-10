#ifndef STRING_H
#define STRING_H

#include "arena_allocator.h"
#include <stdbool.h>

typedef struct String8 String8;
struct String8 {
	char *str;
	usize size;
};

bool 	str8_match		(String8 s1, String8 s2);
String8 str8_concat		(Arena *a, String8 s1, String8 s2);
String8 bool_to_str8	(bool b);
String8 float_to_str8	(Arena *a, float f);
int 	str8_to_float	(String8 s, float *out);

#endif