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

#include "api.h"
#include "DirEntry.h"

//#define DIRENTRY_DEBUG

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
	m_subEntries.insert( NamePair(str, entry) );
	return true;
}
/*----------------------------------------------------------------------------*/
const String DirEntry::firstEntry()
{
	PRINT_DEBUG("asked for the first entry.\n");
	String ret;
	if (m_subEntries.count()) {
		 ret = m_subEntries.min().data().first;
		 PRINT_DEBUG ("There should be an answer,\n");
	}
	ASSERT(m_subEntries.checkOK());
	PRINT_DEBUG ("Asked for the first entry got: \"%s\".\n",ret.cstr()?ret.cstr():"EMPTY");
	return ret;
}
/*----------------------------------------------------------------------------*/
const String DirEntry::nextEntry( const String& previous )
{
	SplayBinaryNode<NamePair> * entry =
		m_subEntries.findItem( NamePair( previous, NULL ) );
	ASSERT (m_subEntries.checkOK());
	String ret;
	if (entry && entry->next()) {
		ret = entry->next()->data().first;
	}
	PRINT_DEBUG ("Asked for entry following \"%s\" got \"%s\".\n",
		previous.cstr(), ret.cstr()?ret.cstr():"EMPTY");
	return ret;
}
/*----------------------------------------------------------------------------*/
Entry* DirEntry::subEntry( const String& name )
{
	SplayBinaryNode<NamePair> * entry =
		m_subEntries.findItem( NamePair( name, NULL ) );
	PRINT_DEBUG ("Found subentry: %s at ptr %p.\n", name.cstr(), entry);
	return entry ? entry->data().second : NULL;
}
/*----------------------------------------------------------------------------*/
uint DirEntry::seek( FilePos pos, int offset )
{
	if (m_subEntries.count() == 0)
		return 0;
	switch (pos) {
		case POS_CURRENT:
		case POS_START:
			m_nextEntry = &m_subEntries.min(); 
			ASSERT(m_nextEntry);
			if (offset < 0 || (uint)offset > m_subEntries.count())
				return 0;
			for (int i = 0; i < offset; ++i) {
				ASSERT(m_nextEntry);
				m_nextEntry = m_nextEntry->next();
			}
			return offset;
		case POS_END:
			m_nextEntry =  &m_subEntries.max();
			ASSERT(m_nextEntry);
			if (offset > 0 || (uint)-offset > m_subEntries.count())
				return m_subEntries.count();
			for (int i = 0; i < offset; ++i) {
				ASSERT(m_nextEntry);
				m_nextEntry = m_nextEntry->previous();
			}
			return m_subEntries.count() - offset;
	}
	m_nextEntry = &m_subEntries.min();
	return 0;
}
/*----------------------------------------------------------------------------*/
const Pair<String, Entry*> DirEntry::nextEntry()
{
	if (!m_nextEntry)
		return Pair<String, Entry*>("", NULL);
	
	return m_nextEntry->data();
	
}
