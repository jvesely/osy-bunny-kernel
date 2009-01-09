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

#include "Object.h"
#include "api.h"

class InnerString: public Object
{
public:
	inline InnerString( const char* text );
	inline ~InnerString();
	inline const char* data() const { return m_text; }
	//InnerString& operator = ( char* text );
private:
	InnerString( const InnerString& other );
	InnerString& operator = ( InnerString& other );
	char* m_text;
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
	++count; /* count the last \0 */
	m_text = (char*)malloc( count );
	memcpy( m_text, text, count );
}
/*----------------------------------------------------------------------------*/
inline InnerString::~InnerString()
{
	free( m_text ); // NULL safe	
}
