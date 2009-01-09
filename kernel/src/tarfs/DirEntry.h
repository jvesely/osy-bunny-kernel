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
#include "Entry.h"
#include "String.h"
#include "structures/Pair.h"
#include "structures/Trees.h"

template class Pair<String, Entry*>;
template class Tree< SplayBinaryNode<Pair<String, Entry*> > >;

typedef Pair<String, Entry*> NamePair;
typedef Trees<NamePair>::SplayTree EntryList;

class DirEntry: public Entry
{
public:
	DirEntry(){};
	bool addSubEntry( char* name, Entry* entry );
	String firstEntry();
	String nextEntry( String previous );
private:
	EntryList m_subEntries;
};
