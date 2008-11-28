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
 * @brief Various tree types.
 *
 * Template typedefs overrides for trees.
 */
#pragma once

#include "Tree.h"
#include "BinaryNode.h"
#include "SplayBinaryNode.h"

/*! 
 * struct Trees Trees.h "structures/Trees.h"
 * @brief Typedefs for different types of trees.
 */
template<typename T>
struct Trees
{
	/*! See Tree and BinaryNode documentation for details. */
	typedef Tree< BinaryNode<T> > BinaryTree;
	/*! See Tree and SplayBinaryNode documentation for details. */
	typedef Tree< SplayBinaryNode<T> > SplayTree;
};

