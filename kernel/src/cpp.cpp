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
 * @brief Needed cpp stuff.
 *
 * This file contains routines that handle pure virtual function calls and
 * exit funtion.
 */

#include "Kernel.h"

extern "C"
{
	/*! pure virtual function call results in kernel panic */
	void __cxa_pure_virtual()
	{
			panic("Pure virtual function call.\n");
	}
	/*! no dso */
	void* __dso_handle = 0;
	/*! nothing is done at exit */
	int __cxa_atexit(void (*func) (void*), void* arg, void* dso_handle) { return 0; }
}

void* operator new(unsigned int size) { return malloc(size); };
void operator delete (void* p) { Kernel::instance().free(p); };
void* operator new[](unsigned int size) { return malloc(size); };
void operator delete[] (void* p) { Kernel::instance().free(p); };
