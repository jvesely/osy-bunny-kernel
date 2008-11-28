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
 * @brief Class Node.
 *
 * Class Node decalaration and implementation.
 */

#pragma once

#include "Tree.h"

/*!
 * @class Node Node.h "structures/Node.h"
 * @brief Predecessor for all Nodes.
 *
 * Iplements functions that needs to access private members of the Tree.
 */

class Node
{

public:
	/*! @brief Makes me think I'm part of the Tree "tree".  */
	Node( Tree< Node > * tree): m_myTree( tree ){};

	virtual ~Node(){};

protected:
	Node( const Node& other );
	Node operator = ( const Node& other );

	/*!
	 * @brief Gets reference to the root of the current Tree.
	 *
	 * @return Reference to the tree's root.
	 * Node must be inserted in the tree for this function to work correctly.
	 */
	Node*& treeRoot()
		{ ASSERT (m_myTree); return m_myTree->m_root; };

	/*!
	 * @brief Gets reference to the tree's Node count.
	 * 
	 * @return Reference to the count.
	 * Node must be inserted in the tree for this function to work correctly.
	 */
	uint& treeCount()
		{ ASSERT (m_myTree); return m_myTree->m_count; };

	/*! Here I belong */
	Tree <Node>* m_myTree;
};
