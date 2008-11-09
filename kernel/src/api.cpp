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
#include "Kernel.h"
#include "mutex/MutexManager.h"
#include "InterruptDisabler.h"

#define va_start __builtin_va_start
#define va_end __builtin_va_end
#define va_arg __builtin_va_arg
/*----------------------------------------------------------------------------*/
inline size_t putc(const char c)
{
	return Kernel::instance().console().outputChar(c);
}
/*----------------------------------------------------------------------------*/
inline size_t puts(const char * str)
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
size_t print_hexa(uint32_t number)
{
	puts("0x");
	size_t count = 2;
	if(number == 0)
		return putc('0') + count;
	size_t size;
	uint32_t reverse;
	for (size = reverse = 0; number != 0 ; ++size){
		reverse *= 16;
		reverse += number % 16;
		number /= 16;
	}
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
				case 'x': count += print_hexa(va_arg(args, uint32_t));
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
	printf("\t 0 %x\t\tat %x\t\tv0 %x\tv1 %x\ta0 %x\n", registers->zero, 
		registers->at, registers->v0, registers->v1, registers->a0 );
	printf("\ta1 %x\t\ta2 %x\t\ta3 %x\tt0 %x\tt1 %x\n", registers->a1, registers->a2,
		registers->a3, registers->t0, registers->t1 );
	printf("\tt2 %x\t\tt3 %x\t\tt4 %x\tt5 %x\tt6 %x\n", registers->t2, registers->t3,
		registers->t4, registers->t5, registers->t6 );
	printf("\tt7 %x\t\ts0 %x\t\ts1 %x\ts2 %x\ts3 %x\n", registers->t7, registers->s0,
		registers->s1, registers->s2, registers->s3 );
	printf("\ts4 %x\t\ts5 %x\t\ts6 %x\ts7 %x\tt8 %x\n", registers->s4, registers->s5,
		registers->s6, registers->s7, registers->t8 );
	printf("\tt9 %x\t\tk0 %x\t\tk1 %x\tgp %x\tsp %x\n", registers->t9, registers->k0,
		registers->k1, registers->gp, registers->sp );
	printf("\tfp %x\t\tra %x\t\tepc %x\tlo %x\thi %x\n", registers->fp, registers->ra,
		registers->epc, registers->lo, registers->hi );
	printf("\tcause %x\t\tbadva %x\tstatus %x\n", registers->cause,	registers->badva, registers->status);

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
	return KernelThread::create(thread_ptr, thread_start, data, flags);
}
/*----------------------------------------------------------------------------*/
thread_t thread_get_current()
{
	return Scheduler::instance().activeThread()->id();
}
/*----------------------------------------------------------------------------*/
int thread_join(thread_t thr)
{
	InterruptDisabler inter;
	Thread* thread = Scheduler::instance().thread(thr);
	return Scheduler::instance().activeThread()->join(thread);
}
/*----------------------------------------------------------------------------*/
int thread_join_timeout(thread_t thr, const uint usec)
{
	InterruptDisabler inter;
	Thread* thread = Scheduler::instance().thread(thr);
	return Scheduler::instance().activeThread()->joinTimeout(thread, usec);
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
		return EINVAL;
	if (thr->detach()) return EOK;
	return EOTHER;
}
/*----------------------------------------------------------------------------*/
void thread_sleep(const unsigned int sec)
{
	Scheduler::instance().activeThread()->sleep(sec);
}
/*----------------------------------------------------------------------------*/
void thread_usleep(const unsigned int usec)
{
	Scheduler::instance().activeThread()->usleep(usec);
}
/*----------------------------------------------------------------------------*/
void thread_yield()
{
	Scheduler::instance().activeThread()->yield();
}
/*----------------------------------------------------------------------------*/
void thread_suspend()
{
	Scheduler::instance().activeThread()->suspend();
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
	dprintf("Killing thread %p\n", thread);
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

/* --------------------------------------------------------------------- */

void mutex_init(struct mutex *mtx) {
	MutexManager::instance().mutex_init(mtx);
}

/* --------------------------------------------------------------------- */

void mutex_destroy(struct mutex *mtx) {
	MutexManager::instance().mutex_destroy(mtx);
}

/* --------------------------------------------------------------------- */

void mutex_lock(struct mutex *mtx) {
	MutexManager::instance().mutex_lock(mtx);
}

/* --------------------------------------------------------------------- */

int mutex_lock_timeout(struct mutex *mtx, const unsigned int usec) {
	return MutexManager::instance().mutex_lock_timeout(mtx, usec);
}

/* --------------------------------------------------------------------- */

void mutex_unlock(struct mutex *mtx) {
	MutexManager::instance().mutex_unlock(mtx);
}

