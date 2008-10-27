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
 * @brief Shared Thread class.
 *
 * Long description. I would paste some Loren Ipsum rubbish here, but I'm afraid
 * It would stay that way. Not that this comment is by any means ingenious but 
 * at least people can understand it. 
 */
#pragma once

#include "api.h"

class Thread
{
	struct Context
	{
		unative_t zero;

		unative_t at;
		unative_t v0;
		unative_t v1;

		unative_t a0;
		unative_t a1;
		unative_t a2;
		unative_t a3;

		unative_t t0;
		unative_t t1;
		unative_t t2;
		unative_t t3;
		unative_t t4;
		unative_t t5;
		unative_t t6;
		unative_t t7;
		unative_t t8;
		unative_t t9;

		unative_t s0;
		unative_t s1;
		unative_t s2;
		unative_t s3;
		unative_t s4;
		unative_t s5;
		unative_t s6;
		unative_t s7;

		unative_t k0;
		unative_t k1;

		unative_t gp;
		unative_t fp;

		unative_t sp;
		unative_t ra;

		unative_t lo;
		unative_t hi;

		unative_t epc;
		unative_t cause;
		unative_t badva;
		unative_t status;
	};

public:
	static const uint32_t DEFAULT_STACK_SIZE = 0x1000; /*!< 4KB */
	/* creation myth */
	inline Thread(
		void (*func)(void*), 
		void* data, 
		uint32_t flags = 0,
		uint32_t stackSize = DEFAULT_STACK_SIZE):
		 m_stack(NULL), m_stackSize(stackSize), m_runFunc(func), m_runData(data)
		{ assert(func); assert(stackSize); };

	/* this method will be run in separate thread */
	virtual void run();
	/* put initial registers on stack, set pointer to run */
	uint32_t setup();
	
	void start() { run(); };

	void yield();	


	inline bool detached() const { return m_detached; }

	inline bool detach() { return m_detached = true; }

	void sleep(const unsigned int sec);

	void usleep(const unsigned int usec);

	int wakeup();

	int kill();

	~Thread();

	inline thread_t id() { return m_id; };

	inline void setId(thread_t id) { m_id = id; };

	inline void** stackTop() { return &m_stackTop; };

protected:
	/* that's my stack */
	void* m_stack;
	void* m_stackTop;
	uint32_t m_stackSize;

	/* I'm supposed to run this stuff */
	void (*m_runFunc)(void*);
	
	/* runFunc expects this */
	void* m_runData;

	/* detached flag */
	bool m_detached;

	thread_t m_id;

private:
	Thread();
	Thread(const Thread& other);
	const Thread& operator=(const Thread& other);
};
