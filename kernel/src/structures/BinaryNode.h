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
 * @brief Class BinaryNode.
 *
 * BinaryNode template class declaration and implementation.
 */
#pragma once

#include "api.h"
#include "Node.h"

/*!
 * @class BinaryNode BinaryNode.h "structures/BinaryNode.h"
 * @brief Represents Node in the Binary tree.
 *
 * Template class:
 * @param T Type of the data to store in the Node. 
 * 	It is required to have operator== and operator< defined.
 * Implemnets standard order tree Node, i.e. smaller items are stored in 
 * the left subtree, bigger in the right. Does not accept multiple same values
 * in one tree.
 */

template <typename T>
class BinaryNode:public Node
{
public:

	/*! @brief Creates Node that stores data.
	 * @param data data to be stored.
	 * @param tree BinaryTree to be inserted into.
	 *
	 * It is possible to uses this constructor as converse constructor from type
	 * T to type BinaryTree<T>.
	 */
	BinaryNode( const T& data, Tree< BinaryNode<T> >* tree = NULL );

	/*!
	 * @brief Correctly destroys Node.
	 * If the Node was inserted in a tree is is removed from the tree before 
	 * destruction.
	 */
	virtual ~BinaryNode();

	/*!
	 * @brief Gets data reference.
	 * @return Reference to the stored data.
	 * Data should not be changed, as it may result in breaking the 
	 * Node order in the tree.
	 */
	const T& data() const { return m_data; };

	/*! 
	 * @brief Inserts node into the subtree.
	 * @return @a true if node was successfully inserted, @a false otherwise.
	 *
	 * Rejects node if duplicate node already exists in the tree.
	 */
	virtual bool subtreeInsert( BinaryNode* nodeptr );

	/*!
	 * @brief Searches for node in the subtree.
	 * @return Pointer to the found node on success, NULL otherwise.
	 *
	 * Searches for Node that complies Node == other in the subtree.
	 */
	virtual BinaryNode* subtreeFindNode( const BinaryNode<T>& other );

	/*!
	 * @brief Gets leftmost Node in the subtree.
	 * @return Pointer to the leftmost Node.
	 */
	virtual BinaryNode* subtreeMinNode();

	/*!
	 * @brief Gets rightmost Node in the subtree.
	 * @return Pointer to the rightmost Node.
	 */
	virtual BinaryNode* subtreeMaxNode();

	/*!
	 * @brief Correctly removes the Node from the Tree.
	 */
	void removeFromTree();

	/*!
	 * @brief Gets pointer to the next (nearest larger) Node.
	 * @return Pointer to the nearest lager Node, NULL if no such Node exists.
	 */
	BinaryNode<T>* next() const { return m_next; };

	/*!
	 * @brief Gets pointer to the previous (nearest smaller) Node.
	 * @return Pointer to the nearest smaller Node, NULL if no such Node exists.
	 */
	BinaryNode<T>* previous() const { return m_previous; };

	/*!
	 * @brief Tests Nodes for equality, using m_data members.
	 * @return @a true if this.m_data == other.m_data, @a false otherwise.
	 *
	 * Just a wrapperr that compares m_data parts.
	 */
	virtual bool operator == ( const BinaryNode<T>& other ) const;

	/*!
	 * @brief Compares Nodes, using m_data members.
	 * @return @a true if this.m_data < other.m_data, @a false otherwise.
	 *
	 * Just a wrapperr that compares m_data parts.
	 */
	virtual bool operator < ( const BinaryNode<T>& other ) const;

protected:

	/*! Stored data */
	T m_data;

	BinaryNode<T>* m_parent;
	BinaryNode<T>* m_left;
	BinaryNode<T>* m_right;
	BinaryNode<T>* m_previous;
	BinaryNode<T>* m_next;

	/*! @brief Rotates with the left son, left son must exist. */
	void rotateLeft();

	/*! @brief Rotates with the right son, right son must exist. */
	void rotateRight();

	/*! @brief Tests if the Node is the left son of its parent.
	 * @return @a true if the Node has parent and parent.left is the Node, 
	 * @a false otherwise.
	 */
	bool isLeftSon() const;

	BinaryNode( const BinaryNode& other );
	BinaryNode& operator = ( const BinaryNode& other );

	/* Tree may need to alter or check my private/protected members */
	friend class Tree< BinaryNode<T> >;
};

/*----------------------------------------------------------------------------*/
/* DEFINITIONS ---------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

template <typename T>
BinaryNode<T>::BinaryNode( const T& data, Tree< BinaryNode<T> >* tree )
	: Node( (Tree< Node >*)tree ), m_data( data ),
	  m_parent( NULL), m_left( NULL ), m_right( NULL ),
		m_previous( NULL ), m_next( NULL )
{
}
/*----------------------------------------------------------------------------*/
template <typename T>
BinaryNode<T>::BinaryNode( const BinaryNode<T>& other )
	: Node( NULL ), m_data( other.m_data ),
	 m_parent( NULL ), m_left( NULL ), m_right( NULL ),
	 m_previous( NULL ), m_next( NULL )
{
}
/*----------------------------------------------------------------------------*/
template <typename T>
bool BinaryNode<T>::subtreeInsert( BinaryNode* item )
{
	/* no duplicates */
	ASSERT ( !(*item == *this) ); 
	
	if (*item < *this) {
		
		if (m_left)
			return m_left->subtreeInsert( item );
		
		/* insert as my left son */
		item->m_parent = this;
		if (m_previous)
			m_previous->m_next = item;
		item->m_previous = m_previous;
		m_previous = item;
		item->m_next = this;
		m_left = item;
		item->m_myTree = this->m_myTree;
		treeCount() += 1;
		return true;
	}

	if (*this < *item) {

		if (m_right)
			return m_right->subtreeInsert( item );

		/* insert as my rigt son */
		item->m_parent = this;
		if (m_next)
			m_next->m_previous = item;
		item->m_next = m_next;
		m_next = item;
		item->m_previous = this;
		m_right = item;
		item->m_myTree = m_myTree;
		treeCount() += 1;
		return true;
	}

	return false;	
}
/*----------------------------------------------------------------------------*/
template <typename T>
BinaryNode<T>* BinaryNode<T>::subtreeFindNode( const BinaryNode& other )
{
	/* check left tree */
	if (other < *this) {
		return m_left ? m_left->subtreeFindNode( other ) : NULL;
	}

	/* check right tree */
	if (*this < other) {
		return m_right ? m_right->subtreeFindNode( other ) : NULL;
	}

	/* check me */
	return (*this == other) ? this : NULL;
}
/*----------------------------------------------------------------------------*/
template <typename T>
BinaryNode<T>* BinaryNode<T>::subtreeMaxNode()
{
	return m_right ? m_right->subtreeMaxNode() : this;
}
/*----------------------------------------------------------------------------*/
template <typename T>
BinaryNode<T>* BinaryNode<T>::subtreeMinNode()
{
	return m_left ? m_left->subtreeMinNode() : this;
}
/*----------------------------------------------------------------------------*/
template <typename T>
void BinaryNode<T>::removeFromTree()
{
	if (!this->m_myTree)
		return;

	/* get rid of at least one son */
	while (m_left && m_right) {
		rotateLeft();
	}

	ASSERT (!m_left || !m_right);

	/* this is my only son */
	BinaryNode<T>* only_son = (BinaryNode<T>*)((uintptr_t)m_left | (uintptr_t)m_right);

	ASSERT (only_son == m_left || only_son == m_right);

	if (only_son)
		only_son->m_parent = m_parent;

	/* take out of the sorted chain */
	if (m_previous)
		m_previous->m_next = m_next;

	if (m_next)
		m_next->m_previous = m_previous;


	/* I am root */
	if (!m_parent) {
		treeRoot() = only_son; // place new root
	} else {
		/* Let my son take my place */
		if (isLeftSon()) {
			m_parent->m_left = only_son;
		} else {
			m_parent->m_right = only_son;
		}
	}

	m_left = m_right = m_parent = m_next = m_previous = NULL;
	treeCount() -= 1;
	m_myTree = NULL;
	return;
}
/*----------------------------------------------------------------------------*/
template <typename T>
BinaryNode<T>::~BinaryNode()
{
	/* remove before deletion */
	removeFromTree();
}
/*----------------------------------------------------------------------------*/
template <typename T>
bool BinaryNode<T>::isLeftSon() const
{
	return ( m_parent && (m_parent->m_left == this) );
}
/*----------------------------------------------------------------------------*/
template <typename T>
void BinaryNode<T>::rotateLeft()
{
	ASSERT (m_left);

	m_left->m_parent = m_parent;
	
	if (m_parent) {
		/* My left son takes my place */
		if (isLeftSon()) m_parent->m_left = m_left;
		else m_parent->m_right = m_left;
	} else {
		/* I was root */
		treeRoot() = m_left;
	}

	/* Son Becomes parent */
	m_parent = m_left;
	m_left = m_left->m_right;
	if (m_left) m_left->m_parent = this;
	m_parent->m_right = this;
}
/*----------------------------------------------------------------------------*/
template <typename T>
void BinaryNode<T>::rotateRight()
{
	ASSERT (m_right);

	m_right->m_parent = m_parent;

	if (m_parent) {
		/* My right son takes my place */
		if (isLeftSon()) m_parent->m_left = m_right;
		else m_parent->m_right = m_right;
	} else {
		/* I was root */
		treeRoot() = m_right;
	}

	/* Son becomes parent */
	m_parent = m_right;
	m_right = m_parent->m_left;
	if (m_right) m_right->m_parent = this;
	m_parent->m_left = this;
}
/*----------------------------------------------------------------------------*/
template <typename T>
bool BinaryNode<T>::operator == ( const BinaryNode<T>& other ) const
{
	return m_data == other.m_data;
}
/*----------------------------------------------------------------------------*/
template <typename T>
bool BinaryNode<T>::operator < ( const BinaryNode<T>& other ) const
{
	return m_data < other.m_data;
}
