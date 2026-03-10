#ifndef ARENA_ALLOCATOR_H
#define ARENA_ALLOCATOR_H

#include "common.h"

#include <stdint.h>
#include <stdbool.h>

#ifndef DEFAULT_ALIGNMENT
#define DEFAULT_ALIGNMENT sizeof(void *)
#endif

#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif

typedef uintptr_t uptr;

typedef struct Arena Arena;
struct Arena {
	unsigned char *buffer;
	usize capacity;
	
	usize committed;
	usize curr;
};

void 	init_arena				(Arena *a, usize capacity);
bool 	is_power_of_two			(usize x);
usize 	align_size				(usize size, usize alignment);
void   *arena_alloc_default		(Arena *a, usize size);
void   *arena_alloc_align		(Arena *a, usize size, usize align);
void 	arena_clear				(Arena *a);
size_t 	arena_save				(Arena *a);
void 	arena_restore_from_save	(Arena *a, usize save);
void 	zero_memory				(Arena *a, usize offset, usize size);

#endif