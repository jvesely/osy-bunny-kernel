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
 * Conditional wait on spinlock with verbose output.
 */
#define cond_wait_spinlock_verbose(cond, spinlock, counter)	\
{							\
	spinlock_lock (& spinlock);				\
	while ((cond)) {				\
		printk ("  %d threads ...\n", counter);	\
		spinlock_unlock (& spinlock);			\
		thread_sleep (1);			\
		spinlock_lock (& spinlock);			\
	};						\
							\
	printk ("  %d threads ...\n", counter);		\
	spinlock_unlock (& spinlock);				\
}

/*
 * Conditional wait on semaphore with verbose output.
 */
#define cond_wait_semaphore_verbose(cond, mutex, counter)	\
{							\
	semaphore_down (& mutex, 1);				\
	while ((cond)) {				\
		printk ("  %d threads ...\n", counter);	\
		semaphore_up (& mutex, 1);			\
		thread_sleep (1);			\
		semaphore_down (& mutex, 1);			\
	};						\
							\
	printk ("  %d threads ...\n", counter);		\
	semaphore_up (& mutex, 1);				\
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
 * Spinlock protected substraction/decrement of a variable.
 */
#define sub_var_spinlock(val, var, spinlock)			\
{							\
	spinlock_lock (& spinlock);				\
	var -= val;					\
	spinlock_unlock (& spinlock);				\
}

#define dec_var_spinlock(var, spinlock)			\
	sub_var_spinlock (1, var, spinlock)

/*
 * semaphore protected substraction/decrement of a variable.
 */
#define sub_var_semaphore(val, var, mutex)			\
{							\
	semaphore_up (& mutex, 1);				\
	var -= val;					\
	semaphore_up (& mutex, 1);				\
}

#define dec_var_semaphore(var, mutex)			\
	sub_var_semaphore (1, var, mutex)


/*
 * Robust thread creation. Checks whether the thread was
 * successfully created.
 */
#define robust_thread_create(func, args)		\
({							\
	int 		_res;				\
	thread_t	_thr;				\
	_res = thread_create (& _thr, func, args, 0);	\
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

#endif /* _DEFS_H_ */
