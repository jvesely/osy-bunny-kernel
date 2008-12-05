/***
 * Area test #1
 *
 * Change Log:
 * 2003/11/25 vilda	created
 * 2003/11/29 vilda	updated
 * 2003/12/02 ghort	polished
 * 2004/04/25 pallas	really polished :-)
 * 2004/11/24 pallas	revamped and updated to match new assignment spec
 */

static const char *	desc =
	"Area test #1\n"
	"Tests whether virtual memory area management functions work. This\n"
	"is only a set of basic function calls with test of expected return\n"
	"values.\n\n"
	"In case of failure, read the test output carefully or look into\n"
	"the sources.\n\n";


#include <api.h>
#include "../include/defs.h"

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


/*
 * Fill a memory block with garbage.
 */
static void
fillmem (uint8_t * ptr, size_t size)
{
	char chr;
	uint8_t * end;

	for (chr = 0, end = ptr + size; ptr < end; ptr++, chr++) {
		*ptr = chr;
	}
} /* fillmem */


/*
 * vma_alloc/vma_free/vma_resize/vma_remap/vma_merge/vma_split parameters.
 */
struct vparam {
	void *		from;		// vma_alloc/vma_resize/vma_split @from
	size_t		size;		// vma_alloc/vma_resize @size
	unsigned int	flags;		// vma_alloc @flags

	void *		area1;		// vma_merge @area1
	void *		area2;		// vma_merge @area2
	void *		split;		// vma_split @split
	void *		to;		// vma_remap @to

	int		result;		// expected result

	char *		desc;		// test description

	/*
	 * Test function to call with the parameters.
	 */
	int		(* test) (struct vparam * param);

	/*
	 * Evaluator fuction for the results.
	 */
	int		(* eval) (struct vparam * param, int result, ...);
}; /* vparam */


/**
 * vma_alloc evaluators.
 *
 * @param:	parameters of the vma_alloc call
 * @result:	result of the call
 * @from:	resulting start address
 *
 * The function evaluate the results of the test and return 0 if the test
 * failed, and nonzero value if the test passed.
 */
static int
eval_vma_alloc_result (struct vparam * param, int result, ...)
{
	return (result == param->result);
} /* eval_vma_alloc_result */

static int
eval_vma_alloc_from_eq (struct vparam * param, int result, ...)
{
	va_list	args;
	void *	from;

	va_start (args, result);
	from = va_arg (args, void *);
	va_end (args);

	// check the result address besides the result
	return eval_vma_alloc_result (param, result, from) && (from == param->from);


} /* eval_vma_alloc_from_eq */

static int
eval_vma_alloc_from_ge (struct vparam * param, int result, ...)
{
	va_list	args;
	void *	from;

	va_start (args, result);
	from = va_arg (args, void *);
	va_end (args);

	// check the result address besides the result
	return eval_vma_alloc_result (param, result, from) && (from >= param->from);
} /* eval_vma_alloc_from_ge */

static int
eval_vma_alloc_from_lt (struct vparam * param, int result, ...)
{
	va_list	args;
	void *	from;

	va_start (args, result);
	from = va_arg (args, void *);
	va_end (args);

	// check the result address besides the result
	return eval_vma_alloc_result (param, result, from) && (from < param->from);
} /* eval_vma_alloc_from_lt */


/**
 * vma_alloc_test
 * @param:	parameters for the vma_alloc call
 *
 * Performs a vma_alloc call with parameters @param and checks the result.
 * Returns nonzero value if the call succeeded, 0 if the call failed.
 */
static int
vma_alloc_test (struct vparam * param)
{
	int 	result;
	void *	from;

	from = param->from;
	printk ("Allocate %x bytes at %p (%s)\n",
		param->size, param->from, param->desc);

	result = vma_alloc (& from, param->size, param->flags);
	printk ("\texpected result: %s, vma_alloc result: %s, from: %p\n",
		result_name (param->result), result_name (result), from);

	return param->eval (param, result, from);
} /* vma_alloc_test */


/**
 * vma_free_test
 * @param:	parameters for the vma_free call
 *
 * Performs a vma_free call with parameters @param and checks the result.
 * Returns nonzero value if the call succeeded, 0 if the call failed.
 */
static int
vma_free_test (struct vparam * param)
{
	int 	result;

	printk ("Free area at %p (%s)\n", param->from, param->desc);

	result = vma_free (param->from);
	printk ("\texpected result: %s, vma_free result: %s\n",
		result_name (param->result), result_name (result));

	return (result == param->result);
} /* vma_free_test */


/**
 * vma_resize_test
 * @param:	parameters for the vma_resize call
 *
 * Performs a vma_resize call with parameters @param and checks the result.
 * Returns nonzero value if the call succeeded, 0 if the call failed.
 */
static int
vma_resize_test (struct vparam * param)
{
	int 	result;

	printk ("Resize area at %p to %x bytes (%s)\n", param->from,
		param->size, param->desc);

	result = vma_resize (param->from, param->size);
	printk ("\texpected result: %s, vma_resize result: %s\n",
		result_name (param->result), result_name (result));

	return (result == param->result);
} /* vma_resize_test */


/**
 * vma_merge_test
 * @param:	parameters for the vma_merge call
 *
 * Performs a vma_merge call with parameters @param and checks the result.
 * Returns nonzero value if the call succeeded, 0 if the call failed.
 */
static int
vma_merge_test (struct vparam * param)
{
	int 	result;

	printk ("Merge areas at %p and %p (%s)\n", param->area1,
		param->area2, param->desc);

	result = vma_merge (param->area1, param->area2);
	printk ("\texpected result: %s, vma_merge result: %s\n",
		result_name (param->result), result_name (result));

	return (result == param->result);
} /* vma_merge_test */


/**
 * vma_split_test
 * @param:	parameters for the vma_split call
 *
 * Performs a vma_split call with parameters @param and checks the result.
 * Returns nonzero value if the call succeeded, 0 if the call failed.
 */
static int
vma_split_test (struct vparam * param)
{
	int 	result;

	printk ("Split area %p at %p (%s)\n", param->from,
		param->split, param->desc);

	result = vma_split (param->from, param->split);
	printk ("\texpected result: %s, vma_split result: %s\n",
		result_name (param->result), result_name (result));

	return (result == param->result);
} /* vma_split_test */


/**
 * vma_remap_test
 * @param:	parameters for the vma_remap call
 *
 * Performs a vma_remap call with parameters @param and checks the result.
 * Returns nonzero value if the call succeeded, 0 if the call failed.
 */
static int
vma_remap_test (struct vparam * param)
{
	int 	result;

	printk ("Remap area from %p to %p (%s)\n", param->from,
		param->to, param->desc);

	result = vma_remap (param->from, param->to);
	printk ("\texpected result: %s, vma_remap result: %s\n",
		result_name (param->result), result_name (result));

	return (result == param->result);
} /* vma_remap_test */


/**
 * vaccess_test
 * @param:	parameters for the memory access
 *
 * Accesses the memory described in @param.
 * Returns nonzero value to indicate success.
 */
static int
vaccess_test (struct vparam * param)
{
	printk ("Writing %x bytes to area at %p (%s)\n", param->size,
		param->from, param->desc);

	fillmem (param->from, param->size);
	return 1;
} /* vaccess_test */


/**
 * test_sequence
 * @param:	an array of test parameters
 * @ntests:	number of test parameters
 *
 * Performs a sequence of tests. Returns 0 at first failure, nonzero value
 * if all tests passed.
 */
static int
test_sequence (struct vparam * param, int ntests)
{
	while (ntests-- > 0) {
		int passed;

		passed = param->test (param);
		printk ("\t%s\n\n", passed ? "PASSED" : "FAILED");

		if (!passed)
			return 0;

		param++;
	}

	return 1;
} /* test_sequence */



/*
 * Area test sequence.
 */
static struct vparam	area1_seq [] = {
	/*
	* vma_alloc test sequence.
	*/
	{
		.test = vma_alloc_test,
		.from = (void *) KUSEG_BASE,
		.size = KUSEG_SIZE,
		.flags = VF_USER_ADDR,
		.result = ENOMEM,
		.eval = eval_vma_alloc_result,
		.desc = "exhaust memory"
	},

	{
		.test = vma_alloc_test,
		.from = (void *) KUSEG_BASE + 1,
		.size = 2 * PAGE_SIZE,
		.flags = VF_USER_ADDR,
		.result = EINVAL,
		.eval = eval_vma_alloc_result,
		.desc = "unaligned address"
	},

	{
		.test = vma_alloc_test,
		.from = (void *) KUSEG_BASE,
		.size = 0,
		.flags = VF_USER_ADDR,
		.result = EINVAL,
		.eval = eval_vma_alloc_result,
		.desc = "zero-sized area"
	},

	{
		.test = vma_alloc_test,
		.from = (void *) KUSEG_BASE,
		.size = PAGE_SIZE - 1,
		.flags = VF_USER_ADDR,
		.result = EINVAL,
		.eval = eval_vma_alloc_result,
		.desc = "unaligned size"
	},

	{
		.test = vma_alloc_test,
		.from = (void *) KSEG0_BASE - PAGE_SIZE,
		.size = 2 * PAGE_SIZE,
		.flags = VF_USER_ADDR,
		.result = EINVAL,
		.eval = eval_vma_alloc_result,
		.desc = "KUSEG->KSEG0 overflow"
	},

	{
		.test = vma_alloc_test,
		.from = (void *) KSEG3_BASE + KSEG3_SIZE - PAGE_SIZE,
		.size = 2 * PAGE_SIZE,
		.flags = VF_USER_ADDR,
		.result = EINVAL,
		.eval = eval_vma_alloc_result,
		.desc = "KSEG3->KUSEG overflow"
	},

	{
		.test = vma_alloc_test,
		.from = (void *) KUSEG_BASE,
		.size = 2 * PAGE_SIZE,
		.flags = VF_USER_ADDR,
		.result = EOK,
		.eval = eval_vma_alloc_from_eq,
		.desc = "A at KUSEG_BASE"
	},

	{
		.test = vaccess_test,
		.from = (void *) KUSEG_BASE,
		.size = 2 * PAGE_SIZE,
		.result = EOK,
		.desc = "fill A"
	},

	{
		.test = vma_alloc_test,
		.from = (void *) KUSEG_BASE + 4 * PAGE_SIZE,
		.size = 2 * PAGE_SIZE,
		.flags = VF_USER_ADDR,
		.result = EOK,
		.eval = eval_vma_alloc_from_eq,
		.desc = "B at KUSEG_BASE + 4 * PAGE_SIZE"
	},

	{
		.test = vaccess_test,
		.from = (void *) KUSEG_BASE + 4 * PAGE_SIZE,
		.size = 2 * PAGE_SIZE,
		.result = EOK,
		.desc = "fill B"
	},

	{
		.test = vma_alloc_test,
		.from = (void *) KUSEG_BASE + 6 * PAGE_SIZE,
		.size = 2 * PAGE_SIZE,
		.flags = VF_USER_ADDR,
		.result = EOK,
		.eval = eval_vma_alloc_from_eq,
		.desc = "C at KUSEG_BASE + 6 * PAGE_SIZE"
	},

	{
		.test = vaccess_test,
		.from = (void *) KUSEG_BASE + 6 * PAGE_SIZE,
		.size = 2 * PAGE_SIZE,
		.result = EOK,
		.desc = "fill C"
	},

	{
		.test = vma_alloc_test,
		.from = (void *) KSEG0_BASE - PAGE_SIZE,
		.size = PAGE_SIZE,
		.flags = VF_USER_ADDR,
		.result = EOK,
		.eval = eval_vma_alloc_from_eq,
		.desc = "F at KSEG0_BASE - PAGE_SIZE [KUSEG]"
	},

	{
		.test = vaccess_test,
		.from = (void *) KSEG0_BASE - PAGE_SIZE,
		.size = PAGE_SIZE,
		.result = EOK,
		.desc = "fill F"
	},

	{
		.test = vma_alloc_test,
		.from = (void *) KSEG3_BASE - PAGE_SIZE,
		.size = PAGE_SIZE,
		.flags = VF_USER_ADDR,
		.result = EOK,
		.eval = eval_vma_alloc_from_eq,
		.desc = "G at KSEG3_BASE - PAGE_SIZE [KSEG2]"
	},

	{
		.test = vaccess_test,
		.from = (void *) KSEG3_BASE - PAGE_SIZE,
		.size = PAGE_SIZE,
		.result = EOK,
		.desc = "fill G"
	},

	{
		.test = vma_alloc_test,
		.from = (void *) KSEG3_BASE,
		.size = PAGE_SIZE,
		.flags = VF_USER_ADDR,
		.result = EOK,
		.eval = eval_vma_alloc_from_eq,
		.desc = "H at KSEG3_BASE"
	},

	{
		.test = vaccess_test,
		.from = (void *) KSEG3_BASE,
		.size = PAGE_SIZE,
		.result = EOK,
		.desc = "fill H"
	},

	{
		.test = vma_alloc_test,
		.from = (void *) KUSEG_BASE - PAGE_SIZE,
		.size = PAGE_SIZE,
		.flags = VF_USER_ADDR,
		.result = EOK,
		.eval = eval_vma_alloc_from_eq,
		.desc = "I at KUSEG_BASE - PAGE_SIZE [KSEG3]"
	},

	{
		.test = vaccess_test,
		.from = (void *) KUSEG_BASE - PAGE_SIZE,
		.size = PAGE_SIZE,
		.result = EOK,
		.desc = "fill I"
	},

	{
		.test = vma_alloc_test,
		.from = (void *) KUSEG_BASE + 4 * PAGE_SIZE,
		.size = 4 * PAGE_SIZE,
		.flags = VF_USER_ADDR,
		.result = EINVAL,
		.eval = eval_vma_alloc_result,
		.desc = "full B, C overlap"
	},

	{
		.test = vma_alloc_test,
		.from = (void *) KUSEG_BASE + 3 * PAGE_SIZE,
		.size = 2 * PAGE_SIZE,
		.flags = VF_USER_ADDR,
		.result = EINVAL,
		.eval = eval_vma_alloc_result,
		.desc = "partial B overlap"
	},

	{
		.test = vma_alloc_test,
		.from = (void *) KUSEG_BASE + 3 * PAGE_SIZE,
		.size = 4 * PAGE_SIZE,
		.flags = VF_USER_ADDR,
		.result = EINVAL,
		.eval = eval_vma_alloc_result,
		.desc = "full B, partial C overlap"
	},

	{
		.test = vma_alloc_test,
		.from = (void *) KUSEG_BASE + 1 * PAGE_SIZE,
		.size = 2 * PAGE_SIZE,
		.flags = VF_USER_ADDR,
		.result = EINVAL,
		.eval = eval_vma_alloc_result,
		.desc = "partial A overlap"
	},

	{
		.test = vma_alloc_test,
		.from = (void *) KUSEG_BASE + 1 * PAGE_SIZE,
		.size = 4 * PAGE_SIZE,
		.flags = VF_USER_ADDR,
		.result = EINVAL,
		.eval = eval_vma_alloc_result,
		.desc = "partial A, B overlap"
	},

	{
		.test = vma_alloc_test,
		.from = (void *) KUSEG_BASE + 8 * PAGE_SIZE,
		.size = 4 * PAGE_SIZE,
		.flags = VF_AUTO_KUSEG,
		.result = EOK,
		.eval = eval_vma_alloc_from_ge,
		.desc = "D at/after KUSEG_BASE + 8 * PAGE_SIZE"
	},

	{
		.test = vma_alloc_test,
		.from = (void *) KSEG0_BASE - PAGE_SIZE,
		.size = 4 * PAGE_SIZE,
		.flags = VF_AUTO_KUSEG,
		.result = EOK,
		.eval = eval_vma_alloc_from_lt,
		.desc = "E before KSEG0_BASE - PAGE_SIZE [KUSEG]"
	},


	/*
	 * vma_free test sequence.
	 */
	{
		.test = vma_free_test,
		.from = (void *) KSSEG_BASE,
		.result = EINVAL,
		.desc = "nonexistent area"
	},

	{
		.test = vma_free_test,
		.from = (void *) KUSEG_BASE + PAGE_SIZE - 1,
		.result = EINVAL,
		.desc = "unaligned address"
	},

	{
		.test = vma_free_test,
		.from = (void *) KUSEG_BASE + PAGE_SIZE,
		.result = EINVAL,
		.desc = "not start of A"
	},

	{
		.test = vma_free_test,
		.from = (void *) KUSEG_BASE + 4 * PAGE_SIZE,
		.result = EOK,
		.desc = "B at KUSEG_BASE + 4 * PAGE_SIZE"
	},

	{
		.test = vma_free_test,
		.from = (void *) KUSEG_BASE + 4 * PAGE_SIZE,
		.result = EINVAL,
		.desc = "nonexistent area"
	},

	/*
	 * vma_resize test sequence.
	 */
	{
		.test = vma_resize_test,
		.from = (void *) KSSEG_BASE,
		.size = 2 * PAGE_SIZE,
		.result = EINVAL,
		.desc = "nonexistent area"
	},

	{
		.test = vma_resize_test,
		.from = (void *) KUSEG_BASE,
		.size = 0,
		.result = EINVAL,
		.desc = "zero-sized area"
	},

	{
		.test = vma_resize_test,
		.from = (void *) KUSEG_BASE,
		.size = PAGE_SIZE - 1,
		.result = EINVAL,
		.desc = "unaligned size"
	},

	{
		.test = vma_resize_test,
		.from = (void *) KUSEG_BASE + PAGE_SIZE,
		.size = PAGE_SIZE,
		.result = EINVAL,
		.desc = "not start of A"
	},

	{
		.test = vma_resize_test,
		.from = (void *) KUSEG_BASE,
		.size = 8 * PAGE_SIZE,
		.result =  EINVAL,
		.desc = "A overflow to C"
	},

	{
		.test = vma_resize_test,
		.from = (void *) KSEG0_BASE - PAGE_SIZE,
		.size = 2 * PAGE_SIZE,
		.result = EINVAL,
		.desc = "F overflow KUSEG->KSEG0"
	},

	{
		.test = vma_resize_test,
		.from = (void *) KUSEG_BASE - PAGE_SIZE,
		.size = 2 * PAGE_SIZE,
		.result = EINVAL,
		.desc = "G overflow KSEG3->KUSEG"
	},

	{
		.test = vma_resize_test,
		.from = (void *) KUSEG_BASE,
		.size = 5 * PAGE_SIZE,
		.result = EOK,
		.desc = "grow A"
	},

	{
		.test = vaccess_test,
		.from = (void *) KUSEG_BASE,
		.size = 5 * PAGE_SIZE,
		.result = EOK,
		.desc = "fill A"
	},

	{
		.test = vma_resize_test,
		.from = (void *) KUSEG_BASE,
		.size = 3 * PAGE_SIZE,
		.result = EOK,
		.desc = "shrink A"
	},

	{
		.test = vaccess_test,
		.from = (void *) KUSEG_BASE,
		.size = 3 * PAGE_SIZE,
		.result = EOK,
		.desc = "fill A"
	},

	{
		.test = vma_resize_test,
		.from = (void *) KUSEG_BASE,
		.size = 3 * PAGE_SIZE,
		.result = EOK,
		.desc = "same A"
	},

	/*
	 * vma_merge test sequence.
	 */
	{
		.test = vma_alloc_test,
		.from = (void *) KUSEG_BASE + 3 * PAGE_SIZE,
		.size = 3 * PAGE_SIZE,
		.flags = VF_USER_ADDR,
		.result = EOK,
		.eval = eval_vma_alloc_from_eq,
		.desc = "B at KUSEG_BASE + 3 * PAGE_SIZE"
	},

	{
		.test = vaccess_test,
		.from = (void *) KUSEG_BASE + 3 * PAGE_SIZE,
		.size = 3 * PAGE_SIZE,
		.result = EOK,
		.desc = "fill B"
	},

	{
		.test = vma_merge_test,
		.area1 = (void *) KUSEG_BASE + 3 * PAGE_SIZE,
		.area2 = (void *) KUSEG_BASE + 6 * PAGE_SIZE,
		.result = EOK,
		.desc = "B + C -> BC at KUSEG_BASE + 3 * PAGE_SIZE"
	},

	{
		.test = vaccess_test,
		.from = (void *) KUSEG_BASE + 3 * PAGE_SIZE,
		.size = 5 * PAGE_SIZE,
		.result = EOK,
		.desc = "fill BC"
	},

	{
		.test = vma_free_test,
		.from = (void *) KUSEG_BASE + 6 * PAGE_SIZE,
		.result = EINVAL,
		.desc = "not start of BC"
	},

	{
		.test = vma_free_test,
		.from = (void *) KUSEG_BASE + 3 * PAGE_SIZE,
		.result = EOK,
		.desc = "BC at KUSEG_BASE + 3 * PAGE_SIZE"
	},

	{
		.test = vma_alloc_test,
		.from = (void *) KUSEG_BASE + 4 * PAGE_SIZE,
		.size = 2 * PAGE_SIZE,
		.flags = VF_USER_ADDR,
		.result = EOK,
		.eval = eval_vma_alloc_from_eq,
		.desc = "C at KUSEG_BASE + 4 * PAGE_SIZE"
	},

	{
		.test = vaccess_test,
		.from = (void *) KUSEG_BASE + 4 * PAGE_SIZE,
		.size = 2 * PAGE_SIZE,
		.result = EOK,
		.desc = "fill C"
	},

	{
		.test = vma_merge_test,
		.area1 = (void *) KUSEG_BASE,
		.area2 = (void *) KUSEG_BASE + 4 * PAGE_SIZE,
		.result = EINVAL,
		.desc = "A, C not adjacent"
	},

	{
		.test = vma_merge_test,
		.area1 = (void *) KUSEG_BASE,
		.area2 = (void *) KUSEG_BASE + 3 * PAGE_SIZE,
		.result = EINVAL,
		.desc = "nonexistant second area"
	},

	{
		.test = vma_alloc_test,
		.from = (void *) KUSEG_BASE + 3 * PAGE_SIZE,
		.size = 2 * PAGE_SIZE,
		.flags = VF_USER_ADDR,
		.result = EINVAL,
		.eval = eval_vma_alloc_from_eq,
		.desc = "C overlap"
	},

	{
		.test = vma_alloc_test,
		.from = (void *) KUSEG_BASE + 3 * PAGE_SIZE,
		.size = PAGE_SIZE,
		.flags = VF_USER_ADDR,
		.result = EOK,
		.eval = eval_vma_alloc_from_eq,
		.desc = "B at KUSEG_BASE + 3 * PAGE_SIZE"
	},

	{
		.test = vaccess_test,
		.from = (void *) KUSEG_BASE + 3 * PAGE_SIZE,
		.size = PAGE_SIZE,
		.result = EOK,
		.desc = "fill B"
	},

	{
		.test = vma_merge_test,
		.area1 = (void *) KUSEG_BASE,
		.area2 = (void *) KUSEG_BASE + 4 * PAGE_SIZE,
		.result = EINVAL,
		.desc = "A, C not adjacent"
	},

	{
		.test = vma_merge_test,
		.area1 = (void *) KUSEG_BASE + 3 * PAGE_SIZE,
		.area2 = (void *) KUSEG_BASE,
		.result = EOK,
		.desc = "A + B -> AB at KUSEG_BASE"
	},

	{
		.test = vaccess_test,
		.from = (void *) KUSEG_BASE,
		.size = 4 *  PAGE_SIZE,
		.result = EOK,
		.desc = "fill AB"
	},

	{
		.test = vma_merge_test,
		.area1 = (void *) KSEG3_BASE - PAGE_SIZE,
		.area2 = (void *) KSEG3_BASE,
		.result = EINVAL,
		.desc = "KSEG2->KSEG3 overflow"
	},

	{
		.test = vma_resize_test,
		.from = (void *) KUSEG_BASE + 4 * PAGE_SIZE,
		.size = 4 * PAGE_SIZE,
		.result = EOK,
		.desc = "grow C"
	},

	{
		.test = vaccess_test,
		.from = (void *) KUSEG_BASE + 4 * PAGE_SIZE,
		.size = 4 * PAGE_SIZE,
		.result = EOK,
		.desc = "fill C"
	},

	{
		.test = vma_merge_test,
		.area1 = (void *) KUSEG_BASE + 4 * PAGE_SIZE,
		.area2 = (void *) KUSEG_BASE,
		.result = EOK,
		.desc = "AB + C -> ABC at KUSEG_BASE"
	},

	{
		.test = vaccess_test,
		.from = (void *) KUSEG_BASE,
		.size = 4 * PAGE_SIZE,
		.result = EOK,
		.desc = "fill ABC"
	},

	{
		.test = vma_split_test,
		.from = (void *) KUSEG_BASE,
		.split = (void *) KUSEG_BASE + 8 * PAGE_SIZE,
		.result = EINVAL,
		.desc = "behind ABC"
	},

	{
		.test = vma_split_test,
		.from = (void *) KUSEG_BASE,
		.split = (void *) KUSEG_BASE,
		.result = EINVAL,
		.desc = "zero-size split"
	},

	{
		.test = vma_split_test,
		.from = (void *) KUSEG_BASE,
		.split = (void *) KUSEG_BASE + PAGE_SIZE / 2,
		.result = EINVAL,
		.desc = "unaligned split"
	},

	{
		.test = vma_split_test,
		.from = (void *) KUSEG_BASE,
		.split = (void *) KUSEG_BASE + 2 * PAGE_SIZE,
		.result = EOK,
		.desc = "ABC at KUSEG_BASE -> A + BC"
	},

	{
		.test = vma_split_test,
		.from = (void *) KUSEG_BASE,
		.split = (void *) KUSEG_BASE + 2 * PAGE_SIZE,
		.result = EINVAL,
		.desc = "behind A"
	},

	{
		.test = vma_free_test,
		.from = (void *) KUSEG_BASE,
		.result = EOK,
		.desc = "free A"
	},

	{
		.test = vma_remap_test,
		.from = (void *) KUSEG_BASE + 2 * PAGE_SIZE,
		.to = (void *) KSSEG_BASE,
		.result = EOK,
		.desc = "C to KSSEG_BASE"
	},

	{
		.test = vaccess_test,
		.from = (void *) KSSEG_BASE,
		.size = 4 * PAGE_SIZE,
		.result = EOK,
		.desc = "fill C"
	},

	{
		.test = vma_free_test,
		.from = (void *) KUSEG_BASE + 2 * PAGE_SIZE,
		.result = EINVAL,
		.desc = "C not there"
	},

	{
		.test = vma_remap_test,
		.from = (void *) KSSEG_BASE,
		.to = (void *) KSEG3_BASE - 2 * PAGE_SIZE,
		.result = EINVAL,
		.desc = "collision with G"
	}

}; /* area1_seq */




void
run_test (void)
{
	int result;

	printk (desc);

	/*
	 * Test sequence.
	 */
	result = test_sequence (area1_seq, sizeof_array (area1_seq));
	printk ("Test %s...\n", result ? "passed" : "failed");
} /* run_test */

