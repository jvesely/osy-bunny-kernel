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

#include "Node.h"

template <typename T>
class BinaryNode:public Node
{
public:

	BinaryNode( const T& data, Tree< BinaryNode<T> >* tree = NULL );

	virtual ~BinaryNode();

	const T& data() const { return m_data; };

	virtual bool subtreeInsert( BinaryNode* node );

	virtual BinaryNode* subtreeFindNode( const BinaryNode<T>& other );

	virtual BinaryNode* subtreeMinNode();

	virtual BinaryNode* subtreeMaxNode();

	void removeFromTree();

	BinaryNode<T>* next() const { return m_next; };

	BinaryNode<T>* previous() const { return m_previous; };

	void printInfix() const;

	void printPrefix() const;

	virtual bool operator == ( const BinaryNode<T>& other ) const;

	virtual bool operator < ( const BinaryNode<T>& other ) const;

protected:
	
	T m_data;

	BinaryNode<T>* m_parent;
	BinaryNode<T>* m_left;
	BinaryNode<T>* m_right;
	BinaryNode<T>* m_previous;
	BinaryNode<T>* m_next;

//	Tree<T, BinaryNode<T> >* m_myTree;

	void rotateLeft();
	void rotateRight();
	bool isLeftSon() const;

	BinaryNode( const BinaryNode& other );
	BinaryNode& operator = ( const BinaryNode& other );

	friend class Tree< BinaryNode<T> >;
};

/******************************
 * DEFINITIONS
 ******************************/

template <typename T>
BinaryNode<T>::BinaryNode( const T& data, Tree< BinaryNode<T> >* tree )
	: Node( (Tree< Node >*)tree ), m_data( data ),
	  m_parent( NULL), m_left( NULL ), m_right( NULL ),
		m_previous( NULL ), m_next( NULL )
{
}
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

//	std::cout << "Deleting node " << m_data << std::endl;
	/* get rid of at least one son */
	while (m_left && m_right) {
		rotateLeft();
	}
//	std::cout << "After rotate " << m_data << std::endl;

	ASSERT (!m_left || !m_right);

	/* this is my only son */
	BinaryNode<T>* son = m_left ? m_left : m_right;

//	std::cout << "Children " << m_left << " " << m_right << " " << son << std::endl;

	if (son)
		son->m_parent = m_parent;

	/* take out of the sorted chain */
	if (m_previous)
		m_previous->m_next = m_next;

	if (m_next)
		m_next->m_previous = m_previous;

//	cout << "Out of chain" << endl;

	/* I am root */
	if (!m_parent) {
//		cout << "I'm root" << endl;
		treeRoot() = son; // place new root
		m_left = m_right = m_parent = m_next = m_previous =  NULL;
		return;
	}

//	cout << "replacing myself" << endl;

	if (isLeftSon()) {
		m_parent->m_left = son;
	} else {
		m_parent->m_right = son;
	}

	m_left = m_right = m_parent = m_next = m_previous = NULL;
	treeCount() -= 1;
	return;

}
/*----------------------------------------------------------------------------*/
template <typename T>
BinaryNode<T>::~BinaryNode()
{
	removeFromTree();
}
/*----------------------------------------------------------------------------*/
template <typename T>
void BinaryNode<T>::printInfix() const
{
	if (m_left) m_left->printInfix();
//	cout << " " << m_data << " ";
	if (m_right) m_right->printInfix();
}
/*----------------------------------------------------------------------------*/
template <typename T>
void BinaryNode<T>::printPrefix() const
{
//	cout << " " << m_data << "(" << (m_left?m_left->m_data:0) << "," << (m_right?m_right->m_data:0) << ")";
	if (m_left) m_left->printPrefix();
	if (m_right) m_right->printPrefix();
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
//		cout << "Rotating left with parent" << endl;	
		if (isLeftSon()) m_parent->m_left = m_left;
		else m_parent->m_right = m_left;
	} else {
		//cout << "My tree: " <<  m_myTree << endl;
		treeRoot() = m_left;
	}

	m_parent = m_left;
	m_left = m_parent->m_right;
	m_parent->m_right = this;
}
/*----------------------------------------------------------------------------*/
template <typename T>
void BinaryNode<T>::rotateRight()
{
	ASSERT (m_right);

	m_right->m_parent = m_parent;

	if (m_parent) {
//		cout << "Rotating right with parent" << endl;	
		if (isLeftSon()) m_parent->m_left = m_right;
		else m_parent->m_right = m_right;
	} else {
		//cout << "My tree: " <<  m_myTree << endl;
		treeRoot() = m_right;
	}

	m_parent = m_right;
	m_right = m_parent->m_left;
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
