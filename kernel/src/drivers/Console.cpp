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
/*----------------------------------------------------------------------------*/
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

	/* there is someone waiting for this char */
	if (m_waitList.size()) {
		Thread * thr = m_waitList.getFront();
		ASSERT (thr->status() == Thread::BLOCKED);
		
		/* resume normal operation */
		thr->resume();
	}
}
