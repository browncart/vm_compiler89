#include "string.h"
#include "arena_allocator.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>
#include <string.h>

bool str8_match(String8 s1, String8 s2) {
	if (s1.size != s2.size) return false;

	return !s1.size || !memcmp(s1.str, s2.str, s1.size);
}

String8 str8_concat(Arena *a, String8 s1, String8 s2) {
	size_t new_size = s1.size + s2.size;

	char *buffer = (char *)(arena_alloc_align(a, new_size + 1, sizeof(char)));
	buffer[new_size] = '\0';

	int i;
	for (i = 0; i < s1.size; ++i) {
		buffer[i] = s1.str[i];
	}
	for (i = 0; i < s2.size; ++i) {
		buffer[i + s1.size] = s2.str[i];
	}

	return (String8) {buffer, new_size};
}

String8 bool_to_str8(bool b) {
	if (b)
		return (String8) {(char *)"true", 4};
	else
		return (String8) {(char *)"false", 5};
}

String8 float_to_str8(Arena *a, float f) {
	size_t size = snprintf(NULL, 0, "%g", f);
	
	char *buffer = (char *)(arena_alloc_align(a, size + 1, sizeof(char)));
	snprintf(buffer, size + 1, "%g", f);

	String8 res = (String8) {
		.str 	= buffer,
		.size 	= size
	};

	return res;
}

int str8_to_float(String8 s, float *out) {
	char *end;
	errno = 0;

	char buffer[s.size + 1];
	buffer[s.size] = '\0';

	int i;
	for (i = 0; i < s.size; ++i) {
		buffer[i] = s.str[i];
	}

	*out = strtof(buffer, &end);

	if (errno == ERANGE) {
        fprintf(stderr, "Value out of range for float!");
		*out = 0.0f;
		return -1;
	} 
	
	if (buffer == end) {
		fprintf(stderr, "Couldn't parse string into float!\n");
		*out = 0.0f;
		return -2;
	}

	return 0;
}