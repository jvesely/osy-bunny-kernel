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
 * @brief Short description.
 *
 * Long description. I would paste some Loren Ipsum rubbish here, but I'm afraid
 * It would stay that way. Not that this comment is by any means ingenious but 
 * at least people can understand it. 
 */
#include "Scheduler.h"
#include "Kernel.h"
#include "drivers/Processor.h"

thread_t Scheduler::schedule(Thread* newThread)
{
	//ostrich stuff, there shall always be free id
	thread_t id = m_nextThread++;
	while (m_threadMap.insert(id, newThread) == -1) {
		//find free id
		id = m_nextThread++;
	}
	assert( Kernel::instance().pool().reserved() );
	ListItem<Thread*>* item = Kernel::instance().pool().get();
	item->data() = newThread;
	newThread->setId(id);

	m_activeThreadList.pushBack(item);
	dprintf("Scheduled thread %u to run.\n", id);
	return id;
}

void Scheduler::switchThread()
{
	//disable interupts
	if (m_activeThreadList.size() == 0) { //no thread to switch to
		return;
	}
	//dprintf("Rotating threadlist with %u items\n", m_activeThreadList.size());

	void* DUMMYSTACK = (void*)0xF00;
	void** old_stack = (void**)(m_currentThread?m_currentThread->stackTop():&DUMMYSTACK);
	m_currentThread = *m_activeThreadList.rotate();
	void** new_stack = m_currentThread->stackTop();
	//dprintf("Switching stacks %x,%x\n", old_stack, new_stack);
	Processor::switch_cpu_context(old_stack, new_stack);
	//enable interupts
}
