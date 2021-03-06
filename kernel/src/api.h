/*
 *          _     _
 *          \`\ /`/
 *           \ V /
 *           /. .\            Bunny Kernel for MIPS
 *          =\ T /=
 *           / ^ \
 *        {}/\\ //\
 *        __\ " " /__
 *   jgs (____/^\____)
 *   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
/*!
 *   @author Matus Dekanek, Tomas Petrusek, Lubos Slovak, Jan Vesely
 *   @par "SVN Repository"
 *   svn://aiya.ms.mff.cuni.cz/osy0809-depeslve
 *
 *   @version $Id$
 *   @note
 *   Semestral work for Operating Systems course at MFF UK \n
 *   http://dsrg.mff.cuni.cz/~ceres/sch/osy/main.php
 *
 *   @date 2008-2009
 */

/*!
 * @file
 * @brief API declarations
 *
 * Requiered functions that should be provided by kernel.
 */
#pragma once

#include "types.h"
#include "flags.h"
#include "assert.h"
#include "dprintf.h"

#define PAGE_SIZE  (1 << 13)
#define FRAME_SIZE PAGE_SIZE

// assignment 1 asks to check DEBUG_MUTEX to greater or equal value than 1
#if DEBUG_MUTEX >= 1
	#define mutex_unlock mutex_unlock_safe
#else
	#define mutex_unlock mutex_unlock_unsafe
#endif

#ifdef __cplusplus
extern "C" {
#endif
void switch_cpu_context(void** old_top, void** new_top);

void run_test(void);

void disable_interrupts();

void enable_interrupts();

/*! putc outputs one character.
 * @param c character to be printed
 * @return number of printed chars (0 or 1)
 */
size_t putc( const char c );

/*! puts outputs strings.
 * checks whether param is not null, prints chars until "\0" is found
 * @param str pointer to the first char of the string
 * @return number of printed chars
 */
size_t puts( const char * str );

/*! printf and printk are the same thing */
#define printk printf

/*! printf prints formated string on the console.
 * formating string may include:
 * %c: corresponding input variable is treated as char
 * %s: corresponding input variale is treated as char *
 * %d: corresponding input variale is treated as signed decimal number
 * %i: same as %d
 * %u: same as %d but the number is treated as unsigned
 * %x: corresponding input variale is treated as unsinged hexadecimal number
 * %p: corresponding input variale is treated as void *output is the same as %x
 * @param format format string
 * @... variable number of paramters used in the format string
 * @return number of printed chars
 */
size_t printf( const char * format, ... );

/*! @brief Reads one char from the device buffer.
 * @return Read char.
 * If there is no char in the input buffer requesting thread is blocked.
 */
char getc();

/*! @brief Tries to read one char from the device buffer.
 * @return Read char or EWOULDBLOCK on empty buffer.
 * If there is no char in the input buffer returns EWOULDBLOCK instead
 * of blocking.
 */
int getc_try();

/*! @brief Tries to read multiple chars from the device buffer.
 *
 * If len is 0 returns EINVAL. Reads from the buffer until '\n'
 * is read or len characters were read. \\0 is always put at the end.
 * @param str pointer to the buffer to be filled.
 * @param len number of chars to be read if no \\n is encountered.
 * @retval number of chars filled into buffer str.
 * @retval EINVAL if len == 0.
 */
ssize_t gets( char* str, const size_t len );

/*! @brief Allocates block of size size on the heap.
 *	@param size requested size of the block.
 *	@return pointer to the allocated block, NULL on failure
 */
void* malloc( const size_t size );

/*! @brief Returns no longer needed block back to the heap.
 * @param ptr pointer to the block
 */
void free( const void* ptr );

/*! dprintf and dprintk are the same thing. */
#define dprintk dprintf

/*! panic macro.
 * Macro dumps registers, prints out message that beggins with "Kernel panic: "
 * and stops simulation, after that it block the kernel.
 */
#define panic(ARGS...) \
	{	void* _panic_top_ = (void*)0xF00;\
	switch_cpu_context(&_panic_top_, (void**)NULL);\
	kpanic(&_panic_top_, ARGS); } while (0) // still needs tu dump those registers

void kpanic(void** context, const char* format, ... );

/*! Creates and runs a new thread. Stores its identifier.
 * @param thread_ptr pointer to place where identifier would be stored.
 * @param thread_start routine to run in the new thread
 * @param data data that will be sent to the routine
 * @param flags ignored param
 * @return ENOMEM on lack of memory, EOK on sucess
 */
int thread_create( thread_t* thread_ptr, void* (*thread_start)(void*),
	void* data, const unsigned int flags);

/*! Gets identifier of the currently running thread.
 * @return thread identifier
 */
thread_t thread_get_current();

/*! Blocks calling thread until given thread finishes execution.
 * @param thr id of the thread to wait for
 * @return EINVAL if attempting to wait for oneself, non-existing thread
 * id or if the thread is already beeing waited for, EKILLED if the thread
 * has already been killed EOK on success. (wakes when the thread is dead).
 */
int thread_join(thread_t thr);

/*! Similiar to version without timer, except that it waits max usec
 * microsecs.
 * @param thr thread id of the threadd one wants to wait for
 * @param usec number of microsecs to wait before returning ETIMEDOUT.
 * @return See non-timed version, plus ETIMEOUT if thread is still running
 * after given time.
 */
int thread_join_timeout(thread_t thr, const unsigned int usec);

/*! Sets thread state to detached. Detached thread can not be waited for
 * and its structures are deleted emediately after it finishes execution.
 * @param thr thread to detach
 */
int thread_detach(thread_t thr);

/*! Stops executing calling thread for sec seconds.
 * @param sec number of seconds to refrain from execution.
 */
void thread_sleep(const unsigned int sec);

/*! Similiar to sleep, this time in microseconds.
 * @param usec umber of microseconds to wait.
 */
void thread_usleep(const unsigned int usec);

/*! Surrender execution of the calling thread in favour of any other */
void thread_yield();

/*! Stop executing calling thread and don't schedule it until the thread has
 * been woken up.
 */
void thread_suspend();

/*! @brief Starts scheduling thread for execution again.
 *
 * @param  thr Thread using this id will be woken.
 * @return EINVAL if no thread with correspodning id exists, EOK if successfull.
 */
int thread_wakeup(thread_t thr);

/*! @brief Kills thread. 
 *
 * @param thr Thread with this id will be killed.
 * @retval EINVAL No thread with the given id exists, or it cannot be killed.
 * @retval EOK Thread was sucessfully killed.
 *
 * If the thread is detached it will be destroyed. Attached
 * threads will stop executing and won't be scheduled, but will remain in
 * the memory until ... .
 */
int thread_kill(thread_t thr);

/*!
 * @brief Copies block of memory from one place to another.
 *
 * This function copies @a count bytes starting from address given by
 * the @a src pointer to an address given by the @a dest pointer.
 * If the source and destination blocks overlap, the behaviour is undefined.
 *
 * @param dest Destination address.
 * @param src Source address.
 * @param count Number of bytes to copy.
 * @return Pointer to the destination block (i.e. @a dest).
 */

void* memcpy( void* dest, const void* src, size_t count );

int copy_from_thread( const thread_t thr,
          void *dest, const void *src, const size_t len);

int copy_to_thread( const thread_t thr,
          void *dest, const void *src, const size_t len);
/* --------------------------------------------------------------------- */
/* ----------------------      MUTEX    -------------------------------- */
/* --------------------------------------------------------------------- */

/**
 * @typedef mutex mutex_t
 * @struct mutex api.h "api.h"
 * @brief Sructure for mutex (mutual exclusion).
 *
 * Mutex structure for use in C and C++ code. It is possible to use it as struct
 * mutex or mutex_t, which is a typedef for it. Contains only a placeholder
 * for the C++ class.
 */
typedef struct mutex {
	byte payload[24];
} mutex_t;

/**
 * Initialize the given mutex struct (to unlocked state).
 *
 * @param mtx Mutex struct to initialize.
 */
void mutex_init(struct mutex *mtx);

/**
 * Destroy the given mutex struct. Remove it from all kernel structures.
 * If there are any locked threads on this mutex, destroy will cause panic.
 *
 * @param mtx Mutex struct to destroy.
 */
void mutex_destroy(struct mutex *mtx);

/**
 * Lock the given mutex. If the mutex is already locked, blocks untill it is unlocked.
 *
 * @param mtx Mutex to lock.
 */
void mutex_lock(struct mutex *mtx);

/**
 * Lock the given mutex, but don't let it take more than the given timelimit in microseconds.
 *
 * @param mtx Mutex to lock within time limit.
 * @param usec Timelimit in microseconds for trying to lock the mutex.
 */
int mutex_lock_timeout(struct mutex *mtx, const unsigned int usec);

/**
 * Unlock the mutex and check if it's being unlocked by the thread which locked it.
 *
 * @param mtx Mutex struct to unlock.
 */
void mutex_unlock_safe(struct mutex *mtx);

/**
 * Unlock the mutex and don't check the owner.
 *
 * @param mtx Mutex struct to unlock.
 */
void mutex_unlock_unsafe(struct mutex *mtx);

//------------------------------------------------------------------------------
/** @brief struct of timer
*	struct which allocates space required for ClassTimer
*/
struct timer{
	unsigned char data[84];//84 is actual size of ClassTimer
};


//------------------------------------------------------------------------------
/** @brief initializes timer
*	@param usec delay time
*	@param handler event handler for this timer
*	@param data pointer to data sent to handler routine
*	@note almost wrapper for ClassTimer::init() (controlls NULL)
*/
int timer_init( struct timer *tmr, const unsigned int usec,
				void (*handler)(struct timer *, void *), void *data);


//------------------------------------------------------------------------------

/** @brief start timer
*	inserts timer into kernel structure holding all timed events and therefore starts it
*	@note wrapper for Timer->startEvent()
*/
void timer_start(struct timer *tmr);

//------------------------------------------------------------------------------

/** @brief safely deinitializes timer
*	is NULL safe
*	calls TimerManager::destroyTimer(tmr)
*/
void timer_destroy(struct timer *tmr);

//------------------------------------------------------------------------------
/** @brief returns whether tmr is pending*/
int timer_pending(struct timer *tmr);


/* --------------------------------------------------------------------- */
/* ----------------------   SEMAPHORE   -------------------------------- */
/* --------------------------------------------------------------------- */

/**
 * @typedef semaphore semaphore_t
 * @struct semaphore api.h "api.h"
 * @brief Semaphore class placeholder for C code.
 */
typedef struct semaphore {
	byte payload[20];
} semaphore_t;

void sem_init(semaphore_t* s, const int value);
void sem_destroy(semaphore_t* s);

int sem_get_value(semaphore_t* s);
void sem_up(semaphore_t* s);
void sem_down(semaphore_t* s);
int sem_down_timeout(semaphore_t* s, const unsigned int usec);

/* --------------------------------------------------------------------- */
/* ----------------------    SPINLOCK   -------------------------------- */
/* --------------------------------------------------------------------- */

/**
 * @typedef spinlock spinlock_t
 * @struct spinlock api.h "api.h"
 * @brief Spinlock class placeholder for C code.
 */
typedef struct spinlock {
	byte payload[4];
} spinlock_t;

void spinlock_init(spinlock_t* s);
void spinlock_destroy(spinlock_t* s);

void spinlock_lock(spinlock_t* s);
void spinlock_unlock(spinlock_t* s);

/* --------------------------------------------------------------------- */
/* -----------------------  FRAME ALLOCATOR  --------------------------- */
/* --------------------------------------------------------------------- */

int frame_alloc(void **paddr, const size_t cnt, const unsigned int flags);

int frame_free(const void *paddr, const size_t cnt);

/* --------------------------------------------------------------------- */
/* ---------------------   VIRTUAL MEMORY AREA  ------------------------ */
/* --------------------------------------------------------------------- */

/**
 * Create (allocate) a new virtual memory area and store it in the virtual memory map.
 *
 * @param[in,out] from Optionally (if VF_VA_USER flag set) it is the user defined
 *   address of the new VMA (starting address). As output parameter it says the
 *   address where the new VMA starts (if allocation successful).
 * @param[in] size Requested size of the new VMA.
 * @param[in] flags Bit array of segment (where to allocate) and the way how to
 *   choose the address. (See flags.h for details.)
 * @return EOK, ENOMEM or EINVAL
 */
int vma_alloc(void **from, const size_t size, const unsigned int flags);

/**
 * Free one virtual memory area at the given address.
 *
 * @param from The starting address of the VMA (first byte).
 * @return EOK or EINVAL if correspoding VMA was not found.
 */
int vma_free(const void *from);

/**
 * Resize the given virtual memory area.
 *
 * @param from The address of a VMA (first byte).
 * @param size The new size of the VMA (non zero value).
 * @return EOK, ENOMEM or EINVAL
 */
int vma_resize(const void *from, const size_t size);

/**
 * Remap virtual memory area to a new virtual address. The new virtual address
 * can be (as it is not defined in the assignment and the tests want this feature)
 * in other segment.
 *
 * @param from The address of the VMA (first byte)
 * @param to The new starting address of the VMA.
 * @return EOK or EINVAL.
 */
int vma_remap(const void *from, const void *to);

/**
 * Merge two consequent virtual memory areas to one bigger block.
 *
 * @param area1 The address of a VMA (first byte). The order doesn't matter.
 * @param area2 The address of a VMA (first byte). The order doesn't matter.
 * @return EOK or EINVAL.
 */
int vma_merge(const void *area1, const void *area2);

/**
 * Split one virtual memory area to two pieces.
 *
 * @param from The address of the VMA (first byte).
 * @param split The address where to split, start the second piece.
 * @return EOK or EINVAL.
 */
int vma_split(const void *from, const void *split);


#ifdef __cplusplus
}
#endif
