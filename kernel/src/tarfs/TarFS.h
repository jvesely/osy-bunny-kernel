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

#include "VFS.h"
#include "structures/HashMap.h"
class Entry;

typedef HashMap<file_t, Entry*> EntryMap;

class TarFS: public VFS
{
public:
	TarFS( DiscDevice* disk );
	bool mount( DiscDevice* disk );
	file_t openFile( char file_name[], char mode );
	void closeFile( file_t file );
	ssize_t readFile( file_t src, void* buffer, size_t size );
	uint seekFile( file_t file, FilePos pos, uint offset);
	bool existsFile( file_t file );
	size_t sizeFile( file_t file );
	bool eof( file_t file );

private:
	EntryMap m_entryMap;
	Entry* m_rootDir;
	DiscDevice* m_mountedDisk;
};
