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
#include "InnerString.h"
#include "Pointer.h"

template class Pointer<InnerString>;


/*! @class String String.h "String.h"
 * @brief String class provided for convient handling of text data.
 *
 * Reference counted implementation, avoids most of the unnecessary copies.
 */
class String: protected Pointer<InnerString>
{
public:
	/*!
	 * @brief Constructs String class from the array of chars.(Data are copied)
	 * @param text Data to copy to newly constructed String.
	 */
	String( const char* text = NULL);

	/*!
	 * @brief Reference coutned copy construction.
	 * @param other original to copy.
	 */
	String( const String& other );


	/*!
	 * @brief Assign operator, uses reference counting.
	 * @param other Instance providing data.
	 */
	String& operator = ( const String& other );

	/*!
	 * @brief Compares Strings alphanumerically.
	 * @param other String to compare to.
	 * @return true if data stored are identical, false otherwise.
	 */
	bool operator == ( const String& other ) const;

	/*!
	 * @brief Alphanumerical comparison of the Strings.
	 * @param other String to compare to.
	 * @return true if data stored comes first when sorted in alphanumerical order
	 * 	false otherwise.
	 *
	 * @note Empty String < any non-empty String.
	 */
	bool operator < ( const String& other ) const;

	/*!
	 * @brief Indicator of emptiness.
	 * @return true if data contains no chars, no even ending \\0, false otherwise.
	 */
	bool empty() const;

	/*!
	 * @brief Returns size of stored data.
	 * @return Size of stored data.
	 * @note Returns 0 if either no data are stored or stored data
	 * 	contains just ending \\0.
	 */
	size_t size() const;

	/*!
	 * @brief Gets pointer to stored data.
	 */
	const char* cstr() const;
};
