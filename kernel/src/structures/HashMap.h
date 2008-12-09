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
 * @brief hash map implementation
 *
 * Contains: class Pair and class HashMap (both templated)
 */

#pragma once
#include <api.h>
#include <structures/List.h>
#include <structures/Pair.h>



/** default size of hash map; should be prime number*/
const int defaultHashArraySize = 1999;

/** error: not inserted (element already exists) - not used (EINVAL used instead)*/
const int eAlreadyExists = -2;

/** internal error enomem - not used right now (ENOMEM is now sufficient)*/
const int eNoMemory = -1;


/** @brief hash map
*	hash map with defined hash array size (in ctor)
*	@note for key must exist function int hash(KeyType key,int rng) hashing key to range <0,(rng-1)>
*	@note dataType must have default ctor
*	implementation note: uses array of List<Pair<key,data> >
*/
template <typename KeyType, typename DataType> class HashMap
{
public:
	/** @brief default ctor */
	inline HashMap ( unsigned int arraySize = defaultHashArraySize ) { init ( arraySize ); }

	/** @brief copy ctor */
	inline HashMap ( const HashMap<KeyType, DataType> & oldMap ) { m_array = NULL; copyMap ( oldMap ); }

	/** @brief dtor */
	~HashMap() { assert ( m_array != NULL );delete [] m_array; }

	/** @brief operator =
	*	@return *this
	*	copies entire map with structure
	*/
	inline const HashMap<KeyType, DataType> & operator = ( const HashMap<KeyType, DataType> & oldMap )
	{
		copyMap ( oldMap );
		return *this;
	}

	/** @brief hash array size*/
	inline unsigned int getArraySize() const {return m_arraySize;}

	/** @brief count of stored elements
	*	computational complexity is O(array size)
	*/
	inline unsigned int size() const
	{
		unsigned int res = 0;
		for ( unsigned int i = 0; i < m_arraySize; i++ ) {
			res += getList ( i )->size();
		}
		return res;
	}

	/** @brief count of stored elements
	*	computational complexity is O(array size)
	*/
	inline unsigned int getSize() const { return size(); }

	/** @brief insert
	*	if element with equal key exists, nothing happens and eAlreadyExists is returned
	*	@note returns int, which is not unsigned int, remember it for methods such as getList
	*	@return EOK if success; EINVAL if already eisted, ENOMEM if not enough memory is available
	*/
	inline int insert ( const KeyType & key, const DataType & value )
	{
		if ( exists ( key ) ) {
			return EINVAL;
		}
		//else
		return insertNew ( key, value );
	}

	/** @brief insert
	*	if element with equal key exists, nothing happens and eAlreadyExists is returned
	*	implementation note: calls insert(key,data)
	*	@note returns int, which is not unsigned int, remember it for methods such as getList
	*	@return hash(key) if success; -1( = eAlreadyExists) else
	*/
	inline int insert ( const Pair<KeyType, DataType>& data ) { return insert ( data.first, data.second ); }

	/** @brief clear values
	*	array size is not changed
	*/
	void clear() {
		for ( unsigned int i = 0; i < m_arraySize; i++ ) {
			m_array[i].clear();
		}
	}

	/** @brief const reference to element
	*	@note if no element with \a key is found, program fails
	*/
	inline const DataType& at ( const KeyType & key ) const
	{
		if ( exists ( key ) ) {
			return m_array[ mhash ( key ) ].find ( key )->second;
		}
		return at ( key );
	}

	/** @brief non-const reference to element
	*	@note if no element with \a key is found, new element is created
	*/
	inline DataType & at ( const KeyType & key )
	{
		if ( exists ( key ) ) {
			return m_array[ mhash ( key ) ].find ( Pair<KeyType, DataType> ( key, DataType() ) )->second;
		} else {
			return m_array[ mhash ( key ) ].pushBack ( Pair<KeyType, DataType> ( key, DataType() ) )->second;
		}
	}

	/** @brief non-const reference to element
	*	@note if no element with \a key is found, new element is created
	*/
	inline DataType& operator [] ( const KeyType & key ) { return at ( key ); }

	/** @brief const reference to element
	*	@note if no element with \a key is found, program fails
	*/
	inline const DataType& operator [] ( const KeyType & key ) const { return at ( key ); }

	/** @brief find
	*	implementation note: uses getItem() method of List::Iterator, return value is compared to NULL
	*/
	bool exists ( const KeyType & key ) const
	{
		return ( getList ( mhash ( key ) )->find( Pair<KeyType, DataType>( key, DataType() ) ).getItem() != NULL );
	}

	/** @brief get list with position pos in array
	*	@return list with position \a pos in array; if position is invalid, NULL is returned
	*/
	List < Pair< KeyType, DataType > > * getList ( unsigned int pos ) const
	{
		if ( pos < m_arraySize )
			return m_array + pos;
		else return NULL;
	}

	/** @brief erase	*/
	inline void erase ( const KeyType & key )
	{
		getList ( mhash ( key ) )->erase ( Pair<KeyType, DataType> ( key, DataType() ) );
	}

	/** @brief statistical function
	*	@return unitar variance of length of List objects in array. 0 is most homogenous, 1 least (all elements in one List)
	*/
	double getUnitarVariance()
	{
		double mean = 0;
		double res = 0;
		double sum2 = 0;
		double sum = 0 ;
		for ( unsigned int i = 0; i < m_arraySize; i++ ) {
			sum += ( double ) getList ( i )->size();
			sum2 += ( double ) ( ( getList ( i )->size() ) * ( getList ( i )->size() ) );
		}
		if ( m_arraySize > 1 ) {
			mean = sum / m_arraySize;
			res = ( sum2 - sum * sum / ( m_arraySize ) ) / ( m_arraySize - 1 );//sample variance
			res = ( res * m_arraySize ) / ( sum * sum );
		}else
			res = 0.0;
		return res;
	}

	/** @brief average length of lists in array	*/
	double getAverageCount()
	{
		double sum = 0;
		for ( unsigned int i = 0; i < m_arraySize; i++ ) {
			sum += getList ( i )->size();
		}
		if ( m_arraySize > 0 ) {
			return sum / m_arraySize;
		}else{
			return 0;
		}
	}

protected:

	/** @brief creates array of List< Pair<KeyType,DataType> >
	*	this opertion may fall on no memory and nothing will be signalled
	*	@param size array size; if is 0 or less, array with size 1 is created
	*/
	void init ( unsigned int size ) {
		m_arraySize = size;
		if ( m_arraySize > 0 )
			m_array = new List < Pair< KeyType,  DataType > > [m_arraySize];
		else {
			init ( 1 );//so that array allways exists
		}
	}


	/** @brief inserts new item
	*	expects that key does not exist in map, no check for existence is performed
	*	@return EOK if success or ENOMEM on memory size failure
	*/
	inline int insertNew ( const KeyType & key, const DataType & data ) {
		assert ( m_arraySize != 0 );
		assert ( m_array != NULL );
		//creation and controll of correst insertion
		if (m_array[mhash ( key ) ].pushBack ( Pair<KeyType, DataType> ( key, data ) )
				== m_array[mhash ( key ) ].end())
			return ENOMEM;
		return EOK;//(int) mhash ( key );
	}

	/** @brief hash function
	*	related to current hash array size
	*	calls hash(key, m_arraySize)
	*/
	inline unsigned int mhash ( const KeyType & key ) const
	{
		assert ( m_arraySize > 0 );
		assert ( hash ( key , m_arraySize ) < m_arraySize );
		return hash ( key , m_arraySize );
	};

	/** @brief copy entire map
	*	copies also structure (array size) and deletes old values and array
	*/
	void copyMap ( const HashMap<KeyType, DataType> & oldMap )
	{
		if ( m_array != NULL ) delete [] m_array; //this is only one case, where m_array == NULL is allowed
			//can happen only in copy-ctor
		init ( oldMap.getArraySize() );
		for ( unsigned int i = 0; i < m_arraySize; i++ ) {
			assert ( oldMap.getList ( i ) != NULL );
			m_array[i].copyList ( * ( oldMap.getList ( i ) ) );
		}
	}

	/** @brief size of has array */
	unsigned int m_arraySize;

	/** @brief hash array
	*	array of List< Pair<KeyType, DataType> >
	*/
	List < Pair< KeyType,  DataType > > * m_array;

private:
};




/** @brief hash funtion for int */
unsigned int hash ( int key, int rng );

