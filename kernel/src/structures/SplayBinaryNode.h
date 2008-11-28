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
 * @brief Class SplayBinaryNode.
 *
 * Template class SplayBinaryNode declaration and implementation.
 */
#pragma once

#include "BinaryNode.h"

/*!
 * @class SplayBinaryNode SplayBinaryNode.h "structures/SplayBinaryNode.h"
 * @brief Node in binary splay tree.
 *
 * Template class:
 * @param T Type of the stored data.
 *
 * Most of the member functions inherited from BinaryNode are fine, only find
 * function needs to do the splaying after successful find.
 */
template <typename T>
class SplayBinaryNode: public BinaryNode<T>
{
public:
	/*! @brief Creates BinaryNode predecessor. */
	SplayBinaryNode( const T& data, Tree< SplayBinaryNode<T> >* tree = NULL )
		: BinaryNode<T>( data, (Tree< BinaryNode<T> >*) tree ){};

	/*! @brief Finds Node in the subtree.
	 * @param other Node to find.
	 * @return Pointer to the found Node on success, NULL otherwise.
	 *
	 * Same as BinaryNode searching, except that it splays the found Node
	 * to the top. This should speed up repetetive requests.
	 */
	SplayBinaryNode<T>* subtreeFindNode( const SplayBinaryNode<T>& other);

protected:
	/*! Rotates to the very top. */
	void splay();

	friend class Tree< SplayBinaryNode<T> >;
};

/*----------------------------------------------------------------------------*/
/* DEFINITIONS ---------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

template <typename T>
SplayBinaryNode<T>* SplayBinaryNode<T>::subtreeFindNode( const SplayBinaryNode<T>& other)
{
	/* find first */
	SplayBinaryNode<T>* found = (SplayBinaryNode<T>*) BinaryNode<T>::subtreeFindNode( other );
	
	/* if it's there move it up */
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
