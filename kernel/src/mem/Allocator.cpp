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
#include "InterruptDisabler.h"
#include "api.h"

//#define ALLOCATOR_DEBUG

#ifndef ALLOCATOR_DEBUG
#define PRINT_DEBUG(...)
#else
#define PRINT_DEBUG(ARGS...) \
  printf("[ ALLOCATOR_DEBUG ]: "); \
  printf(ARGS);
#endif


void Allocator::setup(const uintptr_t from, const size_t length)
{
	uintptr_t end = alignDown(from + length, ALIGMENT);
	uintptr_t start = alignUp(from, ALIGMENT);

	PRINT_DEBUG(" Testing initiail memory chunk: %p to %p.\n", start, end);

	//test first
	*(uint8_t*)(end - sizeof(MAGIC)) = MAGIC;
	ASSERT (*(uint8_t*)(end - sizeof(MAGIC)) == MAGIC);
	
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
	PRINT_DEBUG( "Creating block from %p of size %u : %s\n", start, size, free?"free":"used" );
	ASSERT (start >= m_start);
	ASSERT ( (start + size) <= m_end);
	// test aligment
	ASSERT ((start & ~(ALIGMENT - 1)) == start);
	ASSERT ((size & ~(ALIGMENT - 1)) == size);
		
	// setup
	BlockHeader* header = (BlockHeader *)start;
	BlockFooter* footer = (BlockFooter *)(start + size - sizeof(BlockFooter));
	
	header->size = size - sizeof(BlockHeader) - sizeof(BlockFooter);
	header->magic = BIG_MAGIC;
	header->free = free;

	footer->magic = BIG_MAGIC;
	footer->size = size - sizeof(BlockHeader) - sizeof(BlockFooter);
	
	ASSERT (checkBlock(start));
}
/*----------------------------------------------------------------------------*/
bool Allocator::checkBlock(const uintptr_t start) const
{
	BlockHeader* header = (BlockHeader*)start;
	BlockFooter* footer = (BlockFooter*)(start + sizeof(BlockHeader) + header->size);
	bool ok =  (header->magic == BIG_MAGIC) && (footer->magic == BIG_MAGIC)
	        && (header->size == footer->size);
	PRINT_DEBUG ("Block of size %u B starting on %x seems %s\n", header->size, start + sizeof(BlockHeader), ok?"OK":"BAD");
	return ok;
}
/*----------------------------------------------------------------------------*/
void* Allocator::getMemory(size_t size) const
{
	InterruptDisabler interrupts;

	PRINT_DEBUG ("Requested memory of size: %d B, aligning to %d\n", size, alignUp(size, ALIGMENT) );
	size = alignUp(size, ALIGMENT);
	void * res = NULL;
	if (size > (m_end - m_start)) return res;

	BlockHeader* header = (BlockHeader*)m_start;
	const size_t real_size = size + sizeof(BlockHeader) + sizeof(BlockFooter);

	while (res == NULL && ( (uintptr_t)header < m_end) ) {
		PRINT_DEBUG ("Testing block at %p, size %d\n", header, header->size);
		if (header->free && (header->size >= size) ) { // first fit
			res = (void*)(header + 1);
			PRINT_DEBUG ("Found free block at %p, size %d\n", header, header->size );
			if ( (header->size - size) < (sizeof(BlockHeader) + sizeof(BlockFooter)) ) {
				// not splitting
				PRINT_DEBUG ("Blocksize (%u) is near requested size (%u), NOT splitting.\n", header->size, size);
				header->free = false;
				break;
			} else {
				// split
				PRINT_DEBUG ("Blocksize (%u) is bigger than requested size (%u), splitting", header->size, size);
				const size_t cut_off = header->size + sizeof(BlockHeader) + sizeof(BlockFooter) - real_size;

				//unused rest
				createBlock((uintptr_t)((uintptr_t)header + real_size), cut_off, true);
				//used block
				createBlock((uintptr_t)(header), real_size, false);
				break;
			}
		}

		// next block
		header = (BlockHeader*)((uintptr_t)header + header->size + sizeof(BlockHeader) + sizeof(BlockFooter));
	}
	
	if (res == NULL) {
		dprintf("------------OUT OF MEMORY------------\n");
	}
	return res;
}
/*----------------------------------------------------------------------------*/
void Allocator::freeMemory( const void* address ) const
{
	if ( ((uintptr_t)address <= m_start) 
		|| ((uintptr_t)address > m_end) ) return; // not my heap

	InterruptDisabler interrupts;
	
	BlockHeader* my_header = (BlockHeader*)((uintptr_t)address - sizeof(BlockHeader));
	BlockHeader* from_header = my_header;
	ASSERT (checkBlock((uintptr_t)my_header)); // should be a block
	PRINT_DEBUG ("Freeing block at %p, size: %u free: %s.\n", my_header, my_header->size, my_header->free ? "YES" : "NO" );
	if (my_header->free) return; //already freed??

	size_t size =  my_header->size + sizeof(BlockHeader) + sizeof(BlockFooter);
	//dprintf("Freeing block of size %u, real size:%u\n", my_header->size, size);

	if ((uintptr_t)my_header > m_start)	{
		// check previous block
		BlockFooter* prev_footer = (BlockFooter*)((uintptr_t)my_header - sizeof(BlockFooter));
		BlockHeader* prev_header = (BlockHeader*)((uintptr_t)prev_footer - prev_footer->size - sizeof(BlockHeader));

		PRINT_DEBUG ("Testing previous header %p : %s.\n", prev_header, prev_header->free ? "FREE" : "USED");

		if (prev_header->free){
			from_header = prev_header;
			size += prev_header->size + sizeof(BlockHeader) + sizeof(BlockFooter);
			PRINT_DEBUG ("Merging with the previous free block of size: %u.\n", prev_header->size );
		}
	}
	// next header
	BlockHeader * next_header = (BlockHeader*)((uintptr_t)my_header 
	      + my_header->size + sizeof(BlockHeader) + sizeof(BlockFooter) );
	if ( ((uintptr_t)next_header < m_end) && next_header->free){
		// add next block
		PRINT_DEBUG ("Adding next header of size %u\n", next_header->size );
		size += next_header->size + sizeof(BlockHeader) + sizeof(BlockFooter);
	}
	createBlock((uintptr_t)from_header, size, true);
}
/*----------------------------------------------------------------------------*/
bool Allocator::check()
{
	BlockHeader* header = (BlockHeader*)m_start;
	while ( (uintptr_t)header < m_end) {
		PRINT_DEBUG ("Testing block at %p, size %d\n", header, header->size);
		if (!checkBlock((uintptr_t)header)) return false;
		header = (BlockHeader*)((uintptr_t)header + header->size + sizeof(BlockHeader) + sizeof(BlockFooter));
	}
	return true;
}
