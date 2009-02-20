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

#include "FileEntry.h"

#define ENTRY_DEBUG

#ifndef ENTRY_DEBUG
#define PRINT_DEBUG(...)
#else
#define PRINT_DEBUG(ARGS...)\
  puts("[ FILE ENTRY DEBUG ]: ");\
  printf(ARGS);
#endif


FileEntry::FileEntry( TarHeader& header, uint start_block, DiskDevice* disk ):
	Entry( disk )
{
	m_size = header.fileSize();
	m_startPos = start_block + 1;
}
/*----------------------------------------------------------------------------*/
bool FileEntry::open( const char mode )
{
	if ( (mode & OPEN_W) || (mode & OPEN_A) ) return false;
	++m_readCount;
	return true;
}
/*----------------------------------------------------------------------------*/
void FileEntry::close()
{
	ASSERT (m_readCount);
	--m_readCount;
	if (!m_readCount) m_pos = 0;
}
/*----------------------------------------------------------------------------*/
ssize_t FileEntry::read( void* buffer, int size )
{
	if (size < 0 || (m_pos + size) > m_size) return EIO;
	bool res = readFromDevice( buffer, size, m_startPos, m_pos);
	PRINT_DEBUG("Reading from Entry to buffer %p(%u): %s\n", 
		buffer, size, res ? "OK": "FAIL" );

	return res ? size : EIO;
}
/*----------------------------------------------------------------------------*/
uint FileEntry::seek( FilePos pos, int offset )
{
	PRINT_DEBUG ("Seeking pos: %u, offset %u.\n", pos, offset);
	switch (pos) {
		case POS_START:
			if (offset >= 0 && (uint)offset < m_size) m_pos = offset;
			break;
		case POS_CURRENT:
			if (offset > 0 && (m_pos + offset) < m_size) m_pos += offset;
			if (offset < 0 && (m_pos >= (uint)-offset))  m_pos -= offset;
			break;
		case POS_END:
			if (offset <= 0 && (m_size  > (uint)-offset)) m_pos = m_size + offset;
	}
	PRINT_DEBUG ("Result: %u.\n", m_pos);
	return m_pos;
}
/*----------------------------------------------------------------------------*/
FileEntry::~FileEntry()
{}
