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
 * @brief Contains body of the first thread.
 *
 * First "program" run by the kernel.
 */

#include "api.h"
#include "mem/FrameAllocator.h"
#include "structures/Trees.h"

extern "C" void* test1(void*);
extern "C" void* test2(void*);

//template class Tree<SplayBinaryNode<int> >;

/*
class MyType : public SplayBinaryNode<MyType*> {
public:
	MyType(int i) : SplayBinaryNode<MyType*>(static_cast<MyType*>(this)), m_value(i) {
		printf("constructing mytype with value %d\n", i);
	}

	MyType(const MyType& other) : SplayBinaryNode<MyType*>(static_cast<MyType*>(this)) {
		printf("copy-constructing mytype with value %d\n", other.m_value);
		m_value = other.m_value;
	}

	MyType& operator=(const MyType& other) {
		printf("assigning mytype with value %d\n", other.m_value);
		m_value = other.m_value;

		return *this;
	}

	virtual ~MyType() {
		printf("destructing mytype with value %d\n", m_value);
	}

	bool operator== (const MyType& other) const {
		return m_value == other.m_value;
	}

	bool operator< (const MyType& other) const {
		return m_value < other.m_value;
	}

	void print() const {
		printf("MyNode value is: %d\n", m_value);
	}

private:
	int m_value;

};
*/

void* test(void*)
{
/*
	Tree< MyType > tree;

	tree.insert(new MyType(5));
  tree.insert(new MyType(7));
  tree.insert(new MyType(3));
  tree.insert(new MyType(4));
  tree.insert(new MyType(8));
  tree.insert(new MyType(6));

	printf( "=======================================\n");
	printf( "Inserting done and count is: %u\n", tree.count());
	printf( "=======================================\n");

	//SplayBinaryNode<MyType*>* node = tree.findItem(MyType(4));
	//node->print();

	//tree.remove(MyType(5));
	//delete node;

	printf( "=======================================\n");
	printf( "After remove and delete count %u\n", tree.count());
	printf( "=======================================\n");
*/
	#ifdef KERNEL_TEST
		run_test();
		return NULL;
	#endif
//	dprintf("Pausing execution");
//	Processor::msim_stop();

	const int count = 1024;

	int * array = (int*)0x40000;

	int res = vma_alloc((void**)&array, count, (VF_AT_KUSEG << VF_AT_SHIFT) | VF_VA_AUTO);
	printf("Allocation %s at address: %p.\n", (res == 0) ? "SUCCESSFULL" : "FAILED", array);
	*array = 0xf00dbad;
	*(array + 1022) = 0xbadf00d;
	Processor::msim_stop();

	vma_free(array);
	Processor::msim_stop();


	return NULL;
	thread_t thread1;
//	thread_t thread2;
//	thread_t thread3;
	thread_create(&thread1, test1, NULL, 0);
//	thread_create(&thread2, test1, NULL, 0);
//	thread_create(&thread3, test1, NULL, 0);
	thread_detach(thread_get_current());
	thread_join_timeout( thread1, 100 );
	printf("After join timeout.\n");
	thread_kill( thread1 );
	printf("After kill.\n");
	thread_join( thread1 );

	// fralloc tests

	//printf("Frame allocator testing...\n");
		
	// DO NOT USE THIS, WHEN THE NEW MALLOC IS IN USE!!!
	//MyFrameAllocator::instance().test();


	return NULL;
	thread_yield();
	while (true) {
		thread_wakeup( thread1 );
		thread_sleep(1);
	}

	return NULL;
// */
	while (true) {
/*		char c = getc();
		printf("Hello: %c\n",c);
		continue;
			printf("Sleeping. %x %d %u\n", 0x4916b35f, 0x4916b35f, 0x4916b35f);
			thread_sleep(8);
			printf("done: %x\n",Time::getCurrent().secs());
			continue; // */
/*
		thread_t thread;

//		printf("My thread num is %d\n", thread_get_current());
//		return NULL;
		if (thread_create(&thread, test1, NULL, 0) == EOK) {
			thread_detach(thread);

//			printf("Before kill\n");
//			thread_kill(thread);
//			printf("After kill.\n");
			int res = thread_join(thread);
			printf("Thread join: %d\n", res);
//			assert(res == EINVAL);
			thread_yield();
			printf("Thread again.\n");
//			thread_sleep(10);
//			return NULL;
		} else {
			printf("Thread creation failed.\n");
			assert(false);
			break;
		}// */
	}
	printf("Bailing out.\n");
	return NULL;
}

void* test1(void*)
{
	thread_sleep(1);

	//while (true){
		printf("Test...%d\n", thread_get_current());
//		thread_sleep(1);
//		thread_usleep(1000000);
//		thread_suspend();
//	}
	return NULL;
}

