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
 *	Hash map behaves almost like stl map, but it`s operation compleity
 *	is slightly different, because it is a hash map.
 */

#pragma once
//#include "api.h"
#include "structures/List.h"
#include "structures/Pair.h"



/** \brief default size of hash map; should be prime number*/
const int defaultHashArraySize = 1999;


/** @brief hash map
*	hash map with defined hash array size (in ctor)
*	@note for key must exist function int hash(KeyType key,int rng) hashing key to range <0,(rng-1)>
*	@note dataType must have default ctor
*	implementation note: uses array of List<Pair<key,data> >
*/
template <typename KeyType, typename DataType> class HashMap
{
public:
	/** @brief default constructor
	*
	*	@param arraySize lenghth of hash array
	*/
	inline HashMap(unsigned int arraySize = defaultHashArraySize) { init(arraySize); }

	/** @brief copy constructor
	*
	*	Copies content(values) and hash array length as well.
	*/
	inline HashMap(const HashMap<KeyType, DataType> & oldMap) { m_array = NULL; copyMap(oldMap); }

	/** @brief destructor
	*
	*	Deallocates everything.
	*/
	~HashMap() { assert(m_array != NULL);delete [] m_array; }

	/** @brief operator =
	*	@return *this
	*
	*	Copies entire map with structure.
	*/
	inline const HashMap<KeyType, DataType> & operator = (const HashMap<KeyType, DataType> & oldMap)
	{
		copyMap(oldMap);
		return *this;
	}

	/** @brief get hash array size*/
	inline unsigned int getArraySize() const {return m_arraySize;}

	/** @brief get count of stored elements
	*
	*	Computational complexity is O(array size).
	*/
	inline unsigned int size() const
	{
		unsigned int res = 0;
		for (unsigned int i = 0; i < m_arraySize; i++)
		{
			res += getList(i)->size();
		}
		return res;
	}

	/** @brief count of stored elements
	*
	*	Computational complexity is O(array size).
	*	(is a wrapper for size() function)
	*/
	inline unsigned int getSize() const { return size(); }

	/** @brief insert
	*
	*	If an element with equal key exists, nothing happens and EINVAL is returned.
	*	@return EOK if success; EINVAL if already eisted, ENOMEM if not enough memory is available
	*/
	inline int insert(const KeyType & key, const DataType & value)
	{
		if (exists(key))
		{
			return EINVAL;
		}
		//else
		return insertNew(key, value);
	}

	/** @brief insert
	*
	*	If element with equal key exists, nothing happens and EINVAL is returned
	*	(is a wrapper for insert(data.first, data.second)).
	*	@return EOK if success; EINVAL if already eisted, ENOMEM if not enough memory is available
	*/
	inline int insert(const Pair<KeyType, DataType>& data) { return insert(data.first, data.second); }

	/** @brief clear values
	*
	*	Array size is not changed and hash array is not deleted.
	*/
	void clear()
	{
		for (unsigned int i = 0; i < m_arraySize; i++)
		{
			m_array[i].clear();
		}
	}

	/** @brief const reference to element
	*	@note if no element with \a key is found, program panics
	*/
	inline const DataType& at(const KeyType & key) const
	{
		if (exists(key))
		{
			return m_array[mhash(key)].find(Pair<KeyType, DataType>(key, DataType()))->second;
		}
		return at(key);
	}

	/** @brief non-const reference to element
	*	@note if no element with \a key is found, new element is created
	*/
	inline DataType & at(const KeyType&key)
	{
		if (exists(key))
		{
			return m_array[mhash(key)].find(Pair<KeyType, DataType>(key, DataType()))->second;
		}
		else
		{
			return m_array[mhash(key)].pushBack(Pair<KeyType, DataType>(key, DataType()))->second;
		}
	}

	/** @brief non-const reference to element
	*	@note if no element with \a key is found, new element is created
	*
	*	(is wraper for at(key))
	*/
	inline DataType& operator [](const KeyType & key) {return at(key);}

	/** @brief const reference to element
	*	@note if no element with \a key is found, program panics
	*
	*	(is wraper for at(key))
	*/
	inline const DataType& operator [](const KeyType & key) const { return at(key); }

	/** @brief checks existence of key */
	bool exists(const KeyType & key) const
	{
		return (getList(mhash(key))->find(Pair<KeyType, DataType>(key, DataType())).getItem() != NULL);
	}

	/** @brief get list with position pos in array
	*	@return list with position \a pos in array; if position is invalid, NULL is returned
	*/
	List < Pair< KeyType, DataType > > * getList(unsigned int pos) const
	{
		if (pos < m_arraySize)
			return m_array + pos;
		else return NULL;
	}

	/** @brief erase element by key*/
	inline void erase(const KeyType & key)
	{
		getList(mhash(key))->erase(Pair<KeyType, DataType> (key, DataType()));
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
		for (unsigned int i = 0; i < m_arraySize; i++)
		{
			sum += (double) getList(i)->size();
			sum2 += (double)((getList(i)->size()) * (getList(i)->size()));
		}
		if (m_arraySize > 1)
		{
			mean = sum / m_arraySize;
			res = (sum2 - sum * sum / (m_arraySize)) / (m_arraySize - 1);      //sample variance
			res = (res * m_arraySize) / (sum * sum);
		}
		else
			res = 0.0;
		return res;
	}

	/** @brief average length of lists in array	*/
	double getAverageCount()
	{
		double sum = 0;
		for (unsigned int i = 0; i < m_arraySize; i++)
		{
			sum += getList(i)->size();
		}
		if (m_arraySize > 0)
		{
			return sum / m_arraySize;
		}
		else
		{
			return 0;
		}
	}

protected:

	/** @brief creates array of List< Pair<KeyType,DataType> >
	*
	*	This opertion may fail on no memory and nothing will be signalled.
	*	@param size array size; if is 0 or less, array with size 1 is created
	*/
	void init(unsigned int size)
	{
		if (!size) size = 1;
		m_array = new List< Pair<KeyType, DataType> > [size];
		m_arraySize = (m_array) ? size : 0;
	}


	/** @brief inserts new item
	*
	*	Expects that key does not exist in map, no check for existence is performed.
	*	@return EOK if success or ENOMEM on memory size failure
	*/
	inline int insertNew(const KeyType & key, const DataType & data)
	{
		assert(m_arraySize != 0);
		assert(m_array != NULL);
		//creation and controll of correst insertion
		if (m_array[mhash(key)].pushBack(Pair<KeyType, DataType> (key, data))
		        == m_array[mhash(key)].end())
			return ENOMEM;
		return EOK;
	}

	/** @brief hash function
	*
	*	Related to current hash array size.
	*	Calls hash(key, m_arraySize).
	*/
	inline unsigned int mhash(const KeyType & key) const
	{
		assert(m_arraySize > 0);
		assert(hash(key , m_arraySize) < m_arraySize);
		return hash(key , m_arraySize);
	};

	/** @brief copy entire map
	*
	*	Copies also structure (array size) and deletes old values and array.
	*/
	void copyMap(const HashMap<KeyType, DataType> & oldMap)
	{
		if (m_array != NULL) delete [] m_array;   //this is only one case, where m_array == NULL is allowed
		//can happen only in copy-ctor
		init(oldMap.getArraySize());
		for (unsigned int i = 0; i < m_arraySize; i++)
		{
			assert(oldMap.getList(i) != NULL);
			m_array[i].copyList(* (oldMap.getList(i)));
		}
	}

	/** @brief size of hash array */
	unsigned int m_arraySize;

	/** @brief hash array
	*
	*	It is an array of List< Pair<KeyType, DataType> >
	*/
	List < Pair< KeyType,  DataType > > * m_array;

private:
};




/** @brief hash funtion for int */
unsigned int hash(int key, int rng);

