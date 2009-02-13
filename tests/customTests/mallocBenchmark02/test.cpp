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

#include "librt.h"
#include "../include/defs.h"
#include "../librt/src/UserMemoryAllocator.h"
#include "structures/SimpleList.h"
#include "structures/HashMap.h"
#include "Time.h"
#include "../librt/src/Mutex.h"
#include "../librt/src/YieldingSpinLock.h"
#include "synchronization/Spinlock.h"



static const char * desc =
	"Multithread test for testing performance of memory allocator.\n"
	"Test will try to allocate at least NEEDED_MEMORY of random "
	"char * strings of maximum length MAX_LENGTH  and store them into "
	"hash table. \n"
	"For each 100 malloc operations there will be performed FREE_PERCENTAGE "
	"free operations, what means a random item from hash table will be chosen, removed "
	"and string related to it will be deleted.\n"
	"When NEEDED_MEMORY is allocated, allocator information will be written: \n"
	"free memory, allocated memory in hash table and total memory used by memory allocator.\n"
	"Test will run for this THREAD_COUNT threads, which will allocate into the same hashMap.\n\n";


//how much memory should allocate test
const size_t NEEDED_MEMORY = 1*1024*1024;
//max length of random string
const unsigned int MAX_LENGTH = 2048;
/*percentage of free operations against malloc operations in allocation phase
*	50 means, that on two malloc operations one free operation will be performed
*	100 and more means, that allocation will not continue :)
*/
const unsigned int FREE_PERCENTAGE = 75;

/*	this is multithread test, and therefore it should be possible to
*	set thread count. This value sets thread count in test.
*/
const unsigned int THREAD_COUNT = 10;

unsigned int count;
int counter;
size_t allocated;
//YieldingSpinLock allocatedCountTableLock;
//Spinlock allocatedCountTableLock;
Mutex allocatedCountTableLock;
Spinlock counterLock;

struct strSize
{
	char * s;
	size_t size;
};



/**
 * rand
 *
 * A trivial random number generator.
 * Returns a random unsigned long number.
 */
inline static unsigned long
tst_rand ()
{
  static unsigned long random_seed = 12345678;
  unsigned int foo = Time::getCurrent().toUsecs();

  random_seed = (random_seed * foo) % 22348977 + 7;
  return random_seed >> 8;
} /* rand */



//create random string of random length
char * createRandomString(size_t & resSize)
{
	unsigned int length = 1 + tst_rand() %MAX_LENGTH;
	unsigned char * result = (unsigned char * ) malloc( sizeof(char) * (length + 1));
	result[length] = 0;
	resSize = (size_t)length + 1;
	/*for(unsigned int i = 0; i< length;i++)
	{
		result[i] = 10;
	}*/
	return (char*)result;
}


void addRandomItem(HashMap<int,strSize> * myMap)
{
	size_t resSize;
	char * s = createRandomString(resSize);
	strSize str;
	str.s = s;
	str.size = resSize;
	allocatedCountTableLock.lock();
	//printf("adding item\n");
	counterLock.lock();
	myMap->insert(counter,str);
	counter++;
	counterLock.unlock();
	allocated += sizeof(Pair<int,strSize>) + resSize;
	count++;
	//printf("added item\n");
	allocatedCountTableLock.unlock();
}

void removeRandomItem(HashMap<int,strSize> * myMap)
{
	allocatedCountTableLock.lock();
	//printf("removinf item\n");
	unsigned int hsh = (unsigned int)tst_rand() % myMap->getArraySize();
	//this can be unsafe operation ...
	while(myMap->getList(hsh)->empty()){
		hsh = (hsh+1)%(myMap->getArraySize());
	}
	int key = myMap->getList(hsh)->begin()->first;

	size_t size = (*myMap)[key].size;
	free((*myMap)[key].s);

	allocated -= size + sizeof(Pair<int,strSize>);
	count--;

	myMap->erase(key);
	//printf("removed item\n");
	allocatedCountTableLock.unlock();
}

void * oneThreadWork (void * param)
{
	printf("getting hashmap at %x\n",param);
	HashMap<int,strSize> * myMap = (HashMap<int,strSize>*)param;
	printf("locking\n");

	allocatedCountTableLock.init();
	//counterLock.init();

	allocatedCountTableLock.lock();
	printf("lock before cycle\n");
	while(allocated < NEEDED_MEMORY)
	{
		printf("in cycle\n");
		allocatedCountTableLock.unlock();
		printf("allocating\n");
		for(unsigned int i =0;i<100;i++){
			addRandomItem(myMap);
		}
		printf("freeing\n");
		for(unsigned int i = 0; i<FREE_PERCENTAGE; i++){
			removeRandomItem(myMap);
		}
		printf("done\n");
		allocatedCountTableLock.lock();
		printf("results: \nallocated: %d \nfree:      %d \ntotalUsed: %d \n",allocated,mallocatorGetFreeSize(),mallocatorGetTotalSize());
	}
	allocatedCountTableLock.unlock();

	return NULL;
}



void
main (void)
{
	printf(desc);

	mallocStrategyDefault();
	//mallocStrategyFirstFit();
	//mallocStrategyNextFit();
	//mallocStrategyBestFit();
	//mallocStrategyWorstFit();

	count = 0;
	counter = 0;
	allocated  = 0;
	void * resultStore;

	//used has table
	unsigned int tableSize = 1999;
	HashMap<int,strSize> myMap(tableSize);
	allocated = tableSize * (sizeof(List<Pair<int, strSize> >));

	printf("hashmap at %x \n",&myMap);
	thread_t * threads = new thread_t[THREAD_COUNT];
	for(unsigned int i =0; i<THREAD_COUNT; i++)
	{
		thread_create(threads+i, oneThreadWork, &myMap );
	}
	printf("all threads are running\n");
	for(unsigned int i =0; i<THREAD_COUNT; i++){
		thread_join( *(threads+i), &resultStore );
	}
	printf("all threads have stopped\n");

	size_t overhead = mallocatorGetTotalSize() - mallocatorGetFreeSize() - allocated;
	printf("results: \nallocated: %d \nfree:      %d \ntotalUsed: %d \noverhead:  %d \n",allocated,mallocatorGetFreeSize(),mallocatorGetTotalSize(),overhead);

	SimpleList * freeList = UserMemoryAllocator::instance().getFreeList();
	SimpleList * usedList = UserMemoryAllocator::instance().getUsedList();
	SimpleList * chunkList = UserMemoryAllocator::instance().getChunkList();

	SimpleListItem * foo = freeList->getMainItem()->next;
	unsigned int freeCount = 0;
	unsigned int smallCount = 0;
	size_t smallSize = 160;
	while(foo!= freeList->getMainItem()){
		if(((BasicMemoryAllocator::BlockHeader*)(foo))->size() < smallSize)
			smallCount++;
		foo = foo->next;
		freeCount++;
	}
	printf("total count of free blocks is %d, count of smaller than %d bytes is %d \n",freeCount,smallSize,smallCount);

	foo = usedList->getMainItem()->next;
	unsigned int usedCount = 0;
	while(foo!= usedList->getMainItem()){
		foo = foo->next;
		usedCount++;
	}
	printf("total count of used blocks is %d \n",usedCount);


	foo = chunkList->getMainItem()->next;
	unsigned int chunkCount = 0;
	while(foo!= chunkList->getMainItem()){
		foo = foo->next;
		chunkCount++;
	}
	printf("total count of memory chunks is %d \n",chunkCount);

}






