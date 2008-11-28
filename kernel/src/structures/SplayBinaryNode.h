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

#include "BinaryNode.h"

template <typename T>
class SplayBinaryNode: public BinaryNode<T>
{
public:
	SplayBinaryNode( const T& data, Tree< SplayBinaryNode<T> >* tree = NULL )
		: BinaryNode<T>( data, (Tree< BinaryNode<T> >*) tree ){};

	SplayBinaryNode<T>* subtreeFindNode( const SplayBinaryNode<T>& other);

protected:
	void splay();

	friend class Tree< SplayBinaryNode<T> >;

};

/*****************************
 * DEFINITIONS
 *****************************/

/*----------------------------------------------------------------------------*/
template <typename T>
SplayBinaryNode<T>* SplayBinaryNode<T>::subtreeFindNode( const SplayBinaryNode<T>& other)
{
	SplayBinaryNode<T>* found = (SplayBinaryNode<T>*) BinaryNode<T>::subtreeFindNode( other );
	
	if (found)
		found->splay();
	
	return found;
}
/*----------------------------------------------------------------------------*/
template <typename T>
void SplayBinaryNode<T>::splay()
{
	while (this->m_parent) {
		if (this->isLeftSon()) ( (SplayBinaryNode<T>*)(this->m_parent) )->rotateLeft();
		else ( (SplayBinaryNode<T>*)(this->m_parent) )->rotateRight();
	}
}
