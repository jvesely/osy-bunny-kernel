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
 * @brief Generic Tree container class.
 *
 * Contains template class Tree, declaration and implementation. Properties of 
 * Tree depends on the Node class it is instantiated by.
 */
#pragma once

#include "api.h"

/*! 
 * @class Tree Tree.h "structures/Tree.h"
 * @brief Implementation of generic tree.
 *
 * Template class:
 * @param MyNode type of Node the Tree should use to store data.
 * Needs to have these member functions implemented:
 * 	operator==() used in searching makes two nodes equal
 * 	operator<() used to sort Nodes in the tree
 * 	subtreeInsert( Node* other) insert another node into node's subtree
 * 	subtreeFindNode( cosnt Node& other) find node that == "other" Node
 * 	subtreeMinNode() find minimal node in subtree
 * 	subtreeMaxNode() find maximal node in subtree
 * Properties of the tree are dependant on the type of Node beeing used.
 * Class Tree only provides wrapping and root creation.
 */
class Node;

template<class MyNode>
class Tree
{
public:
	/*! @brief Creates empty tree. */
	Tree(): m_root( NULL ), m_count( 0 ) {};

	bool checkOK();
	/*!
	 * @brief Checks for existance of item in the tree.
	 *
	 * Checks for existance of Node that complies with:  Node == item
	 * @param item Node to find.
	 * @return @a true if node == item was found, @a false otherwise.
	 * Overloaded function provided for convenience.
	 */
	bool contains( const MyNode& item );

	/*!
	 * @brief Checks for presence of the given node in the tree.
	 *
	 * Checks whether the item is present in the tree.
	 * @param itemptr Pointer to Node to be checked.
	 * @return @a true if node == item was found, @a false otherwise.
	 */
	bool contains( const MyNode* itemptr );

	/*!
	 * @brief Inserts COPY of the input item into the tree.
	 *
	 * @param item Node to copy and insert.
	 * @return @a true if the copy was sucessfully inserted, @a false otherwise.
	 * @note If conversion constructor exists this works for inserting data that
	 * are to be stored in the tree.
	 * Overloaded function provided for convenience.
	 */
	bool insert( const MyNode& item );

	/*!
	 * @brief Inserts node into the tree.
	 *
	 * @param itemptr Pointer to the Node that should be inserted.
	 * @return @a true if item was sucessfully inserted, @a false otherwise.
	 * Inserts node pointed to by itemptr.
	 */
	bool insert( MyNode* itemptr );

	/*!
	 * @brief Removes item from the tree.
	 *
	 * @param item Node to find and remove.
	 * @return @a true if Node == item was found and removed, @a false otherwise.
	 */
	bool remove( const MyNode& item );

	/*!
	 * @brief Removes item from the tree.
	 *
	 * @param itemptr Pointer to the item to be removed.
	 * @return @a true if item was present in the tree and removed, 
	 * @a false otherwise.
	 * This function is provided for convenience, Nodes should provide 
	 * safe mechanism for removing from the tree upon deletion.
	 */
	bool remove( MyNode* itemptr );

	/*!
	 * @brief Finds and disconnects the item from the tree.
	 *
	 * @param item Node to be taken from the tree.
	 * @return reference to the found node, undefined if coresponding node was
	 * not present in the tree.
	 * Function provided for convenience.
	 */
	MyNode& take( const MyNode& item );

	/*!
	 * @brief Finds item in the tree.
	 *
	 * @param item Item to find.
	 * @return @a Copy of the found item if it was found, item copy-constructed
	 * from input otherwise.
	 * Function searches for Node that complies Node == item.
	 */
	MyNode find( const MyNode& item );

	/*!
	 * @brief Finds item in the tree.
	 *
	 * @param item Item to find.
	 * @return Pointer to the found item (if it was found), NULL otherwise.
	 */
	MyNode* findItem( const MyNode& item );

	/*!
	 * @brief Finds minimal Node in the tree.
	 *
	 * @return Reference to the minimal Node in the tree.
	 * Tree must contain at least one Node, otherwise the result is undefined.
	 */
	MyNode& min();

  /*!
	 * @brief Finds maximal Node in the tree.
	 * 
	 * @return Reference to the maximal Node in the tree.
	 * Tree must contain at least one Node, otherwise the result is undefined.
	 */
	MyNode& max();

	/*!
	 * @brief Deletes ALL Nodes currently in the tree.
	 */
	void clear();

	/*!
	 * @brief Get number of the inserted elements.
	 *
	 * @return Number of the Nodes in the tree.
	 */
	inline uint count() { return m_count; };

protected:

	/*! Here the tree begins */
	MyNode* m_root;

	/*! Numberr of the Nodes */
	uint m_count;

	/*! These shall not be used */
	Tree( const Tree& other );
	Tree& operator = ( const Tree& other );

	/*! Nodes may need to change the count or root */
	friend class Node;
};

/*----------------------------------------------------------------------------*/
/* DEFINITIONS ---------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
template <class MyNode>
bool Tree<MyNode>::checkOK()
{
	return (m_root == NULL) || m_root->checkOK( (Tree<Node>*)this );
}
/*----------------------------------------------------------------------------*/
template <class MyNode>
bool Tree<MyNode>::contains( const MyNode& item )
{
	/* conversion to bool shalll do the trick */
	return findItem( item );
}
/*----------------------------------------------------------------------------*/
template <class MyNode>
bool Tree<MyNode>::contains( const MyNode* item )
{
	/* just check if Node's tree is me */
	return item->m_myTree == (Tree<Node>*)(this);
}
/*----------------------------------------------------------------------------*/
template <class MyNode>
bool Tree<MyNode>::insert( const MyNode& item )
{
	/* create copy */
	MyNode* new_item = new MyNode( item );

	if (!new_item) return false;

	/* insert if it was successfully created */
	return insert( new_item );
}
/*----------------------------------------------------------------------------*/
template <class MyNode>
bool Tree<MyNode>::insert( MyNode* itemptr )
{
	ASSERT (itemptr);
	
	/* if there is a tree let root handle this */
	if (m_root) 
		return m_root->subtreeInsert(itemptr);

	/* otherwise create root */
	itemptr->m_myTree = (Tree<Node>*)this;
	m_root = itemptr;
	++m_count;
	return true;
}
/*----------------------------------------------------------------------------*/
template <class MyNode>
bool Tree<MyNode>::remove( const MyNode& item )
{
	/* seek */
	const MyNode* my_item = findItem( item );
	
	/* and destroy, this is NULL safe */
	return (delete my_item), my_item;

	/* use invalid pointer converted to bool */
	return my_item;
}
/*----------------------------------------------------------------------------*/
template <class MyNode>
bool Tree<MyNode>::remove( MyNode* itemptr )
{
	/* if it's not here just return */
	if ( !contains(itemptr) )
		return false;
	
	/* otherwise delete and report success */
	delete itemptr;
	return true;
}
/*----------------------------------------------------------------------------*/
template <class MyNode>
MyNode& Tree<MyNode>::take( const MyNode& item )
{
	/* find first */
	MyNode* my_item = findItem( item ); 

	if (my_item) {
		ASSERT (my_item->m_myTree == (Tree<Node>*)this);
		/* cut all the connestions */
		my_item->removeFromTree();
	}

	/* return found, or invalid */
	return *my_item;
}
/*----------------------------------------------------------------------------*/
template <class MyNode>
MyNode Tree<MyNode>::find( const MyNode& item )
{
	/* find */
	MyNode* my_item = findItem( item );

	/* return dereferenced found or invalid */
	return my_item ? *my_item : item;
}
/*----------------------------------------------------------------------------*/
template <class MyNode>
MyNode* Tree<MyNode>::findItem( const MyNode& item )
{

	if (m_root)
		return m_root->subtreeFindNode( item );
	
	/* no tree, means nothing to return */
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
