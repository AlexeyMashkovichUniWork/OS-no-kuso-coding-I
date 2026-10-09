#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdalign.h>
#include <stdio.h>

#define MSHSTK_ARENA_MAX_SIZE 65536

typedef struct mshstk_header {
	size_t size;
} mshstk_header_t;

void * mshstk_arena = NULL;
size_t mshstk_arena_offs = 0;
size_t mshstk_arena_size = 0;


void * mshstk_initArena () {
	size_t arena_size = MSHSTK_ARENA_MAX_SIZE * 2;
	while (!mshstk_arena && arena_size) {
		arena_size /= 2;
		mshstk_arena = malloc(arena_size);
	}
	mshstk_arena_size = arena_size;
	return mshstk_arena;

}

void * mshstk_alloc (size_t size);
void mshstk_free(void * block);

void * mshstk_alloc (size_t size) {
	if (!size) {
		return NULL;
	}
	if (!mshstk_arena) {
		mshstk_initArena();
	}

	size_t size_with_footer = size + sizeof(mshstk_header_t);
	int paddingB = (alignof(max_align_t) - (size_with_footer % alignof(max_align_t)))
				  % alignof(max_align_t);

	size_t real_size, offs_of_footer;

	offs_of_footer = size + paddingB;
	real_size = size_with_footer + paddingB;

	if (real_size > (mshstk_arena_size - mshstk_arena_offs)) {
		return NULL;
	}

	void * retptr = (char *)mshstk_arena + mshstk_arena_offs;
	mshstk_arena_offs += real_size;

	mshstk_header_t footer = {real_size};

	*(mshstk_header_t * )((char *)retptr + offs_of_footer) = footer;

	return retptr;
}

void mshstk_free (void * block) {
	if (!block) {
		return;
	}

	// is this the last memory block allocated?
	void * cur_ptr = (char *)mshstk_arena + mshstk_arena_offs;
	mshstk_header_t h = *(mshstk_header_t * )((char *)cur_ptr - sizeof(mshstk_header_t));
	if (cur_ptr - h.size == block && cur_ptr - h.size >= mshstk_arena) {
		mshstk_arena_offs -= h.size;
	}

}


int main () {
	printf("The test shall commence now.\n");

	uint32_t * tmp = mshstk_alloc(1);
	printf("%p\n", tmp);
	uint32_t * pmt = mshstk_alloc(8);
	printf("%p\n", pmt);
	uint32_t * bo = mshstk_alloc(5);
	printf("%p\n", bo);
	uint32_t * akpfga = mshstk_alloc(210);
	printf("%p\n", akpfga);
	mshstk_free(pmt);
	printf("%zu\n", mshstk_arena_offs);
	mshstk_free(akpfga);
	printf("%zu\n", mshstk_arena_offs);
	mshstk_free(bo);
	printf("%zu\n", mshstk_arena_offs);
	uint32_t * slow_turtle = mshstk_alloc(329);
	printf("%p\n", slow_turtle);
	mshstk_free(slow_turtle);

	uint32_t * colors_on_monitor = mshstk_alloc(16777216);
	printf("%p\n", colors_on_monitor);
	mshstk_free(colors_on_monitor);



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