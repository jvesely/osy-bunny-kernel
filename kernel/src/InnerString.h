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
 * @brief InnerString class decalration adn implementation.
 *
 * Long description. I would paste some Loren Ipsum rubbish here, but I'm afraid
 * It would stay that way. Not that this comment is by any means ingenious but 
 * at least people can understand it. 
 */

#pragma once

#include "Object.h"
#include "api.h"

/*!
 * @class InnerString InnerString.h "InnerString.h"
 * @brief Class storing string data.
 *
 * Together with the String class, InnerString provides refernce coutned
 * storage for String data. Inner String may onyl be constructed and destructed.
 */
class InnerString: public Object
{
public:
	/*!
	 * @brief Creates new InnerString using provided data.
	 * @param text Data to store.
	 *
	 * @note Data are copied during initialization.
	 */
	inline InnerString( const char* text );

	/*! @brief Correctly disposes all resources used by InnerString. */
	inline ~InnerString();

	/*! @brief Grants access directly to the stoerd data. */
	inline const char*  data() const { return m_text; }

	/*! @brief Gets size of the stored data. */
	inline size_t size() const { return m_size; }

private:
	InnerString( const InnerString& other );
	InnerString& operator = ( InnerString& other );
	char*  m_text;  /*!< Stored data.      */
	size_t m_size;  /*!< Size of the data. */
};
/*----------------------------------------------------------------------------*/
inline InnerString::InnerString( const char* text )
{
	size_t count;
	if (!text) {
		return;
	}
	ASSERT (text);
	for ( count = 0; text[count]; ++count ) {};
	m_size = count;
	++count; /* count the last \0 */
	m_text = (char*)malloc( count );
	memcpy( m_text, text, count );
}
/*----------------------------------------------------------------------------*/
inline InnerString::~InnerString()
{
	free( m_text ); // NULL safe	
}
