#include "arena_allocator.h"

#include <string.h>
#include <assert.h>
#include <windows.h>
#include <stdio.h>

bool is_power_of_two(usize x) {
	return (x & (x - 1)) == 0;
}

usize align_size(usize size, usize alignment) {
	assert(is_power_of_two(alignment) && "Requested alignment is not power of two!");
	return (size + alignment - 1) & ~(alignment - 1);
}

void init_arena(Arena *a, usize capacity) {
	void *mem 		= (unsigned char *)VirtualAlloc(NULL, capacity, MEM_RESERVE, PAGE_READWRITE);
	if (!mem) {
		DWORD error = GetLastError();
		fprintf(stderr, "VirtualAlloc failed on arena initialization! Error code: %lu\n", error);
		exit(1);
	}
	a->buffer 		= (unsigned char *)mem;
	a->capacity 	= capacity;
	a->curr 		= 0;
	a->committed 	= 0;
}

void *arena_alloc_default(Arena *a, usize size) {
	return arena_alloc_align(a, size, DEFAULT_ALIGNMENT);
}

void *arena_alloc_align(Arena *a, usize size, usize align) {
	usize offset 		= align_size(a->curr, align);
	usize total_alloc 	= offset + size;

	if (total_alloc < a->capacity) {
		if (a->committed < total_alloc) {
			usize to_commit 	= align_size(total_alloc, PAGE_SIZE) - a->committed;
			void *ptr 			= &a->buffer[a->committed];			
			void *commitment 	= VirtualAlloc(ptr, to_commit, MEM_COMMIT, PAGE_READWRITE);
			if (!commitment) {
				DWORD error = GetLastError();
				fprintf(stderr, "VirtualAlloc failed to commit new memory! Error code: %lu\n", error);
				exit(1);
			}
			a->committed = to_commit;
		}
		void *res 	= &a->buffer[offset];
		a->curr 	= total_alloc;
		memset(res, 0, size);
		return res;
	}

	fprintf(stderr, "Arena does not have enough storage!\n");
	return NULL;
}

void arena_clear(Arena *a) {
	a->curr = 0;
}

usize arena_save(Arena *a) {
	return a->curr;
}

void arena_restore_from_save(Arena *a, usize save) {
	usize size = a->curr - save;
	zero_memory(a, save, size);
	a->curr = save;
}

void zero_memory(Arena *a, usize offset, usize size) {
	memset(&a->buffer[offset], 0, size);
}