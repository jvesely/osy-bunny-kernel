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
 * @brief DirEntry class declaration.
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

/*!
 * @class DirEntry DirEntry.h "tarfs/DirEntry.h"
 * @brief Class representing directory on TarFs.
 *
 * Class stores Pairs identifying names of the directory Entries with
 * their respective Entries. This map is stored in the SplayTree.
 */
class DirEntry: public Entry
{
public:
	/*!
	 * @brief No device to store Direntry data on.
	 */
	DirEntry(DiskDevice* storage = NULL):Entry( storage ), m_opencount( 0 ),
		m_nextEntry( NULL )
		{};

	/*!
	 * @brief Adds name->Entry maping coresponding to the item in this dir.
	 * @return @a true if Entry was sucessfully added, @a false otherwise.
	 */
	bool addSubEntry( char* name, Entry* entry );

	/*!
	 * @brief Gets the name of the first Entry in this directory,
	 * 	in alphanumeric order.
	 * @return Name of the first Entry.
	 */
	const String firstEntry();

	/*!
	 * @brief Gets Name of the Entry following immediatelly the given Entry.
	 * @param previous Name of the preceding Entry.
	 * @return Name of the following Entry.
	 */
	const String nextEntry( const String& previous );

	const Pair<String, Entry*> nextEntry();

	/*!
	 * @brief Translates Name into the Entry*.
	 * @param name Name of the Entry.
	 * @return Ptr to the corresponding entry, NULL on failure.
	 */
	Entry* subEntry( const String& name );

	/*!
	 * @brief Converts self to DirEntry pointer.
	 * @return this, always.
	 */
	DirEntry* dirEntry() { return this; };

	/*!
	 * @brief Gets the number of Entries in this directory.
	 * @return Number of Entries.
	 */
	size_t size() const { return m_subEntries.count(); };

	/*!
	 * @brief Fails the reading operation as directories cannot be read
	 * 	in the way files can.
	 * @param buffer Ignored.
	 * @param size Ignored.
	 * @return EIO, always.
	 */
	ssize_t read( void* buffer, int size ) { return EIO; };

	/*!
	 * @brief Fails the seek operation as directories do not
	 * 	support this operation.
	 * @param pos Reference point for offset. 
	 * @param offset Real position as offset from pos.
	 * @return New position.
	 *
	 * @note Reference point POS_CURRENT is ingored and treated as POS_START
	 */
	uint seek( FilePos pos, int offset );

	/*!
	 * @brief Marks directory as opened and increases open counter.
	 * @param mode Ignored. Provided for Entry compatibility.
	 * @return @a True always.
	 */
	bool open( char mode = 0 ) { return ++m_opencount,true; };

	/*!
	 * @brief Decrease open count. And resets postion indicator if no more 
	 * opens are pending.
	 */
	void close() 
	{ 
		if (m_opencount) --m_opencount; 
		if (!m_opencount) m_nextEntry = m_subEntries.min();
	};

private:
	EntryList m_subEntries;  /*!< List of stored Entries. */
	uint m_opencount;        /*!< Count of open calls - close calls.*/

	/*! Position wihtin directory. */
	BinaryNode<Pair<String, Entry*> >* m_nextEntry; };
