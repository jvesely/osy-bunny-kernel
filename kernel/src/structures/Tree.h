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

#include "api.h"

class Node;

template<class MyNode>
class Tree
{
public:

	Tree(): m_root( NULL ), m_count( 0 ) {};

	bool contains( const MyNode& item );

	bool contains( const MyNode* itemptr );

	bool insert( const MyNode& item );

	bool insert( MyNode* itemptr );

	bool remove( const MyNode& item );

	bool remove( MyNode* item );

	MyNode& take( const MyNode& item );

	MyNode find( const MyNode& item );

	MyNode* findItem( const MyNode& item );

	MyNode& min();

	MyNode& max();
	
	void clear();

	inline uint count() { return m_count; };

	void printInfix() { if (m_root) m_root->printInfix(); }

	void printPrefix() { if (m_root) m_root->printPrefix(); }

protected:

	MyNode* m_root;
	uint m_count;

	Tree( const Tree& other );
	Tree& operator = ( const Tree& other );

	friend class Node;
};

/*************************************
 * DEFINITIONS
 ************************************/

template <class MyNode>
bool Tree<MyNode>::contains( const MyNode& item )
{
	return findItem( item );
}
/*----------------------------------------------------------------------------*/
template <class MyNode>
bool Tree<MyNode>::contains( const MyNode* item )
{
	return item->m_myTree == (Tree<Node>*)(this);
}
/*----------------------------------------------------------------------------*/
template <class MyNode>
bool Tree<MyNode>::insert( const MyNode& item )
{
	MyNode* new_item = new MyNode( item );
//	std::cout << "Created new Item: " << new_item << " data: " << new_item->data() << std::endl;
	return insert( new_item );
}
/*----------------------------------------------------------------------------*/
template <class MyNode>
bool Tree<MyNode>::insert( MyNode* itemptr )
{
	ASSERT (itemptr);

	if (m_root) 
		return m_root->subtreeInsert(itemptr);

	itemptr->m_myTree = (Tree<Node>*)this;
	m_root = itemptr;
	++m_count;
	return true;
}
/*----------------------------------------------------------------------------*/
template <class MyNode>
bool Tree<MyNode>::remove( const MyNode& item )
{
	const MyNode* my_item = findItem( item );
	delete my_item;
	return my_item;
}
/*----------------------------------------------------------------------------*/
template <class MyNode>
MyNode& Tree<MyNode>::take( const MyNode& item )
{
	MyNode* my_item = findItem( item ); 
	if (my_item) {
		ASSERT (my_item->m_myTree == (Tree<Node>*)this);
		my_item->removeFromTree();
	}
	return *my_item;
}
/*----------------------------------------------------------------------------*/
template <class MyNode>
MyNode Tree<MyNode>::find( const MyNode& item )
{
	MyNode* my_item = findItem( item );

	return my_item ? *my_item : item;
}
/*----------------------------------------------------------------------------*/
template <class MyNode>
MyNode* Tree<MyNode>::findItem( const MyNode& item )
{
	if (m_root)
		return m_root->subtreeFindNode( item );
	return NULL;
}
/*----------------------------------------------------------------------------*/
template <class MyNode>
void Tree<MyNode>::clear()
{
	while (m_root) delete m_root;
}
/*----------------------------------------------------------------------------*/
template <class MyNode>
MyNode& Tree<MyNode>::min()
{
	ASSERT (m_root);
	return *static_cast<MyNode*>(m_root->subtreeMinNode());
}
/*----------------------------------------------------------------------------*/
template <class MyNode>
MyNode& Tree<MyNode>::max()
{
	ASSERT (m_root);
	return *static_cast<MyNode*>(m_root->subtreeMaxNode());
}
