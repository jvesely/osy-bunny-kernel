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

#include "DirEntry.h"
#include "api.h"

#define DIRENTRY_DEBUG

#ifndef DIRENTRY_DEBUG

#define PRINT_DEBUG(...)
#else
#define PRINT_DEBUG(ARGS...)\
  puts("[ DIR DEBUG ]: ");\
	  printf(ARGS);
#endif

bool DirEntry::addSubEntry( char* name, Entry* entry )
{
	String str( name );
	PRINT_DEBUG ("Adding SubEntry %s.\n", str.cstr() );
	m_subEntries.pushBack(NamePair(str, entry));
	return true;
}
/*----------------------------------------------------------------------------*/
String DirEntry::firstEntry()
{
	if (m_subEntries.size())
		return m_subEntries.begin()->first;
	return String();
}
/*----------------------------------------------------------------------------*/
String DirEntry::nextEntry( String previous )
{
	EntryList::Iterator it = m_subEntries.begin();
	for (; it != m_subEntries.end(); ++it) {
		if (it->first == previous ){
				++it; break;
		}
	}
	if (it == m_subEntries.end())
		return String();
	else
		return it->first;
}
