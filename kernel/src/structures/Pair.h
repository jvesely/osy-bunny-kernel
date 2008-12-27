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

/** @brief pair of key and data, used in Map or HashMap
*	requires copy ctor of both types and == operator of KeyType
*	is only basic encapsulation of key and data into one entity
**/
template <typename KeyType, typename DataType> class Pair
{
public:
	/**	@brief ctor with values	*/
	inline Pair ( const KeyType & key , const DataType & value ) : first ( key ), second ( value ) {};

	/** @brief operator ==
	*	compares only keys
	*/
	inline bool operator == ( const Pair<KeyType, DataType> & item ) const {return first == item.first;}

	/** @brief operator !=
	*	compares only keys
	*/
	inline bool operator != ( const Pair<KeyType, DataType> & item ) const {return !operator== ( item );}

//these are public also in STL pair, so they are left to public
	/** @brief key value */
	KeyType first;

	/** @brief data */
	DataType second;
};



