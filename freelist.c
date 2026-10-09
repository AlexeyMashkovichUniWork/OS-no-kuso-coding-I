#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdalign.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

#define MSHLIST_ARENA_MAX_SIZE 65536

#define MSHLIST_BIT_OCCUPIED(header) ((header).size & 1)
#define MSHLIST_SIZE(header) ((header).size & ~(size_t)1)

#define MSHLIST_HEADER_PADDING ((alignof(max_align_t) - (sizeof(mshlist_header_t) % alignof(max_align_t))) % alignof(max_align_t))

typedef struct mshlist_header {
	size_t size;
} mshlist_header_t;

void * mshlist_arena = NULL;
size_t mshlist_arena_size = 0;


void * mshlist_initArena () {
	size_t arena_size = MSHLIST_ARENA_MAX_SIZE * 2;
	while (!mshlist_arena && arena_size) {
		arena_size /= 2;
		mshlist_arena = malloc(arena_size);
	}

	size_t footer_offset = arena_size - sizeof(mshlist_header_t);
	mshlist_header_t h2 = {arena_size | 0};
	*(mshlist_header_t *)(mshlist_arena + 0) = h2;
	*(mshlist_header_t *)(mshlist_arena + footer_offset) = h2;

	mshlist_arena_size = arena_size;
	return mshlist_arena;
}

void * mshlist_alloc (size_t size);
void mshlist_free(void * block);
void mshlist_coalesce (void);

void * mshlist_alloc (size_t size) {
	if (!size) {
		return NULL;
	}
	if (!mshlist_arena) {
		mshlist_initArena();
	}

	char * run_ptr = mshlist_arena;
	mshlist_header_t h;

	size_t real_min_size;

	{	// todo: refactor out to dedicated function


		size_t size_with_header_and_footer = size + 2 * sizeof(mshlist_header_t) + MSHLIST_HEADER_PADDING;

		int footer_padding = (alignof(max_align_t) - (size_with_header_and_footer % alignof(max_align_t)))
				  % alignof(max_align_t);

		real_min_size = size_with_header_and_footer + footer_padding;
	}
	// todo: DRY
	while (run_ptr < ((char *)mshlist_arena + mshlist_arena_size)) {
		// check size
		h = *(mshlist_header_t *)run_ptr;
		// printf("AAAAAA %p\n", run_ptr);
		// usleep(1000000);
		if (MSHLIST_BIT_OCCUPIED(h) || MSHLIST_SIZE(h) < real_min_size) {
			// skip period
			run_ptr += MSHLIST_SIZE(h);
			continue;
		}
		break;
	}
	// todo: DRY
	if (run_ptr >= ((char *)mshlist_arena + mshlist_arena_size)) {
		return NULL;
	}

	size_t real_size;

	// minimum wage
	if (MSHLIST_SIZE(h) < real_min_size + (2 * sizeof(mshlist_header_t))) {
		real_size = real_min_size + (2 * sizeof(mshlist_header_t)) + (2 * MSHLIST_HEADER_PADDING);
	} else {
		real_size = real_min_size;
	}

	// TODO refactor this kuso out as well
	size_t header_offset = 0;	// i trust the compiler too much
	size_t out_mem_offset = sizeof(mshlist_header_t) + MSHLIST_HEADER_PADDING;
	size_t footer_offset = real_size - sizeof(mshlist_header_t);
	mshlist_header_t h2 = {real_size | 1};
	*(mshlist_header_t *)(run_ptr + header_offset) = h2;
	*(mshlist_header_t *)(run_ptr + footer_offset) = h2;

	char * next_block_start = run_ptr + real_size;
	mshlist_header_t h3 = {MSHLIST_SIZE(h) - real_size};
	size_t footer_offset2 = MSHLIST_SIZE(h3) - sizeof(mshlist_header_t);
	*(mshlist_header_t *)(next_block_start + header_offset) = h3;
	*(mshlist_header_t *)(next_block_start + footer_offset2) = h3;

	return run_ptr + out_mem_offset;

}

void mshlist_free (void * block) {
	if (!block || !mshlist_arena) {
		return;
	}

	char * begin_internal_block = (char *)block - MSHLIST_HEADER_PADDING - sizeof(mshlist_header_t);

	mshlist_header_t h = *(mshlist_header_t *)begin_internal_block;

	if (!MSHLIST_BIT_OCCUPIED(h)) {
		return;
	}

	// we do it for cheap
	h.size &= ~(size_t)1;

	// todo: dry refactor
	size_t header_offset = 0;	// i trust the compiler too much
	size_t footer_offset = MSHLIST_SIZE(h) - sizeof(mshlist_header_t);
	*(mshlist_header_t *)(begin_internal_block + header_offset) = h;
	*(mshlist_header_t *)(begin_internal_block + footer_offset) = h;

	// mshlist_coalesce();
}


void mshlist_coalesce (void) {
	if (!mshlist_arena) {
		return;
	}

	char * run_ptr = mshlist_arena;
	mshlist_header_t h, h2;

	// todo: DRY
	while (run_ptr < ((char *)mshlist_arena + mshlist_arena_size)) {
		// check size
		h = *(mshlist_header_t *)run_ptr;
		if (MSHLIST_BIT_OCCUPIED(h)) {
			// skip period
			run_ptr += MSHLIST_SIZE(h);
			continue;
		}

		char * next_block = run_ptr + MSHLIST_SIZE(h);
		h2 = *(mshlist_header_t *)next_block;

		if (MSHLIST_BIT_OCCUPIED(h2)) {
			// skip period
			run_ptr += MSHLIST_SIZE(h);
			run_ptr += MSHLIST_SIZE(h2);
			continue;
		}

		h.size = MSHLIST_SIZE(h) + MSHLIST_SIZE(h2);

		size_t footer_offset = MSHLIST_SIZE(h) - sizeof(mshlist_header_t);

		*(mshlist_header_t *)run_ptr = h;
		*(mshlist_header_t *)(run_ptr + footer_offset) = h;
	}
}

int main () {
	printf("The test shall commence now.\n");

	uint32_t * tmp = mshlist_alloc(1);
	printf("%p\n", tmp);
	uint32_t * pmt = mshlist_alloc(8);
	printf("%p\n", pmt);
	uint32_t * bo = mshlist_alloc(5);
	printf("%p\n", bo);
	mshlist_free(pmt);
	uint32_t * akpfga = mshlist_alloc(2);
	printf("%p\n", akpfga);
	mshlist_free(akpfga);
	mshlist_free(bo);
	uint32_t * slow_turtle = mshlist_alloc(329);
	printf("%p\n", slow_turtle);
	mshlist_free(slow_turtle);

	uint32_t * colors_on_monitor = mshlist_alloc(16777216);
	printf("%p\n", colors_on_monitor);
	mshlist_free(colors_on_monitor);



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