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
#include "api.h"

class TarHeader
{
public:
	inline char* fileName() { return m_fileName; };
	inline uint  fileSize();
private:
	char m_fileName[100];
	char m_mod[8];
	char m_uid[8];
	char m_gid[8];
	char m_fileSize[12];
	char m_modTime[12];
	char m_checksum[8];
	char m_link;
	char m_linkName[100];
	byte zeros[255];
};

/*----------------------------------------------------------------------------*/
/* DEFINITIONS ---------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
uint TarHeader::fileSize()
{
	uint res = 0;
	for (int i = 1; i < 11; ++i){
	//	printf("<%c,%u> ", m_fileSize[i], m_fileSize[i]);
		res = (res * 8) + (m_fileSize[i] - '0');
	}
	return res;
}
