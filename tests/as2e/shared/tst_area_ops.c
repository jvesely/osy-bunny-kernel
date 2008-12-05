/***
 * Area operations.
 *
 * A set of operations performed on the list of virtual memory areas.
 *
 * Change Log:
 * 2004/12/09 pallas	created
 */

#include "../include/tst_area_ops.h"
#include "../include/tst_rand.h"
#include "test.h"


/****************************************************************************\
| PRIVATE DEFS                                                               |
\****************************************************************************/

#define PRINT_ACTION_COMMON_HEADER 					\
	printk ("<%d,%d> .. cycle %d (actionno: %d) -- ", 		\
		GET_GROUP_ID (grpthrid), GET_THREAD_ID (grpthrid), 	\
		cycles, actionno)


/****************************************************************************\
| PUBLIC DATA                                                                |
\****************************************************************************/

/*
 * Global flag which is set when an error is encountered
 * (overlapping blocks, incorrect data in a block, etc.).
 */
volatile int 		error_flag = 0;


/****************************************************************************\
| PRIVATE CODE (MISC)                                                        |
\****************************************************************************/

/**
 * result_name
 * @result:	result to translate
 *
 * A human-friendly result name.
 */
static char *
result_name (int result)
{
	char * name;

	switch (result) {
	case EOK:
		name = "EOK";
		break;

	case EINVAL:
		name = "EINVAL";
		break;

	case ENOMEM:
		name = "ENOMEM";
		break;

	default:
		name = "UNKNOWN";
	}

	return name;
} /* result_name */


/**
 * within_range
 *
 * Check if a page number and size in pages are still withing
 * an allowed page range.
 */
static inline int
within_range (int page, int size)
{
	int page_start = ALLOWED_RANGE_START / PAGE_SIZE;
	int page_count = ALLOWED_RANGE_SIZE / PAGE_SIZE;

	if (page < page_start)
		return 0;

	if (page + size >= page_start + page_count)
		return 0;

	return 1;
} /* within_range */


/****************************************************************************\
| PRIVATE CODE (MEMORY)                                                      |
\****************************************************************************/

/**
 * expected_value
 *
 * Computes the expected value of a byte located at position pos.
 */
static inline unsigned char
expected_value (struct phase * phase, tst_area_list_s * mem, void * pos)
{
	/*
	 * Phase is not used since data from different phases would differ,
	 * which is not what we actually want.
	 */
	return ((unsigned int) mem ^ (unsigned int) pos) & 0xff;
} /* expected_value */


/**
 * fill_block
 *
 * Fills a block of memory with precomputed values.
 */
static void
fill_block (struct phase * phase, tst_area_list_s *mem, void *start, size_t size)
{
	unsigned char * pos = (unsigned char *) start;

	while (size-- > 0) {
		(* pos) = expected_value (phase, mem, pos);
		pos++;
	}
} /* fill_block */


/**
 * verify_block
 *
 * Verifies that a block of memory contains expected values.
 */
static int
verify_block (struct phase * phase, tst_area_list_s *mem, void *start, size_t size)
{
	unsigned char * pos = (unsigned char *) start;

	while (size-- > 0) {
		if ((* pos) != expected_value (phase, mem, pos))
			return 0;

		pos++;
	}

	return 1;
} /* verify_block */


/****************************************************************************\
| PRIVATE CODE (RANDOM)                                                      |
\****************************************************************************/

/**
 * random_range
 *
 * Generates a random number in the range [start,end).
 */
static inline unsigned int
random_range (unsigned int start, unsigned int end)
{
	return start + tst_rand () % (end - start);
} /* random_range */


/**
 * random_page
 *
 * Returns a random page number in the allowed page range.
 */
static inline unsigned int
random_page (void)
{
	int page_start = ALLOWED_RANGE_START / PAGE_SIZE;
	int page_count = ALLOWED_RANGE_SIZE / PAGE_SIZE;

	return random_range (page_start, page_start + page_count);
} /* random_page */


/**
 * random_size
 *
 * Returns a random size in pages in range [min,max].
 */
static inline int
random_size (int min, int max)
{
	return random_range (min, max + 1);
} /* random_size */


/**
 * random_ptr
 *
 * Returns a random pointer to a memory location given by
 * starting page and size in pages.
 */
static inline unsigned char *
random_ptr (int page, int size)
{
	return (unsigned char *) (page * PAGE_SIZE + (tst_rand () % (size * PAGE_SIZE)));
} /* random_ptr */


/****************************************************************************\
| PUBLIC CODE (VMA_ALLOC)                                                    |
\****************************************************************************/

/**
 * action_vma_alloc
 *
 * Allocates an area in the designated segment using the VF_VA_AUTO flag,
 * which means that the action should always succeed unless there is no
 * memory available.
 */
void
action_vma_alloc (
	struct phase *		phase,
	tst_area_list_s *	mem,
	struct action_param *	data,
	grpthr_id_t 		grpthrid,
	int 			cycles,
	int 			actionno
){
	/*
	 * Lock the list of areas.
	 */
	tst_area_list_lock (mem);


	/*
	 * Allocate a random-sized virtual memory area.
	 */
	int alloc_size = random_size (data->min_block_size, data->max_block_size);
	size_t alloc_bytes = alloc_size * PAGE_SIZE;

	LOCK_CONSOLE;
		PRINT_ACTION_COMMON_HEADER;
		printk ("vma_alloc (size=%x)", alloc_bytes);

		void * alloc_ptr;
		int alloc_res = vma_alloc (& alloc_ptr, alloc_bytes, ALLOWED_RANGE_VFLAGS);
		printk (" = %s, ptr=%p\n", result_name (alloc_res), alloc_ptr);

		/*
		 * If the allocation failed, we just shrug our shoulders
		 * and return without making much fuss around it.
		 */
		if (alloc_res != EOK)
			goto exit_unlock_both;

		/*
		 * The allocation succeeded, check if the pointer is
		 * aligned to PAGE_SIZE.
		 */
		if ((unsigned int) alloc_ptr % PAGE_SIZE != 0) {
			printk ("<%d,%d>   ..  Error: Unaligned pointer "
				"returned by vma_alloc.\n", GET_GROUP_ID (grpthrid),
				GET_THREAD_ID (grpthrid));

			error_flag = 1;
			goto exit_unlock_both;
		}

		/*
		 * The pointer is OK, convert it to page number, add
		 * the area to the area list and check for overlaps.
		 */
		int alloc_page = (unsigned int) alloc_ptr / PAGE_SIZE;
		int expect_res = tst_area_list_add (mem, alloc_page, alloc_size);

		/*
		 * Check if there is an overlap of areas...
		 */
		if (expect_res == EINVAL) {
			/*
			 * There is an overlap of areas...
			 */
			printk ("<%d,%d>   ..  Error: Pointer to occupied "
				"memory returned by vma_alloc.\n",
				GET_GROUP_ID (grpthrid),
				GET_THREAD_ID (grpthrid));

			error_flag = 1;
			goto exit_unlock_both;
		}

		/*
		 * If there was not enough memory to create a new list item,
		 * just free the allocated area, but don't report an error.
		 */
		if (expect_res == ENOMEM) {
			PRINT_ACTION_COMMON_HEADER;
			printk ("consistency vma_free (ptr=%p)", alloc_ptr);
			vma_free (alloc_ptr);
			goto exit_unlock_both;
		}

		/*
		 * If everything went smoothly, fill the allocated memory
		 * with a pattern that can be later checked.
		 */
		fill_block (phase, mem, alloc_ptr, alloc_bytes);

exit_unlock_both:
	UNLOCK_CONSOLE;


	/*
	 * Unlock the area list.
	 */
	tst_area_list_unlock (mem);
} /* action_vma_alloc */


/**
 * action_vma_alloc_rand
 *
 * Randomly picks a page in the virtual memory and tries to allocate memory
 * starting from it.
 */
void
action_vma_alloc_rand (
	struct phase *		phase,
	tst_area_list_s * 	mem,
	struct action_param *	data,
	grpthr_id_t 		grpthrid,
	int 			cycles,
	int 			actionno
){
	/*
	 * Lock the list of areas.
	 */
	tst_area_list_lock (mem);


	/*
	 * Allocate a random-sized virtual memory area with random
	 * start address.
	 */
	unsigned int alloc_page = random_page ();
	void * alloc_ptr = (void *) (alloc_page * PAGE_SIZE);
	int alloc_size = random_size (data->min_block_size, data->max_block_size);
	size_t alloc_bytes = alloc_size * PAGE_SIZE;

	LOCK_CONSOLE;
		PRINT_ACTION_COMMON_HEADER;
		printk ("vma_alloc_rand (ptr=%p, size=%x)", alloc_ptr, alloc_bytes);

		int alloc_res = vma_alloc (&alloc_ptr, alloc_bytes, VF_USER_ADDR);
		printk (" = %s, ptr=%p\n", result_name (alloc_res), alloc_ptr);

		/*
		 * If there was not enough memory to allocate the area,
		 * we just return without reporting an error.
		 */
		if (alloc_res == ENOMEM)
			goto exit_unlock_both;

		/*
		 * In other cases, the result of vma_alloc should be the same
		 * as the result of tst_area_list_add, except when the result
		 * is ENOMEM, in which case we just make a consistency vma_free.
		 */
		int expect_res = tst_area_list_add (mem, alloc_page, alloc_size);
		if (expect_res == ENOMEM) {
			PRINT_ACTION_COMMON_HEADER;
			printk ("consistency vma_free (ptr=%p)", alloc_ptr);
			vma_free (alloc_ptr);
			goto exit_unlock_both;
		}

		if (alloc_res != expect_res) {
			/*
			 * There must have been an overlap.
			 */
			printk ("<%d,%d>   ..  Error: Pointer to occupied "
				"memory returned by vma_alloc.\n",
				GET_GROUP_ID (grpthrid),
				GET_THREAD_ID (grpthrid));

			error_flag = 1;
			goto exit_unlock_both;
		}

		/*
		 * If everything went smoothly, check that vma_alloc returned
		 * the same pointer as requested and fill the area with
		 * predefined pattern.
		 */
		if (alloc_res == EOK) {
			if ((unsigned int) alloc_ptr != (alloc_page * PAGE_SIZE)) {
				printk ("<%d,%d>   ..  Error: Invalid pointer "
					"returned by vma_alloc.\n",
					GET_GROUP_ID (grpthrid),
					GET_THREAD_ID (grpthrid));

				error_flag = 1;
				goto exit_unlock_both;
			}

			fill_block (phase, mem, alloc_ptr, alloc_bytes);
		}

exit_unlock_both:
	UNLOCK_CONSOLE;

	/*
	 * Unlock the area list.
	 */
	tst_area_list_unlock (mem);
} /* action_vma_alloc_rand */


/**
 * action_vma_alloc_bad
 *
 * Attempts to allocate an area that fully or partially overlaps with an
 * existing virtual memory area.
 */
void
action_vma_alloc_bad (
	struct phase *		phase,
	tst_area_list_s *	mem,
	struct action_param *	data,
	grpthr_id_t 		grpthrid,
	int 			cycles,
	int 			actionno
){
	/*
	 * Lock the list of areas.
	 */
	tst_area_list_lock (mem);

	/*
	 * Check if there is at least one allocated area to work with.
	 * Do nothing if there is none.
	 */
	int area_count = tst_area_list_area_count (mem);
	if (area_count == 0) {
		LOCK_CONSOLE;
			PRINT_ACTION_COMMON_HEADER;
			printk ("vma_alloc_bad .. no memory to be checked\n");
		UNLOCK_CONSOLE;

		tst_area_list_unlock (mem);
		return;
	}

	/*
	 * Randomly select an allocated area.
	 */
	int area_idx;
	int area_page;
	int area_size;

	area_idx = tst_rand () % area_count;
	tst_area_list_get_area (mem, area_idx, & area_page, & area_size);


	/*
	 * Select a random page/size overlapping the existing area.
	 */
	int alloc_size = random_size (data->min_block_size, data->max_block_size);
	int alloc_page = area_page + random_range (-alloc_size + 1, area_size);

	size_t alloc_bytes = alloc_size * PAGE_SIZE;
	void * alloc_ptr = (void *) (alloc_page * PAGE_SIZE);

	LOCK_CONSOLE;
		PRINT_ACTION_COMMON_HEADER;
		printk ("vma_alloc_bad (ptr=%p, size=%x)", alloc_ptr, alloc_bytes);

		/*
		 * If the randomly selected addresses are not within the
		 * allowed range, just return without reporting an error.
		 */
		if (! within_range (alloc_page, alloc_size)) {
			printk (", out of tested range\n");
			goto exit_unlock_both;
		}

		/*
		 * Attempt to allocate the memory and check the result.
		 */
		int alloc_res = vma_alloc (& alloc_ptr, alloc_bytes, VF_USER_ADDR);
		printk (" = %s\n", result_name (alloc_res));

		if (alloc_res == EOK) {
			printk ("   ..  Error: Allocated area that should not "
				"be available.\n");
			error_flag = 1;
		}

exit_unlock_both:
	UNLOCK_CONSOLE;

	/*
	 * Unlock the list of areas.
	 */
	tst_area_list_unlock (mem);
} /* action_vma_alloc_bad */


/****************************************************************************\
| PUBLIC CODE (VMA_FREE)                                                     |
\****************************************************************************/

/**
 * action_vma_free
 *
 * Finds an allocated memory area and releases ii.
*/
void
action_vma_free (
	struct phase *		phase,
	tst_area_list_s *	mem,
	struct action_param *	data,
	grpthr_id_t 		grpthrid,
	int 			cycles,
	int 			actionno)
{
	/*
	 * Lock the list of areas.
	 */
	tst_area_list_lock (mem);

	/*
	 * Check if there is at least one allocated area to work with.
	 * Do nothing if there is none.
	 */
	int area_count = tst_area_list_area_count (mem);
	if (area_count == 0) {
		LOCK_CONSOLE;
			PRINT_ACTION_COMMON_HEADER;
			printk ("vma_free .. no memory to be released\n");
		UNLOCK_CONSOLE;

		tst_area_list_unlock (mem);
		return;
	}

	/*
	 * Randomly select an allocated area.
	 */
	int area_idx;
	int area_page;
	int area_size;

	area_idx = tst_rand () % area_count;
	tst_area_list_get_area (mem, area_idx, & area_page, & area_size);

	/*
	 * Verify and release the area.
	 */
	void * release_ptr = (void *) (area_page * PAGE_SIZE);

	LOCK_CONSOLE;
		PRINT_ACTION_COMMON_HEADER;
		printk ("vma_free (ptr=%p)", release_ptr);

		/*
		 * Verify the area first.
		 */
		if (! verify_block (phase, mem, release_ptr, area_size * PAGE_SIZE)) {
			printk ("\n   ..  Error: The area to be freed has "
				"been corrupted!\n");

			error_flag = 1;
			goto exit_unlock_both;
		}

		/*
		 * Release the area.
		 */
		int release_res = vma_free (release_ptr);
		printk (" = %s\n", result_name (release_res));

		if (release_res != EOK) {
			printk ("   ..  Error: Failed to free area expected "
				"to be allocated!\n");

			error_flag = 1;
		}
exit_unlock_both:
	UNLOCK_CONSOLE;

	/*
	 * Delete the area from the list and unlock the area list.
	 */
	tst_area_list_del (mem, area_idx);
	tst_area_list_unlock (mem);
}


/**
 * action_vma_free_bad
 *
 * Attempts to release a virtual memory area with start address in
 * a hole in the virtual memory map.
 */
void
action_vma_free_bad (
	struct phase *		phase,
	tst_area_list_s *	mem,
	struct action_param *	data,
	grpthr_id_t 		grpthrid,
	int 			cycles,
	int 			actionno
){
	/*
	 * Lock the list of areas.
	 */
	tst_area_list_lock (mem);

	/*
	 * Check if there is at least one unallocated area to work with.
	 * Do nothing if there is none.
	 */
	int hole_count = tst_area_list_hole_count (mem);
	if (hole_count == 0) {
		LOCK_CONSOLE;
			PRINT_ACTION_COMMON_HEADER;
			printk ("vma_free_bad .. no areas worth overlapping\n");
		UNLOCK_CONSOLE;

		tst_area_list_unlock (mem);
		return;
	}


	/*
	 * Select a random hole.
	 */
	int hole_idx;
	int hole_page;
	int hole_size;

	hole_idx = tst_rand () % hole_count;
	tst_area_list_get_hole (mem, hole_idx, & hole_page, & hole_size);

	/*
	 * Try to release the area.
	 */
	int release_page = hole_page + tst_rand () % hole_size;
	void * release_ptr = (void *) (release_page * PAGE_SIZE);

	LOCK_CONSOLE;
		PRINT_ACTION_COMMON_HEADER;
		printk ("vma_free_bad (ptr=%p)", release_ptr);

		int release_res = vma_free (release_ptr);
		printk (" = %s\n", result_name (release_res));

		if (release_res != EINVAL) {
			printk ("   ..  Error: Released memory area that was "
				"supposed to be unallocated!\n");
			error_flag = 1;
		}
	UNLOCK_CONSOLE;


	/*
	 * Unlock the area list.
	 */
	tst_area_list_unlock (mem);
} /* action_vma_free_bad */


/****************************************************************************\
| PUBLIC CODE (VMA_RESIZE)                                                   |
\****************************************************************************/

/**
 * action_vma_resize
 *
 * Randomly picks an allocated virtual memory area and resizes it
 * (i.e. shrinks or grows its size).
*/
void
action_vma_resize (
	struct phase *		phase,
	tst_area_list_s *	mem,
	struct action_param *	data,
	grpthr_id_t 		grpthrid,
	int 			cycles,
	int actionno
){
	/*
	 * Lock the list of areas.
	 */
	tst_area_list_lock (mem);

	/*
	 * Check if there is at least one allocated area to work with.
	 * Do nothing if there is none.
	 */
	int area_count = tst_area_list_area_count (mem);
	if (area_count == 0) {
		LOCK_CONSOLE;
			PRINT_ACTION_COMMON_HEADER;
			printk ("vma_resize .. no areas to resize\n");
		UNLOCK_CONSOLE;

		tst_area_list_unlock (mem);
		return;
	}


	/*
	 * Randomly select an allocated area.
	 */
	int area_idx;
	int area_page;
	int area_size;

	area_idx = tst_rand () % area_count;
	tst_area_list_get_area (mem, area_idx, & area_page, & area_size);

	/*
	 * Choose a size to resize the area to.
	 */
	void * resize_ptr = (void *) (area_page * PAGE_SIZE);
	int resize_size = random_size (data->min_block_size, data->max_block_size);
	int resize_bytes = resize_size * PAGE_SIZE;

	LOCK_CONSOLE;
		PRINT_ACTION_COMMON_HEADER;
		printk ("vma_resize (ptr=%p, size=%x)", resize_ptr, resize_bytes);

		/*
		 * Resize the area in kernel. If there was not enough memory
		 * to grow the area, we just return without reporting an
		 * error.
		 */
		int resize_res = vma_resize (resize_ptr, resize_bytes);
		printk (" = %s", result_name (resize_res));

		if (resize_res == ENOMEM) {
			printk ("\n");
			goto exit_unlock_both;
		}

		int expect_res = tst_area_list_resize (mem, area_idx, resize_size);
		printk (", expected %s\n", result_name (expect_res));

		/*
		 * In other cases, the result of tst_area_list_resize should
		 * be the same as that of vma_resize, since tst_area_list_resize
		 * cannot fail with ENOMEM.
		 */
		if (resize_res != expect_res) {
			/*
			 * There must have been an overlap.
			 */
			printk ("<%d,%d>   ..  Error: Mismatching vma_resize "
				"results.\n", GET_GROUP_ID (grpthrid),
				GET_THREAD_ID (grpthrid));

			error_flag = 1;
			goto exit_unlock_both;
		}

		/*
		 * If everything went smoothly, fill the area with
		 * predefined pattern.
		 */
		if (resize_res == EOK)
			fill_block (phase, mem, resize_ptr, resize_bytes);

exit_unlock_both:
	UNLOCK_CONSOLE;


	/*
	 * Unlock the area list.
	 */
	tst_area_list_unlock (mem);
} /* action_vma_resize */


/**
 * action_vma_resize_bad
 *
 * Attempts to resize a randomly selected hole.
*/
void
action_vma_resize_bad (
	struct phase *		phase,
	tst_area_list_s *	mem,
	struct action_param *	data,
	grpthr_id_t 		grpthrid,
	int 			cycles,
	int 			actionno
){
	/*
	 * Lock the list of areas.
	 */
	tst_area_list_lock (mem);

	/*
	 * Check if there is at least one unallocated area to work with.
	 * Do nothing if there is none.
	 */
	int hole_count = tst_area_list_hole_count (mem);
	if (hole_count == 0) {
		LOCK_CONSOLE;
			PRINT_ACTION_COMMON_HEADER;
			printk ("vma_resize_bad .. no unallocated areas\n");
		UNLOCK_CONSOLE;

		tst_area_list_unlock (mem);
		return;
	}


	/*
	 * Select a random hole.
	 */
	int hole_idx;
	int hole_page;
	int hole_size;

	hole_idx = tst_rand () % hole_count;
	tst_area_list_get_hole (mem, hole_idx, & hole_page, & hole_size);

	/*
	 * Try to resize a hole.
	 */
	void * resize_ptr = (void *) ((hole_page + (tst_rand () % hole_size)) * PAGE_SIZE);
	int resize_bytes = random_size (data->min_block_size, data->max_block_size) * PAGE_SIZE;

	LOCK_CONSOLE;
		PRINT_ACTION_COMMON_HEADER;
		printk ("vma_resize_bad (ptr=%p, size=%x)", resize_ptr, resize_bytes);

		int resize_res = vma_resize (resize_ptr, resize_bytes);
		printk (" = %s\n", result_name (resize_res));

		if (resize_res != EINVAL) {
			printk ("   ..  Error: Attempt to resize a hole should "
				"return EINVAL.\n");
			error_flag = 1;
		}
	UNLOCK_CONSOLE;


	/*
	 * Unlock the area list.
	 */
	tst_area_list_unlock (mem);
} /* action_vma_resize_bad */


/****************************************************************************\
| PUBLIC CODE (VMA_SPLIT/VMA_MERGE/VMA_REMAP)                                |
\****************************************************************************/

/**
 * action_vma_split
 *
 * Randomly picks a virtual memory area and splits it into two areas.
 */
void
action_vma_split (
	struct phase *		phase,
	tst_area_list_s *	mem,
	struct action_param *	data,
	grpthr_id_t 		grpthrid,
	int 			cycles,
	int actionno
){
	/*
	 * Lock the list of areas.
	 */
	tst_area_list_lock (mem);

	/*
	 * Check if there is at least one allocated area to work with.
	 * Do nothing if there is none.
	 */
	int area_count = tst_area_list_area_count (mem);
	if (area_count == 0) {
		LOCK_CONSOLE;
			PRINT_ACTION_COMMON_HEADER;
			printk ("vma_split .. no areas to split\n");
		UNLOCK_CONSOLE;

		tst_area_list_unlock (mem);
		return;
	}


	/*
	 * Randomly select an allocated area.
	 */
	int area_idx;
	int area_page;
	int area_size;

	area_idx = tst_rand () % area_count;
	tst_area_list_get_area (mem, area_idx, & area_page, & area_size);

	/*
	 * Don't split areas of size 1.
	 */
	if (area_size == 1) {
		LOCK_CONSOLE;
			PRINT_ACTION_COMMON_HEADER;
			printk ("vma_split .. cannot split single page area\n");
		UNLOCK_CONSOLE;

		tst_area_list_unlock (mem);
		return;
	}

	/*
	 * Split the area.
	 */
	void * area_ptr = (void *) (area_page * PAGE_SIZE);
	int split_page = area_page + 1 + (tst_rand () % (area_size - 1));
	void * split_ptr = (void *) (split_page * PAGE_SIZE);

	LOCK_CONSOLE;
		PRINT_ACTION_COMMON_HEADER;
		printk ("vma_split (area=%p, split=%p)", area_ptr, split_ptr);

		int split_res = vma_split (area_ptr, split_ptr);
		printk (" = %s\n", result_name (split_res));

		/*
		 * If there was not enough memory to split the area,
		 * we just return without reporting an error.
		 */
		if (split_res == ENOMEM)
			goto exit_unlock_both;

		/*
		 * In other cases, the result of vma_split should be the same
		 * as the result of tst_area_list_split, except when the
		 * results is ENOMEM, in which case we just make a
		 * consistency vma_merge.
		 */
		int expect_res = tst_area_list_split (mem, split_page);
		if (expect_res == ENOMEM) {
			PRINT_ACTION_COMMON_HEADER;
			printk ("consistency vma_merge (ptr1=%p, ptr2=%p)\n",
				area_ptr, split_ptr);
			vma_merge (area_ptr, split_ptr);
			goto exit_unlock_both;
		}

		if (split_res != expect_res) {
			printk ("<%d,%d>   ..  Error: Mismatching vma_split "
				"results.\n", GET_GROUP_ID (grpthrid),
				GET_THREAD_ID (grpthrid));

			error_flag = 1;
			goto exit_unlock_both;
		}

exit_unlock_both:
	UNLOCK_CONSOLE;

	/*
	 * Unlock the area list.
	 */
	tst_area_list_unlock(mem);
} /* action_vma_split */


/**
 * action_vma_merge
 *
 * Randomly picks a pair of neighboring areas and merges them.
 */
void
action_vma_merge (
	struct phase *		phase,
	tst_area_list_s *	mem,
	struct action_param *	data,
	grpthr_id_t 		grpthrid,
	int 			cycles,
	int			actionno
){
	/*
	 * Lock the list of areas.
	 */
	tst_area_list_lock (mem);

	/*
	 * Check if there is at least one pair of neighbor areas to work with.
	 * Do nothing if there is none.
	 */
	int neigh_count = tst_area_list_neigh_count (mem);
	if (neigh_count == 0) {
		LOCK_CONSOLE;
			PRINT_ACTION_COMMON_HEADER;
			printk ("vma_merge .. no neighbor areas to merge\n");
		UNLOCK_CONSOLE;

		tst_area_list_unlock (mem);
		return;
	}


	/*
	 * Select a random pair of neighboring areas.
	 */
	int neigh_idx;
	int area_idx;
	int area_page;
	int area_size;

	neigh_idx = tst_rand () % neigh_count;
	tst_area_list_get_neigh (mem, neigh_idx, & area_idx, & area_page, & area_size);

	/*
	 * Merge the areas.
	 */
	void * merge_ptr1 = (void*) (area_page * PAGE_SIZE);
	void * merge_ptr2 = (void*) ((area_page + area_size) * PAGE_SIZE);

	LOCK_CONSOLE;
		PRINT_ACTION_COMMON_HEADER;
		printk("vma_merge (ptr1=%p, ptr2=%p)", merge_ptr1, merge_ptr2);

		int merge_res = vma_merge (merge_ptr1, merge_ptr2);
		printk (" = %s\n", result_name (merge_res));

		if (merge_res != EOK) {
			printk ("<%d,%d>   ..  Error: Merge failed when it "
				"should have succeeded.\n",
				GET_GROUP_ID (grpthrid),
				GET_THREAD_ID(grpthrid));

			error_flag = 1;
			goto exit_unlock_both;
		}

		/*
		 * Merge the areas in the model.
		 */
		tst_area_list_merge (mem, area_idx);

exit_unlock_both:
	UNLOCK_CONSOLE;


	/*
	 * Unlock the area list.
	 */
	tst_area_list_unlock (mem);
} /* action_vma_merge */


/**
 * action_vma_remap
 *
 * Attempts to remap a randomly picked area to a randomly picked hole.
 */
void
action_vma_remap (
	struct phase *		phase,
	tst_area_list_s *	mem,
	struct action_param *	data,
	grpthr_id_t 		grpthrid,
	int 			cycles,
	int 			actionno
){
	/*
	 * Lock the list of areas.
	 */
	tst_area_list_lock (mem);

	/*
	 * Check if there is at least one allocated area to work with.
	 * Do nothing if there is none.
	 */
	int area_count = tst_area_list_area_count (mem);
	if (area_count == 0) {
		LOCK_CONSOLE;
			PRINT_ACTION_COMMON_HEADER;
			printk ("vma_remap .. no areas to remap\n");
		UNLOCK_CONSOLE;

		tst_area_list_unlock (mem);
		return;
	}

	/*
	 * Check if there is at least one unallocated area to work with.
	 * Do nothing if there is none.
	 */
	int hole_count = tst_area_list_hole_count (mem);
	if (hole_count == 0) {
		LOCK_CONSOLE;
			PRINT_ACTION_COMMON_HEADER;
			printk ("vma_remap .. no hole to remap to\n");
		UNLOCK_CONSOLE;

		tst_area_list_unlock (mem);
		return;
	}

	/*
	 * Randomly select an allocated area.
	 */
	int area_idx;
	int area_page;
	int area_size;

	area_idx = tst_rand () % area_count;
	tst_area_list_get_area (mem, area_idx, & area_page, & area_size);

	/*
	 * Select a random hole.
	 */
	int hole_idx;
	int hole_page;
	int hole_size;

	hole_idx = tst_rand () % hole_count;
	tst_area_list_get_hole (mem, hole_idx, & hole_page, & hole_size);


	/*
	 * Attempt to remap the area.
	 */
	void * area_ptr = (void *) (area_page * PAGE_SIZE);
	int remap_page = hole_page + (tst_rand() % hole_size);
	void * remap_ptr = (void *) (remap_page * PAGE_SIZE);

	LOCK_CONSOLE;
		PRINT_ACTION_COMMON_HEADER;
		printk ("vma_remap (from=%p, to=%p)", area_ptr, remap_ptr);

		/*
		 * Remap the area both in the kernel memory and in the area
		 * list and check for mismatching results. If there was not
		 * enough memory to remap the area, we just return without
		 * reporting an error.
		 */
		int remap_res = vma_remap (area_ptr, remap_ptr);
		printk (" = %s", result_name (remap_res));

		if (remap_res == ENOMEM) {
			printk ("\n");
			goto exit_unlock_both;
		}

		int expect_res = tst_area_list_remap (mem, area_idx, remap_page);
		printk (", expected %s\n", result_name (expect_res));

		/*
		 * In other cases, the results of tst_area_list_remap and of
		 * kernel vma_remap should be the same, since
		 * tst_area_list_remap cannot fail with ENOMEM.
		 */
		if (remap_res != expect_res) {
			/*
			 * There must have been an overlap.
			 */
			printk ("<%d,%d>   ..  Error: Mismatching vma_remap "
				"results.\n", GET_GROUP_ID (grpthrid),
				GET_THREAD_ID (grpthrid));

			error_flag = 1;
			goto exit_unlock_both;
		}

		/*
		 * If everything went smoothly, me may check that the
		 * memory is intact after the remapping.
		 */
		if (remap_res == EOK && !verify_block (phase, mem,
			remap_ptr, area_size * PAGE_SIZE)) {

			printk ("<%d,%d>   ..  Error: The memory has been "
				"corrupted during remap.\n",
				GET_GROUP_ID (grpthrid),
				GET_THREAD_ID (grpthrid));

			error_flag = 1;
			goto exit_unlock_both;
		}

exit_unlock_both:
	UNLOCK_CONSOLE;


	/*
	 * Unlock the area list.
	 */
	tst_area_list_unlock (mem);
} /* action_vma_remap */


/****************************************************************************\
| PUBLIC CODE (ACCESS)                                                       |
\****************************************************************************/

/**
 * action_access
 *
 * Randomly picks an address in a randomly selected virtual memory area and
 * attempts to read a byte from it. The value should correspond to a value
 * expected to be read at that location.
 */
void
action_access (
	struct phase *		phase,
	tst_area_list_s *	mem,
	struct action_param *	data,
	grpthr_id_t 		grpthrid,
	int 			cycles,
	int 			actionno
){
	/*
	 * Lock the list of areas.
	 */
	tst_area_list_lock (mem);

	/*
	 * Check if there is at least one allocated area to work with.
	 * Do nothing if there is none.
	 */
	int area_count = tst_area_list_area_count (mem);
	if (area_count == 0) {
		LOCK_CONSOLE;
			PRINT_ACTION_COMMON_HEADER;
			printk ("access .. no areas to access\n");
		UNLOCK_CONSOLE;

		tst_area_list_unlock (mem);
		return;
	}

	/*
	 * Randomly select an allocated area.
	 */
	int area_idx;
	int area_page;
	int area_size;

	area_idx = tst_rand () % area_count;
	tst_area_list_get_area (mem, area_idx, & area_page, & area_size);


	/*
	 * Pick a location within the area.
	 */
	unsigned char * access_ptr = random_ptr (area_page, area_size);
	unsigned char expected_val = expected_value (phase, mem, access_ptr);

	LOCK_CONSOLE;
		PRINT_ACTION_COMMON_HEADER;
		printk ("access (ptr=%p) = ", access_ptr);
		printk ("%d, expected %d\n", *access_ptr, expected_val);

		if (*access_ptr != expected_val) {
			/*
			 * The value read does not match the expected value...
			 */
			printk ("<%d,%d>   ..  Error: Unexpected value "
				"found at %p\n", GET_GROUP_ID (grpthrid),
				GET_THREAD_ID (grpthrid), access_ptr);
			error_flag = 1;
		}
	UNLOCK_CONSOLE;


	/*
	 * Unlock the area list.
	 */
	tst_area_list_unlock (mem);
} /* action_access */


/**
 * action_access_bad
 *
 * Randomly picks an invalid location and attempts to read a byte from it.
 * Calling this action should result in the calling thread being killed.
 */
void
action_access_bad (
	struct phase *		phase,
	tst_area_list_s *	mem,
	struct action_param *	data,
	grpthr_id_t 		grpthrid,
	int 			cycles,
	int 			actionno
){
	/*
	 * Lock the list of areas.
	 */
	tst_area_list_lock (mem);

	/*
	 * Check if there is at least one unallocated area to work with.
	 * Do nothing if there is none.
	 */
	int hole_count = tst_area_list_hole_count (mem);
	if (hole_count == 0) {
		LOCK_CONSOLE;
			PRINT_ACTION_COMMON_HEADER;
			printk ("access_bad .. no hole to access\n");
		UNLOCK_CONSOLE;

		tst_area_list_unlock (mem);
		return;
	}


	/*
	 * Select a random hole.
	 */
	int hole_idx;
	int hole_page;
	int hole_size;

	hole_idx = tst_rand () % hole_count;
	tst_area_list_get_hole (mem, hole_idx, & hole_page, & hole_size);

	/*
	 * Attempt to access a location within a hole in the memory map.
	 */
	unsigned char * access_ptr = random_ptr (hole_page, hole_size);

	LOCK_CONSOLE;
		PRINT_ACTION_COMMON_HEADER;
		printk("access_bad (ptr=%p) should cause exception\n", access_ptr);
	UNLOCK_CONSOLE;

	/*
	 * Don't forget to unlock the memory map, otherwise the other threads
	 * will sleep on the area list lock. There would be a tiny window
	 * for a race condition when somebody else could create a valid
	 * mapping under the location, so we disable interrupts just before
	 * letting go of the lock and get ourselves killed.
	 *
	 * NOTE: This can only work if mutexes do not reschedule
	 * the current thread.
	 */
	disable_interrupts ();
	tst_area_list_unlock (mem);
	(* access_ptr) = 0;

	/*
	 * This statement should not have been reached.
	 */
	enable_interrupts ();
	LOCK_CONSOLE;
		printk ("<%d,%d>   ..  Error: Access to non-mapped memory did "
			"not cause a fatal exception.\n",
			GET_GROUP_ID (grpthrid), GET_THREAD_ID (grpthrid));

		error_flag = 1;
	UNLOCK_CONSOLE;
} /* action_access_bad */


