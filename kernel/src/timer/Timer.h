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
 *   @version $Id: header.tmpl 41 2008-10-26 18:00:14Z vesely $
 *   @note
 *   Semestral work for Operating Systems course at MFF UK \n
 *   http://dsrg.mff.cuni.cz/~ceres/sch/osy/main.php
 *   
 *   @date 2008-2009
 */

/*!
 * @file 
 * @brief Timer class declaration.
 *
 * Timer class stores Threads in the heap and wakes when their time comes.
 */

#pragma once

#include "structures/Heap.h"
#include "Singleton.h"
#include "timer/Time.h"
#include "drivers/InterruptHandler.h"
#include "proc/Thread.h"

typedef Heap<Thread*, THREAD_HEAP_CHILDREN> ThreadHeap;

/*! class Timer Timer.h "timer/Timer.h"
 * @brief Timer class keeps truck of sleeping threads that wish to be awaken 
 * on given time.
 *
 * This class only implements two methods, plan and interupt. Plan is used 
 * to add an event to the event heap. Interupt signals that some events might
 * be due. Future interupts are planned using timer interrupts.
 */
class Timer:public InterruptHandler, public Singleton<Timer>
{
public:
	/*! @brief Adds thread that wish to be waken on given time to the event heap.
	 *
	 * @param thread Thread to handle.
	 * @param time Time of event.
	 */
	void plan(Thread* thread, const Time& time);

	/*! @brief Signals that some events might be due. 
	 *
	 * Takes the thread and acts according to it's status:
	 *		RUNNING threads are switched (at the end of processing)
	 *		other threads are enqueued to the running queue.
	 */
	void handleInterrupt();

private:
	/*! @brief Event heap */
	ThreadHeap m_heap;
};
