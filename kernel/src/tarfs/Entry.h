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

#pragma once

#include "types.h"
class DiskDevice;

class Entry
{
public:
	Entry(DiskDevice* storage): m_storage( storage ) {};
	virtual size_t size() const = 0;
	virtual ssize_t read( void* buffer, int size ) = 0;
	virtual uint seek( FilePos pos, int offset ) = 0;
	virtual bool open( const char mode ) { return false; };
	virtual void close() {};
	virtual bool addSubEntry( char* name, Entry* entry ) { return false; };
	virtual ~Entry() {};
protected:
	Entry( const Entry& other );
	Entry& operator = (const Entry& other);
	bool readFromDevice(void* buffer, size_t count, uint start_block, uint offset);
private:
	DiskDevice* m_storage;
};
