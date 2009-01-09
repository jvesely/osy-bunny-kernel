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

#include "String.h"

String::String( const char* text ):
	Pointer<InnerString>(new InnerString( text )), END( 0 )
{
	ASSERT (data());
}
/*----------------------------------------------------------------------------*/
String::String( const String& other ):
	Pointer<InnerString>(const_cast<InnerString*>(other.data()) ), END( 0 )
{
	ASSERT (data() == other.data());
};
/*----------------------------------------------------------------------------*/
String& String::operator = ( const char* text )
{
	InnerString* inner = new InnerString( text );
	static_cast<Pointer<InnerString>*>(this)->operator = (inner);
	return *this;
}
/*----------------------------------------------------------------------------*/
String& String::operator = ( const String& other )
{
	static_cast<Pointer<InnerString>*>(this)->operator = (
		const_cast<InnerString*>( other.data() ) );
	return *this;
}
/*----------------------------------------------------------------------------*/
bool String::operator == ( const String& other )
{
	if (cstr() == other.cstr()) return true; /* same text */
	const char* me  = cstr();
	const char* him = other.cstr();
	if (!me || !him) return false; /* one of them is empty */
	while (*me && *him){
		if (*me != *him) break;
		++me; ++him;
	}
	return *me == *him;
}
/*----------------------------------------------------------------------------*/
bool String::empty() const
{
		return (!data() || ! cstr());
}
/*----------------------------------------------------------------------------*/
const char* String::cstr() const
{
	return (data() ? data()->data() : NULL);
}
/*----------------------------------------------------------------------------*/
