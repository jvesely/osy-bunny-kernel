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

#include "structures/HashMap.h"

/*!
 * @class IdMap IdMap.h "structures/IdMap.h"
 * @brief Basically a hashmap with ability to generate uniqe keys.
 *
 * Wrapper class for standard HashMap with added ability to generate keys.
 * Maintains mappings from ID to T, cleans all mappings upon destruction.
 */
template <typename ID, typename T>
class IdMap
{
public:
	/*!
	 * @brief Standard onstructor, just prepares hashmap.
	 * @param count number of bags used by hasmap, default is 61
	 */
	IdMap(int count = 61);

	/*!
	 * @brief Generates unused ID and creates mapping to T.
	 * @param element ID would map to this instance.
	 * @return new generated ID != BAD_ID when succeeded, BAD_ID otherwise
	 */
	ID getFreeId( const T& element );

	/*!
	 * @brief Destroys mapping assigned to given ID, marking it as free.
	 * @param id ID to free.
	 */
	inline void returnId( const ID& id );

	/*!
	 * @brief Traslates given ID into type T using existing mapping.
	 * @param id ID to translate.
	 * @return Associated T on success, default constructed T otherwise.
	 */
	inline T translateId( const ID& id );

	/*!
	 * @brief Used HashMap getter, provided for convenience.
	 * @return Used HashMap const reference.
	 */
	inline const HashMap<ID, T>& map() const;

	/*!
	 * @brief Standard destructor, clears hashmap.
	 */
	~IdMap();

	/*! @brief Error indicator, returned by some memeber functions on error. */
	const ID BAD_ID;

private:
	/*! @brief Last used ID, used during next ID generation. */
	ID m_lastId;

	/*! @brief Hasmap storing existing associations. */
	HashMap<ID, T> m_map;

	IdMap( const IdMap& );
	IdMap& operator = ( const IdMap& );
};

/*----------------------------------------------------------------------------*/
/* DEFINITIONS ---------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/
template <typename ID, typename T>
IdMap<ID, T>::IdMap( int count ): BAD_ID( 0 ), m_lastId( 0 ), m_map( count )
{
	const int result = m_map.insert( BAD_ID, T() );
	ASSERT (result == EOK);
}
/*----------------------------------------------------------------------------*/
template <typename ID, typename T>
ID IdMap<ID, T>::getFreeId( const T& element )
{
	ID new_id = ++m_lastId;

  /* there has to be free id becasue if all were occupied, than
   * every byte of adressable memory would byb occupied by element instance
   * but the next variable and the parameter take up 8 bytes 
   * (and even more is used by the rest of the kernel), 
   * thus there must be a free id
   */
	int result;
	while ((new_id == BAD_ID) 
			|| (result = m_map.insert( new_id, element ) == EINVAL) 
			) 
	{
		new_id = ++m_lastId;
	}

	if (result == ENOMEM)
		return BAD_ID;

	return new_id;
}
/*----------------------------------------------------------------------------*/
template <typename ID, typename T>
void IdMap<ID, T>::returnId( const ID& id )
{
	m_map.erase( id );
}
/*----------------------------------------------------------------------------*/
template <typename ID, typename T>
T IdMap<ID, T>::translateId( const ID& id)
{
	return m_map.exists( id ) ? m_map.at( id ) : T();
}
/*----------------------------------------------------------------------------*/
template <typename ID, typename T>
const HashMap<ID, T>& IdMap<ID, T>::map() const
{
	return m_map;
}
/*----------------------------------------------------------------------------*/
template <typename ID, typename T>
IdMap<ID, T>::~IdMap()
{
	m_map.clear();
}
