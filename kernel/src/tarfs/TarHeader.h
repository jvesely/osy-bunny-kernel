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
 * @brief TarHeader class declaration and implementation.
 *
 * Long description. I would paste some Loren Ipsum rubbish here, but I'm afraid
 * It would stay that way. Not that this comment is by any means ingenious but 
 * at least people can understand it. 
 */

#pragma once

#include "types.h"
#include "api.h"

class Entry;

/*!
 * @class TarHeader TarHeader.h "tarfs/TarHeader.h"
 * @brief Provides simple parsing of the TAR headers format.
 */
class TarHeader
{
public:
	/*! @brief Types of data in TAR. */
	enum FileType {
		Unknown, File, HardLink, SymLink, Directory, Character, Block, FIFO
	};
	
	/*! @brief Gets file name. */
	inline char*    fileName() { return m_fileName; };

	/*! @brief Gets size converted from TAR format. */
	inline uint     fileSize();

	/*! @brief Gets type of the file. */
	inline FileType fileType();
private:
	char m_fileName[100];
	char m_mode[8];
	char m_uid[8];
	char m_gid[8];
	char m_fileSize[12];
	char m_modTime[12];
	char m_checksum[8];
	char m_link;
	char m_linkName[100];
	byte m_zeros[255];
	
	friend class Entry;
};

/*----------------------------------------------------------------------------*/
/* DEFINITIONS ---------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
inline uint TarHeader::fileSize()
{
	uint res = 0;
	for (int i = 1; i < 11; ++i){
	//	printf("<%c,%u> ", m_fileSize[i], m_fileSize[i]);
		res = (res * 8) + (m_fileSize[i] - '0');
	}
	return res;
}
/*----------------------------------------------------------------------------*/
inline TarHeader::FileType TarHeader::fileType()
{
	switch (m_link){
		case '\0':
		case '7':  /* Special Continous file is treated as file. */
		case '0':
					return File;
		case '1':
					return HardLink;
		case '2':
					return SymLink;
		case '3':
					return Character;
		case '4':
					return Block;
		case '5':
					return Directory;
		case '6':
					return FIFO;
	}
	
	return Unknown;
}
