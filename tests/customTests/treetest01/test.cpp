#include "structures/Trees.h"
#include "structures/List.h"
#include "api.h"
#include "tools.h"

class Dummy
{
public:
	Dummy( uint value = 0 ):m_value( value ) {
		printf("Dummy created with value: %u.\n", m_value);
	}
	Dummy( const Dummy& other ):m_value( other.m_value ) {
		printf("Dummy created with value: %u.\n", m_value);
	}
	~Dummy() {
		printf("Destoroyed Dummy with value %u.\n", m_value);
	}

	Dummy operator = ( const Dummy& other ) {
		printf("Assign operator: old value: %u, new value %u.\n", m_value, other.m_value);
		m_value = other.m_value;
		return *this;
	}

	bool operator == ( const Dummy& other ) const {
		printf("Comparing dummies: %u == %u, %s.\n", m_value, other.m_value, 
			(m_value == other.m_value) ? "TRUE" : "FALSE" );
		return (m_value == other.m_value);
	}

	bool operator != ( const Dummy& other ) const {
		return !(*this == other);
	}

	bool operator < ( const Dummy& other ) const {
		printf("Comparing dummies: %u < %u, %s.\n", m_value, other.m_value, 
    	(m_value < other.m_value) ? "TRUE" : "FALSE" );
		return (m_value < other.m_value);
	}
	inline uint value() const { return m_value; };
private:
	uint m_value;
};


template class Tree<SplayBinaryNode<Dummy> >;
template class List<Dummy>;

typedef List<Dummy> DummyList;
typedef Tree<SplayBinaryNode<Dummy> > DummyTree;

void run_test()
{
	DummyTree dummy_tree;
	DummyList	dummy_list;

	disable_interrupts();

	while (true) {
		ASSERT (dummy_tree.checkOK());
		switch (rand() % 3) {
			case 0:
				{
					puts( "[INSERT] STARTED.\n" );
					ASSERT (dummy_list.size() == (int)dummy_tree.count());
					Dummy element = rand();
					printf( "[INSERT] Inserting random element into the tree %u exists: ", element.value() );
					if (!dummy_tree.contains( element )) {
						puts("NO.\n");
						puts( "[INSERT] inserting into the tree.\n" );
						dummy_tree.insert( element );
						puts( "[INSERT] inserting into the list.\n" );
						dummy_list.pushBack( element );
						puts( "Done.\n" );
					} else {
						puts( "YES.\n" );
					}
					ASSERT (dummy_list.size() == (int)dummy_tree.count());
					puts( "[INSERT] ENDED.\n" );
					break;
				}
			case 1:
				puts( "[SEARCH] STARTED.\n" );
				ASSERT (dummy_list.size() == (int)dummy_tree.count());
				if (!dummy_tree.count()) {
					printf( "[SEARCH] The tree is empty nothing to search for.\n" );
				} else {
					printf( "[SEARCH] Searching the tree of %u elements (%u).\n", dummy_tree.count(), dummy_list.size() );
					uint max = (rand() % 12) + 1;
					DummyList::Iterator it;
					printf( "[SEARCH] Start shuffling(%u).\n", max );
					for (uint i = 0; i < max; ++i) {
						puts( "[SEARCH] Shuffling.\n" );
						it = dummy_list.rotate();
					}
					puts( "[SEARCH] Ended shuffling.\n" );
					ASSERT( it != dummy_list.end());
					puts( "[SEARCH] Tree search.\n" );
					SplayBinaryNode<Dummy> * node = dummy_tree.findItem( *it );
					ASSERT (node);
					printf( "[SEARCH] Searching for random element: %u Node: %p, value: %u.\n",
						it->value(), (void*)node, node->data().value()  );
					ASSERT (it->value() == node->data().value());
				}
				ASSERT (dummy_list.size() == (int)dummy_tree.count());
				puts( "[SEARCH] ENDED.\n" );
				break;
			case 2:
				puts( "[DELETE] STARTED.\n" );
				if (!dummy_tree.count()) {
        	printf( "[DELETE] The tree is empty nothing to search for.\n" );
				} else {
					printf( "[DELETE] Deleting element...tree: %u, list: %u\n", dummy_tree.count(), dummy_list.size() );
					ASSERT (dummy_list.size() == (int)dummy_tree.count());
					uint max = (rand() % 12) + 1;
          DummyList::Iterator it;
					printf( "[DELETE] Start shuffling(%u).\n", max );
          for (uint i = 0; i < max; ++i) {
						puts( "[DELETE] Shuffling.\n" );
            it = dummy_list.rotate();
          }

					ASSERT( it != dummy_list.end());

					puts( "[DELETE] Ended shuffling.\n" );
					printf("[DELETE] Deleting random element: %u %s.\n",
						it->value(), dummy_tree.remove( *it ) ? "OK" : "FAIL" );
					puts( "[DELETE] Deleting from the list.\n" );
					dummy_list.remove( it );
					printf( "[DELETE] After delete: tree: %u, list %u.\n", dummy_tree.count(), dummy_list.size() );
					ASSERT (dummy_list.size() == (int)dummy_tree.count());
				}
				puts( "[DELETE] ENDED.\n" );
		}
//		getc();
	}
}

