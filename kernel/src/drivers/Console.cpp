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
	while (count() == 0) { /* buffer is empty */
//		Scheduler::instance().dequeue(Scheduler::instance().activeThread());
		//remove from the scheduling queue
//		ListItem<Thread*>* item = Kernel::instance().pool().get();
//		assert(item); //there must be one as dequeue returned one to the pool
//		item->data() = Kernel::instance().scheduler().activeThread(); 
		Thread * thread = Scheduler::instance().activeThread();
		thread->removeFromHeap();
		thread->append(&m_waitList); 
		thread->setStatus(Thread::BLOCKED);
		thread->yield();
	//	m_waitList.pushBack(item);
	//	item->data()->setStatus(Thread::BLOCKED);
	//	item->data()->yield();

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
//	dprintf("Char on address %p : %c (%d).\n", m_inputAddress, *m_inputAddress, *m_inputAddress);
	insert();
//	*m_outputAddress = m_buffer.readLast(); //echo
//	dprintf("First char is still \"%c\".\n", m_buffer.read());
	//dprintf("Processing Interupt\n");
	if (m_waitList.size()) {
		// get first waiting thread
//		ListItem<Thread*>* item = m_waitList.removeFront();
//		assert(item);
//		assert(item->data());
		Thread * thr = m_waitList.getFront(); //item->data();
	//	dprintf("Unblocking thread %u\n", thr->id());
//		item->data() = NULL;
		assert(thr->status() == Thread::BLOCKED);
		// item needs to be returned before the thread could be scheduled
//		Kernel::instance().pool().put(item);
		Scheduler::instance().enqueue(thr);
	}
//	dprintf("Buffer count: %u \n", m_buffer.count());
}
