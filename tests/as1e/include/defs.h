/***
 * Shared defs for all the tests.
 *
 * Change Log:
 * 03/10/22 ghort	created
 * 03/11/02 pallas	added conditional wait macros
 * 03/11/03 pallas	added protected variable addition/substraction macros
 * 03/11/03 ghort	added verbose variant of cond_wait_<sem|mutex|spinlock> macros
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
 * Conditional wait on semaphore.
 */
#define cond_wait_sem(cond, sem)			\
{							\
	sem_down (& sem);				\
	while ((cond)) {				\
		sem_up (& sem);				\
		thread_sleep (1);			\
		sem_down (& sem);			\
	};						\
	sem_up (& sem);					\
}


/*
 * Conditional wait on semaphore with verbose output.
 */
#define cond_wait_sem_verbose(cond, sem, counter)	\
{							\
	sem_down (& sem);				\
	while ((cond)) {				\
		printk ("  %d threads ...\n", counter);	\
		sem_up (& sem);				\
		thread_sleep (1);			\
		sem_down (& sem);			\
	};						\
							\
	printk ("  %d threads ...\n", counter);		\
	sem_up (& sem);					\
}


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
 * Conditional wait on spinlock.
 */
#define cond_wait_spinlock(cond, lock)			\
{							\
	spinlock_lock (& lock);				\
	while ((cond)) {				\
		spinlock_unlock (& lock);			\
		thread_sleep (1);			\
		spinlock_lock (& lock);			\
	};						\
	spinlock_unlock (& lock);				\
}


/*
 * Conditional wait on spinlock with verbose output.
 */
#define cond_wait_spinlock_verbose(cond, lock, counter)	\
{							\
	spinlock_lock (& lock);				\
	while ((cond)) {				\
		printk ("  %d threads ...\n", counter);	\
		spinlock_unlock (& lock);			\
		thread_sleep (1);			\
		spinlock_lock (& lock);			\
	};						\
							\
	printk ("  %d threads ...\n", counter);		\
	spinlock_unlock (& lock);				\
}


/*
 * Semaphore protected addition/increment of a variable.
 */
#define add_var_sem(val, var, sem)			\
{							\
	sem_down (& sem);				\
	var += val;					\
	sem_up (& sem);					\
}

#define inc_var_sem(var, sem)				\
	add_var_sem (1, var, sem)


/*
 * Semaphore protected substraction/decrement of a variable.
 */
#define sub_var_sem(val, var, sem)			\
{							\
	sem_down (& sem);				\
	var -= val;					\
	sem_up (& sem);					\
}

#define dec_var_sem(var, sem)				\
	sub_var_sem (1, var, sem)


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
 * Spinlock protected addition/increment of a variable.
 */
#define add_var_spinlock(val, var, lock)			\
{							\
	spinlock_lock (& lock);				\
	var += val;					\
	spinlock_unlock (& lock);				\
}

#define inc_var_spinlock(var, lock)			\
	add_var_spinlock (1, var, lock)


/*
 * Spinlock protected substraction/decrement of a variable.
 */
#define sub_var_spinlock(val, var, lock)			\
{							\
	spinlock_lock (& lock);				\
	var -= val;					\
	spinlock_unlock (& lock);				\
}

#define dec_var_spinlock(var, lock)			\
	sub_var_spinlock (1, var, lock)


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
