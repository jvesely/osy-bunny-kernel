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
 * @brief API imaplementation
 *
 * Functions from api.h and helper functions are implemented here.
 */
#include "api.h"
#include "cpp.h"
#include "Kernel.h"
#include "synchronization/MutexManager.h"
#include "synchronization/Semaphore.h"
#include "InterruptDisabler.h"

//timer includes
#include "timer/TimerManager.h"
#include "timer/ClassTimer.h"

#define va_start __builtin_va_start
#define va_end __builtin_va_end
#define va_arg __builtin_va_arg


/*----------------------------------------------------------------------------*/
size_t putc(const char c)
{
	return Kernel::instance().console().outputChar(c);
}
/*----------------------------------------------------------------------------*/
size_t puts(const char * str)
{
	return Kernel::instance().console().outputString(str);
}
/*----------------------------------------------------------------------------*/
char getc()
{
	return Kernel::instance().console().readChar();
}
/*----------------------------------------------------------------------------*/
int getc_try()
{
	if (Kernel::instance().console().count())
		return Kernel::instance().console().getChar();
	else
		return EWOULDBLOCK;
}
/*----------------------------------------------------------------------------*/
ssize_t gets(char* str, const size_t len)
{
	return Kernel::instance().console().readString(str, len);
}
/*----------------------------------------------------------------------------*/
/*! prints number as unsigned decimal
 * @param number number to be printed
 * @return number of printed decimal digits
 */
size_t print_udecimal( uint32_t number)
{
	/* too many digits for the reverting algorithm */
	if ( number > ( (uint)(-1) / 10 ))
		return print_udecimal( number / 10) + putc( number % 10 + '0');
	uint32_t reverse;
	int size;
	size_t count = 0;
	if (number == 0)
		return putc('0');
	for(size = reverse = 0; number != 0;++size){
		reverse *= 10;
		reverse += number % 10;
		number /= 10;
	}
	for (;size;reverse/=10,--size)
		count += putc('0' + reverse % 10);

	return count;
};
/*----------------------------------------------------------------------------*/
/*! prints number as signed decimal
 * @param number number to be printed
 * @return number of printed decimal digits (+ sign)
 */

size_t print_decimal(const int32_t number)
{
	if (number < 0){
		return putc('-') + print_udecimal(-number);
	}
	return print_udecimal(number);

}
/*----------------------------------------------------------------------------*/
/*! prints number as unsigned hexadecimal
 * @param number number to be printed
 * @return number of printed hexadigits
 */
size_t print_hexa(uint32_t number, bool align)
{
	puts("0x");
	size_t count = 2;
	if(number == 0 && !align)
		return putc('0') + count;
	size_t size;
	uint32_t reverse;
	for (size = reverse = 0; number != 0 ; ++size){
		reverse *= 16;
		reverse += number % 16;
		number /= 16;
	}
	for (uint i = 8; align && i > size ; --i)
			count += putc('0');
	int res;
	for(;size;--size,reverse/=16)
		if((res = reverse % 16) >= 10)
			count += putc('a' + res%10);
		else
			count += putc('0' + res);
	return count;
}
/*----------------------------------------------------------------------------*/
/*! printk prints formated string on the console.
 * formating string may include:
 * %c: corresponding input variable is treated as char
 * %s: corresponding input variale is treated as char *
 * %d: corresponding input variale is treated as signed decimal number
 * %i: same as %d
 * %u: same as %d but the number is treated as unsigned
 * %x: corresponding input variale is treated as unsinged hexadecimal number
 * %p: corresponding input variale is treated as void *output is the same as %x
 * @param format formating string
 * @param args list of variables used in format
 * @return number of printed chars
 */
size_t vprintk(const char * format, va_list args)
{
	bool align = false;
	size_t count = 0;
	while (*format){
		if (*format == '%'){
			switch (*(format+1)){
				case 'c': count += putc(va_arg(args, int));
									++format;
									break;
				case 's': count += puts(va_arg(args, const char*));
									++format;
									break;
				case 'i':
				case 'd': count += print_decimal(va_arg(args, int32_t));
									++format;
									break;
				case 'u': count += print_udecimal(va_arg(args, uint32_t));
									++format;
									break;
				case 'p':
									align = true;
				case 'x': count += print_hexa(va_arg(args, uint32_t), align);
									align = false;
									++format;
									break;
				default:
					count += putc('%');
			}
		}else
			count += putc(*format);
		++format;
	}
	return count;
}
/*----------------------------------------------------------------------------*/
/*! printk is varibale paramerr version of vprintk
 * see declaration or vprintk for details
 * @param format format string
 * @param ... variable number of parameters
 */
size_t printk(const char * format, ...)
{
	if (!format) return 0;

	va_list args;
	va_start(args, format);
	size_t written = vprintk(format, args);
	va_end(args);

	return written;
}
/*----------------------------------------------------------------------------*/
void kpanic(void** context, const char* format, ...){
	using namespace Processor;
	Kernel::instance().regDump();

	Context* registers = (Context*)*context;
	printf("Register DUMP: %p\n", *context );
	printf("   0 %p\tat %p\tv0 %p\tv1 %p\ta0 %p\n", registers->zero, 
		registers->at, registers->v0, registers->v1, registers->a0 );
	printf("  a1 %p\ta2 %p\ta3 %p\tt0 %p\tt1 %p\n", registers->a1, registers->a2,
		registers->a3, registers->t0, registers->t1 );
	printf("  t2 %p\tt3 %p\tt4 %p\tt5 %p\tt6 %p\n", registers->t2, registers->t3,
		registers->t4, registers->t5, registers->t6 );
	printf("  t7 %p\ts0 %p\ts1 %p\ts2 %p\ts3 %p\n", registers->t7, registers->s0,
		registers->s1, registers->s2, registers->s3 );
	printf("  s4 %p\ts5 %p\ts6 %p\ts7 %p\tt8 %p\n", registers->s4, registers->s5,
		registers->s6, registers->s7, registers->t8 );
	printf("  t9 %p\tk0 %p\tk1 %p\tgp %p\tsp %p\n", registers->t9, registers->k0,
		registers->k1, registers->gp, registers->sp );
	printf("  fp %p\tra %p\tepc %p\tlo %p\thi %p\n", registers->fp, registers->ra,
		registers->epc, registers->lo, registers->hi );
	printf("  cause %p\tbadva %p\tstatus %p\n", registers->cause,	registers->badva, registers->status);

	printf("Kernel PANIC: ");
	if (!format) return;

	va_list args;
	va_start(args, format);
	vprintk(format, args);
	va_end(args);

	Kernel::instance().stop();
	Kernel::instance().block();

}
/*----------------------------------------------------------------------------*/
void* malloc (size_t size)
{
	return Kernel::instance().malloc(size);
}
/*----------------------------------------------------------------------------*/
void free(void* ptr)
{
	Kernel::instance().free(ptr);
}
/*----------------------------------------------------------------------------*/
int thread_create( thread_t* thread_ptr, void* (*thread_start)(void*),
  void* data, const unsigned int flags)
{
	InterruptDisabler inter;
	return KernelThread::create( thread_ptr, thread_start, data, flags );
}
/*----------------------------------------------------------------------------*/
thread_t thread_get_current()
{
	return Thread::getCurrent()->id();
}
/*----------------------------------------------------------------------------*/
int thread_join(thread_t thr)
{
	InterruptDisabler inter;
	Thread* thread = Scheduler::instance().thread(thr);
	return Thread::getCurrent()->join(thread);
}
/*----------------------------------------------------------------------------*/
int thread_join_timeout(thread_t thr, const uint usec)
{
	InterruptDisabler inter;
	Thread* thread = Scheduler::instance().thread(thr);
	return Thread::getCurrent()->joinTimeout(thread, usec);
}
/*----------------------------------------------------------------------------*/
int thread_detach(thread_t thread)
{
	InterruptDisabler inter;
	Thread* thr = Scheduler::instance().thread(thread);
	if (!thr
		|| thr->detached()
		|| thr->status() == Thread::FINISHED
		|| thr->status() == Thread::KILLED
		|| thr->status() == Thread::JOINING
		|| thr->follower())
	{ 
		return EINVAL;
	}

	if (thr->detach()) return EOK;
	assert(false);
	return EOK;
}
/*----------------------------------------------------------------------------*/
void thread_sleep(const unsigned int sec)
{
	Thread::getCurrent()->sleep(sec);
}
/*----------------------------------------------------------------------------*/
void thread_usleep(const unsigned int usec)
{
	Thread::getCurrent()->usleep(usec);
}
/*----------------------------------------------------------------------------*/
void thread_yield()
{
	Thread::getCurrent()->yield();
}
/*----------------------------------------------------------------------------*/
void thread_suspend()
{
	Thread::getCurrent()->suspend();
}
/*----------------------------------------------------------------------------*/
int thread_wakeup(thread_t thr)
{
	InterruptDisabler inter;
	Thread* thread = Scheduler::instance().thread(thr);
	if (!thread) return EINVAL;
	thread->wakeup();
	return EOK;
}
/*----------------------------------------------------------------------------*/
int thread_kill(thread_t thr)
{
	InterruptDisabler inter;
	Thread* thread = Scheduler::instance().thread(thr);
	if (!thread) return EINVAL;
	thread->kill();
	return EOK;
}
/*----------------------------------------------------------------------------*/
void* memcpy( void* dest, const void* src, size_t count )
{
	char* dstc = (char*) dest;
	char* srcc = (char*) src;

	while (count--) {
			*dstc++ = *srcc++;
	}
	return dest;
}

/*----------------------------------------------------------------------------*/

void mutex_init(struct mutex *mtx) {
	MutexManager::instance().mutex_init(mtx);
}

/*----------------------------------------------------------------------------*/

void mutex_destroy(struct mutex *mtx) {
	MutexManager::instance().mutex_destroy(mtx);
}

/*----------------------------------------------------------------------------*/

void mutex_lock(struct mutex *mtx) {
	MutexManager::instance().mutex_lock(mtx);
}

/*----------------------------------------------------------------------------*/

int mutex_lock_timeout(struct mutex *mtx, const unsigned int usec) {
	return MutexManager::instance().mutex_lock_timeout(mtx, usec);
}

/*----------------------------------------------------------------------------*/

void mutex_unlock(struct mutex *mtx) {
	MutexManager::instance().mutex_unlock(mtx);
}

//------------------------------------------------------------------------------
int timer_init( struct timer *tmr, const unsigned int usec,
				void (*handler)(struct timer *, void *), void *data)
{
	if(!tmr) return EINVAL;
	return drftmr(tmr).init(tmr,usec,handler,data);
}

//------------------------------------------------------------------------------
void timer_start(struct timer *tmr)
{
	TimerManager::instance().startEvent(tmr2Tmr(tmr));
}

//------------------------------------------------------------------------------
void timer_destroy(struct timer *tmr)
{
	TimerManager::instance().destroyTimer(tmr2Tmr(tmr));
}

//------------------------------------------------------------------------------
int timer_pending(struct timer *tmr)
{
	if(!tmr) return (int) false;
	return (int) drftmr(tmr).pending();
}

/*----------------------------------------------------------------------------*/

void semaphore_init(semaphore_t* s, unsigned int num) {
	ASSERT(sizeof(semaphore_t) >= sizeof(Semaphore));
	new (s) Semaphore(num);
}

void semaphore_destroy(semaphore_t* s) {
	((Semaphore *)s)->~Semaphore();
}

void semaphore_up(semaphore_t* s, unsigned int num) {
	((Semaphore *)s)->up(num);
}

void semaphore_down(semaphore_t* s, unsigned int num) {
	((Semaphore *)s)->down(num);
}

int semaphore_down_timeout(semaphore_t* s, unsigned int num, unsigned int usec) {
	return ((Semaphore *)s)->downTimeout(num, Time(0, usec));
}

/*----------------------------------------------------------------------------*/

