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
 * @brief Heap allocator implementation.
 *
 * Copied from Kalisto, reimplemented as a class.
 */
#include "mem/Allocator.h"
#include "api.h"

void Allocator::setup(const uintptr_t from, const size_t length)
{
	uintptr_t end = alignDown(from + length, ALIGMENT);
	uintptr_t start = alignUp(from, ALIGMENT);
	dprintf("Testing recieved memory chunk from %x to %x.\n", start, end);
	dprintf("Original blocak from %x to %x \n", from, from + length );

	//test first
	*(uint8_t*)(end - sizeof(MAGIC)) = MAGIC;
	assert( *(uint8_t*)(end - sizeof(MAGIC)) == MAGIC);
	
	// test ok
	//dprintf("Mem chunk seems OK, creating block.\n");
	m_start = start;
	m_end = end;
	createBlock(start, end - start, true);

}
/*----------------------------------------------------------------------------*/
void Allocator::createBlock(
	const uintptr_t start, const size_t size, const bool free
) const
{
	dprintf("Creating block from %x of size %u : %s\n", start, size, free?"free":"used");
	assert(start >= m_start);
	printf("requested %x end real end %x\n ", (start + size), m_end);
	assert( (start + size) <= m_end);
	// test aligment
	assert((start & ~(ALIGMENT - 1)) == start);
	assert((size & ~(ALIGMENT - 1)) == size );
		
	// setup
	BlockHeader* header = (BlockHeader *)start;
	BlockFooter* footer = (BlockFooter *)(start + size - sizeof(BlockFooter));
	
	header->size = size - sizeof(BlockHeader) - sizeof(BlockFooter);
	header->magic = BIG_MAGIC;
	header->free = free;

	footer->magic = BIG_MAGIC;
	footer->size = size - sizeof(BlockHeader) - sizeof(BlockFooter);
	
	assert(checkBlock(start));
}
/*----------------------------------------------------------------------------*/
bool Allocator::checkBlock(const uintptr_t start) const
{
	BlockHeader* header = (BlockHeader*)start;
	BlockFooter* footer = (BlockFooter*)(start + sizeof(BlockHeader) + header->size);
	bool ok =  (header->magic == BIG_MAGIC) && (footer->magic == BIG_MAGIC)
	        && (header->size == footer->size);
	dprintf("Block of size %u B starting on %x seems %s\n", header->size, start + sizeof(BlockHeader), ok?"OK":"BAD");
	return ok;
}
/*----------------------------------------------------------------------------*/
void* Allocator::getMemory(size_t size) const
{
	dprintf("Requested memory of size: %d B, aligning to %d\n", size, alignUp(size, ALIGMENT) );
	size = alignUp(size, ALIGMENT);
	void * res = NULL;
	if (size > (m_end - m_start)) return res;

	BlockHeader* header = (BlockHeader*)m_start;
	const size_t real_size = size + sizeof(BlockHeader) + sizeof(BlockFooter);
	dprintf("Statring search from %x (m_start %x) size %d\n", header, m_start, size);

	while (res == NULL && ( (uintptr_t)header < m_end) ) {
		printf("Testing block at %x, size %d\n", header, header->size);
		if (header->free && (header->size >= size) ) { // first fit
			res = (void*)(header + 1);
			printf ("Found free block at %x, size %d\n", header, header->size );
			if ( (header->size - size) < (sizeof(BlockHeader) + sizeof(BlockFooter)) ) {
				// not splitting
				header->free = false;
				break;
			} else {
				// split
				const size_t cut_off = header->size + sizeof(BlockHeader) + sizeof(BlockFooter) - real_size;

				//unused rest
				printf ("Create unused block from: %x, size %d\n", (uintptr_t)header + real_size, cut_off);
				createBlock((uintptr_t)((uintptr_t)header + real_size), cut_off, true);
				//used block
				createBlock((uintptr_t)(header), real_size, false);
				break;
			}
		}

		// next block
		header = (BlockHeader*)((uintptr_t)header + header->size + sizeof(BlockHeader) + sizeof(BlockFooter));
	}
	return res;
}
/*----------------------------------------------------------------------------*/
void Allocator::freeMemory(void* address) const
{
	if ( ((uintptr_t)address <= m_start) 
		|| ((uintptr_t)address > m_end) ) return; // not my heap

	BlockHeader* my_header = (BlockHeader*)((uintptr_t)address - sizeof(BlockHeader));
	BlockHeader* from_header = my_header;
	assert(checkBlock((uintptr_t)my_header)); // should be a block

	if (my_header->free) return; //already freed??

	size_t size =  my_header->size + sizeof(BlockHeader) + sizeof(BlockFooter);
	dprintf("Freeing block of size %u, real size:%u\n", my_header->size, size);

	if ((uintptr_t)my_header > m_start)	{
		// check previous block
		BlockFooter* prev_footer = (BlockFooter*)((uintptr_t)my_header - sizeof(BlockFooter));
		BlockHeader* prev_header = (BlockHeader*)((uintptr_t)prev_footer - prev_footer->size - sizeof(BlockHeader));
		if (prev_header->free){
			from_header = prev_header;
			size += prev_header->size + sizeof(BlockHeader) + sizeof(BlockFooter);
		}
	}
	// next header
	BlockHeader * next_header = (BlockHeader*)((uintptr_t)my_header 
	      + my_header->size + sizeof(BlockHeader) + sizeof(BlockFooter) );
	if ( ((uintptr_t)next_header < m_end) && next_header->free){
		// add next block
		dprintf("Adding next header of size %u\n", next_header->size );
		size += next_header->size + sizeof(BlockHeader) + sizeof(BlockFooter);
	}
	createBlock((uintptr_t)from_header, size, true);
}
/*----------------------------------------------------------------------------*/
bool Allocator::check()
{
	BlockHeader* header = (BlockHeader*)m_start;
	while ( (uintptr_t)header < m_end) {
		printf("Testing block at %x, size %d\n", header, header->size);
		if (!checkBlock((uintptr_t)header)) return false;
		header = (BlockHeader*)((uintptr_t)header + header->size + sizeof(BlockHeader) + sizeof(BlockFooter));
	}
	return true;
}
