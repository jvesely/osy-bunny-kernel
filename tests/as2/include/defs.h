/***
 * Shared defs for all the tests.
 *
 * Change Log:
 * 03/10/22 ghort	created
 * 03/11/02 pallas	added conditional wait macros
 * 03/11/03 pallas	added protected variable addition/substraction macros
 * 03/11/03 ghort	added verbose variant of cond_wait_mutex macros
 * 03/11/07 pallas	added robust_thread_create and THREAD_MAGIC macros
 * 04/10/25 pallas	updated robust_thread_create to new API
 * 04/10/25 pallas	added robust_thread_func and friends
 */

#ifndef _DEFS_H_
#define _DEFS_H_

/*
 * Base task size.
 */
#define TASK_SIZE	10

/*
 * Dummy thread parameter.
 */
#define THREAD_MAGIC	((void *) 0xdeadbeef)


/*
 * Conditional wait on mutex.
 */
#define cond_wait_mutex(cond, mutex)			\
{							\
	mutex_lock (& mutex);				\
	while ((cond)) {				\
		mutex_unlock (& mutex);			\
		thread_sleep (1);			\
		mutex_lock (& mutex);			\
	};						\
	mutex_unlock (& mutex);				\
}


/*
 * Conditional wait on mutex with verbose output.
 */
#define cond_wait_mutex_verbose(cond, mutex, counter)	\
{							\
	mutex_lock (& mutex);				\
	while ((cond)) {				\
		printk ("  %d threads ...\n", counter);	\
		mutex_unlock (& mutex);			\
		thread_sleep (1);			\
		mutex_lock (& mutex);			\
	};						\
							\
	printk ("  %d threads ...\n", counter);		\
	mutex_unlock (& mutex);				\
}


/*
 * Mutex protected addition/increment of a variable.
 */
#define add_var_mutex(val, var, mutex)			\
{							\
	mutex_lock (& mutex);				\
	var += val;					\
	mutex_unlock (& mutex);				\
}

#define inc_var_mutex(var, mutex)			\
	add_var_mutex (1, var, mutex)


/*
 * Mutex protected substraction/decrement of a variable.
 */
#define sub_var_mutex(val, var, mutex)			\
{							\
	mutex_lock (& mutex);				\
	var -= val;					\
	mutex_unlock (& mutex);				\
}

#define dec_var_mutex(var, mutex)			\
	sub_var_mutex (1, var, mutex)


/*
 * Robust thread creation. Checks whether the thread was
 * successfully created.
 */
#define robust_thread_create(func, args, flags)		\
({							\
	int 		_res;				\
	thread_t	_thr;				\
	_res = thread_create (& _thr, func, args, flags);	\
	if (_res != EOK) {				\
		panic ("Test failed...\n"		\
			"Unable to create "		\
			"thread (%d).\n", _res);	\
	}						\
	_thr;						\
})


/*
 * Generic robustness wrapper for thread API functions.
 */
#define robust_thread_func(thread, func, expv, opname)	\
{							\
	int _res;					\
	_res = func ((thread));				\
	if (_res != (expv)) {				\
		panic ("Failed to %s thread %p "	\
			"(got %d, expected %d).\n"	\
			"Test failed....\n", opname, 	\
			thread, _res, expv);		\
	}						\
}


/*
 * Robust thread join. Checks whether the thread was
 * successfully joined.
 */
#define robust_thread_join(thread)			\
	robust_thread_func (thread, thread_join, EOK, "join")


/*
 * Robust thread wake up. Checks whether the thread was
 * successfully woken.
 */
#define robust_thread_wakeup(thread)			\
	robust_thread_func (thread, thread_wakeup, EOK, "wake up")


/*
 * Robust thread detach. Checks whether the thread was
 * successfully detached.
 */
#define robust_thread_detach(thread)			\
	robust_thread_func (thread, thread_detach, EOK, "detach")


/*
 * Robust thread kill. Checks whether the thread was
 * successfully killed.
 */
#define robust_thread_kill(thread)			\
	robust_thread_func (thread, thread_kill, EOK, "kill")


#define DEBUG printf(".. %s: %d\n", __FILE__, __LINE__);

/**
 * TRUE/FALSE
 *
 * Primitive boolean types.
 */
#define FALSE	0
#define TRUE	!FALSE

/**
 * NULL
 *
 * Defines a null pointer.
 */
#ifndef NULL
#define NULL	((void *) 0)
#endif /* NULL */

/**
 * INT_MAX
 *
 * Defines maximal integer value.
 */
#ifndef INT_MAX
#define INT_MAX (int)0x7fffffff
#endif /* NULL */

/**
 * sizeof_array
 * @array:	array to determine the size of
 *
 * Returns the size of @array in array elements.
 */
#define sizeof_array(array)		\
	(sizeof (array) / sizeof ((array) [0]))

/**
 * Memory segments (32-bit kernel mode)
 */
#ifndef KUSEG_BASE
#define KUSEG_BASE		0x00000000UL	/* mapped, cached	*/
#endif

#ifndef KUSEG_SIZE
#define KUSEG_SIZE		(1 << 31)
#endif

#ifndef KSEG0_BASE
#define KSEG0_BASE		0x80000000UL	/* unmapped, cached	*/
#endif

#ifndef KSEG0_SIZE
#define KSEG0_SIZE		(1 << 29)
#endif

#ifndef KSEG1_BASE
#define KSEG1_BASE		0xa0000000UL	/* unmapped, uncached	*/
#endif

#ifndef KSEG1_SIZE
#define KSEG1_SIZE		(1 << 29)
#endif

#ifndef KSSEG_BASE
#define KSSEG_BASE		0xc0000000UL	/* mapped, cached	*/
#endif

#ifndef KSSEG_SIZE
#define KSSEG_SIZE		(1 << 29)
#endif

#ifndef KSEG3_BASE
#define KSEG3_BASE		0xe0000000UL	/* mapped, cached	*/
#endif

#ifndef KSEG3_SIZE
#define KSEG3_SIZE		(1 << 29)
#endif


/*
 * Minimal page size expected in the tests.
 */
#ifndef PAGE_SIZE
#define PAGE_SIZE		(1 << 12)
#endif

#ifndef PAGE_MASK
#define PAGE_MASK		(PAGE_SIZE - 1)
#endif

#if PAGE_MASK != (PAGE_SIZE - 1)
#error Unexpected value of PAGE_MASK!
#endif

/*
 * vma_alloc allocation flags.
 */
#define VF_USER_ADDR \
	(VF_VA_USER << VF_VA_SHIFT)
#define VF_AUTO_KUSEG \
	((VF_AT_KUSEG << VF_AT_SHIFT) + (VF_VA_AUTO << VF_VA_SHIFT))
#define VF_AUTO_KSSEG \
	((VF_AT_KSSEG << VF_AT_SHIFT) + (VF_VA_AUTO << VF_VA_SHIFT))

/*
 * Typedef for thread start function.
 */
typedef void *(* thread_fnc) (void * data);

#endif /* _DEFS_H_ */
