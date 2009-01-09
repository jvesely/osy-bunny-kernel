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
#include "TarHeader.h"
#include "Entry.h"

class DiscDevice;

class FileEntry: public Entry
{
public:
	FileEntry( TarHeader& tarHeader, uint block, DiscDevice* disc );
	size_t size() const { return m_size; };
	ssize_t read( void* buffer, int size );
	uint seek( FilePos pos, int offset );
	bool open( const char mode );
	void close();
	~FileEntry();
		
private:
	uint m_uid;
	uint m_gid;
	uint m_size;
	uint m_startPos;
	uint m_modTime;
	uint m_readCount;
	uint m_pos;
		
	FileEntry( const FileEntry& other );
	FileEntry& operator = ( const FileEntry& other );
};
