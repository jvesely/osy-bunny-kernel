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
 * @brief Reference counted pointer.
 *
 */

#pragma once

#include "Object.h"

/*! @class Pointer Pointer.h "Pointer.h"
 * 	@brief Reference counted pointer.
 *
 * 	Class replaces standard pointer in case reference counting is needed.
 */
template <class T>
class Pointer
{
public:
	Pointer():m_obj( NULL ) {}
	
	/*! @brief Creates Pointer gurading instance object. */
	Pointer( T * object ):m_obj( object )
		{ if (m_obj)  m_obj->incCount(); }
	
	Pointer( const Pointer<T>& other )
		{ if ((m_obj = other.m_obj)) m_obj->incCount(); }

	/*! @brief Correctly destroys pointer. */
	virtual ~Pointer() { releaseObj(); }

	inline const T* data() const { return m_obj; }
		
	inline T* data() { return m_obj; }

	/*! @brief Assigns instance to guard. */
	inline Pointer<T>& operator = ( T* other_obj )
	{
		if (m_obj != other_obj) {
			releaseObj();
			if ((m_obj = other_obj)) m_obj->incCount(); 
		}
		return *this;
	}

	/*! @brief Standard opertor, keeps count */
	inline Pointer<T>& operator = ( const Pointer<T>& other )
	{ 
		if (m_obj != other.m_obj) {
			releaseObj();
			if ((m_obj = other.m_obj)) m_obj->incCount();
		}
		return *this;
	}

	/*! @brief Standard operator -> . */
	inline T* operator-> ()	             { return m_obj; }

	/*! @brief Standard const operator -> . */
	inline const T* operator-> () const  { return m_obj; }
	
	/*! @brief Standard operator * . */
	inline T& operator* ()               { return *m_obj; }

	/*! @brief Standard const operator * . */
	inline const T& operator* () const   { return *m_obj; }

	/*! @brief Conversion operator (Checks for NULL). */
	inline operator bool () const        { return m_obj != NULL; }

	/*! @brief Checks for NULL. */
	inline bool operator ! () const      { return m_obj == NULL; }

	/*! @brief Checks for storing the same instances of T. */
	inline bool operator == ( Pointer<T>& other ) const
		{ return m_obj == other.m_obj; }
	
	/*! @brief Checks for storing the different instances of T. */
	inline bool operator != ( Pointer<T>& other ) const
		{ return !(*this ==  other); }
	

private:
	/* @brief Decresases count on stored object instance () if any. */
	inline void releaseObj()
		{ if (m_obj) m_obj->decCount(); };

	/*! @brief Guraded object instance. */
	T* m_obj;
};
