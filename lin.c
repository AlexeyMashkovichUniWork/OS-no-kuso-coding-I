#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdalign.h>
#include <stdio.h>

#define MSHLIN_ARENA_MAX_SIZE 65536



void * mshlin_arena = NULL;
size_t mshlin_arena_offs = 0;
size_t mshlin_arena_size = 0;


void * mshlin_initArena () {
	size_t arena_size = MSHLIN_ARENA_MAX_SIZE * 2;
	while (!mshlin_arena && arena_size) {
		arena_size /= 2;
		mshlin_arena = malloc(arena_size);
	}
	mshlin_arena_size = arena_size;
	return mshlin_arena;

}

void * mshlin_alloc (size_t size);
void mshlin_free(void * block);

void * mshlin_alloc (size_t size) {
	if (!size) {
		return NULL;
	}
	if (!mshlin_arena) {
		mshlin_initArena();
	}

	int padding = (alignof(max_align_t) - (size % alignof(max_align_t)))
				  % alignof(max_align_t);

	size_t real_size = size + padding;

	if (real_size > (mshlin_arena_size - mshlin_arena_offs)) {
		return NULL;
	}

	void * retptr = (char *)mshlin_arena + mshlin_arena_offs;
	mshlin_arena_offs += real_size;

	return retptr;
}

void mshlin_free (void * block) {}


int main () {
	printf("The test shall commence now.\n");

	uint32_t * tmp = mshlin_alloc(1);
	printf("%p\n", tmp);
	uint32_t * pmt = mshlin_alloc(8);
	printf("%p\n", pmt);
	uint32_t * bo = mshlin_alloc(5);
	printf("%p\n", bo);
	uint32_t * akpfga = mshlin_alloc(210);
	printf("%p\n", akpfga);
	mshlin_free(pmt);
	mshlin_free(akpfga);
	uint32_t * slow_turtle = mshlin_alloc(329);
	printf("%p\n", slow_turtle);
	mshlin_free(slow_turtle);

	uint32_t * colors_on_monitor = mshlin_alloc(16777216);
	printf("%p\n", colors_on_monitor);
	mshlin_free(colors_on_monitor);



	// for (int i = 0; i < 1548 && i < 6645 / sizeof(int); i++) {
	// 	tmp[i] = i;
	// }

	// for (int k = 0; k < 6645; k += 8) {
	// 	for (int i = 0; i < 8 && k + i < 6645; i++) {
	// 		printf("%08x ", tmp[i + k]);
	// 	}
	// 	printf("| \n");
	// }


	return 0;
	
}