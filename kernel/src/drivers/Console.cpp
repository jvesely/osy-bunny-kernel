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
 * @brief Console class implementation.
 *
 * Implements class Console member functions.
 */

#include "Console.h"
#include "Kernel.h"

//#define CONSOLE_DEBUG

#ifndef CONSOLE_DEBUG
#define PRINT_DEBUG(...)
#else
#define PRINT_DEBUG(ARGS...) \
  printf("[ CONSOLE_DEBUG ]: "); \
  printf(ARGS);
#endif

size_t Console::outputString(const char* str) const
{
	const char *  it = str; /* fly through the string */
	
	for (;*it;++it)
	{
		outputChar(*it);
	}

	return it - str; /* finish - start should give the number of chars */
}
/*----------------------------------------------------------------------------*/
char Console::readChar()
{
	/* buffer is empty */
	while (count() == 0) { 
		Thread * thread = Thread::getCurrent();

		/* remove from Timer and Scheduler */
		thread->block();

		/* add to my list */
		thread->append(&m_waitList); 

		/* set status and rest */
		thread->setStatus(Thread::BLOCKED);
		thread->yield();
	}
	return getChar();
}
/*----------------------------------------------------------------------------*/
ssize_t Console::readString(char* str, const size_t len)
{
	if (len == 0) return EINVAL;
	char* c = str;
	while ( (*c = readChar())  != '\n' 
			&&  (unsigned int)(c - str) < (len - 1) ) {
		++c;
	}
	*c = '\0';
	return c - str;
}
/*----------------------------------------------------------------------------*/
void Console::interrupt()
{
	/* read character */
	insert();

	PRINT_DEBUG ("Console interrupt, waiting threads: %u\n", m_waitList.size());

	/* there is someone waiting for this char */
	if (m_waitList.size()) {
		Thread * thr = m_waitList.getFront();
		PRINT_DEBUG ("Unblocking thread %u.\n", thr->id());
		ASSERT (thr->status() == Thread::BLOCKED);
		/* resume normal operation */
		thr->resume();
	}
}
