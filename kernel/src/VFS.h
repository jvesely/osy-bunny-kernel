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
typedef int file_t;



class VFS
{
public:

	VFS(){};
	virtual bool mount( DiskDevice* disk ) = 0;
	virtual file_t openFile( const char file_name[], const char mode ) = 0;
	virtual void closeFile( file_t file ) = 0;
	virtual ssize_t readFile( file_t src, void* buffer, size_t size ) = 0;
	virtual uint seekFile( file_t file, FilePos pos, int offset ) = 0; 
	virtual bool existsFile( file_t file ) = 0;
	virtual bool eof( file_t file ) = 0;

private:
	
	VFS( const VFS& );
};
