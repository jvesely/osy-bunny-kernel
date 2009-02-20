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
 *   @version $Id: FrameAllocator.cpp 732 2009-02-14 23:24:08Z slovak $
 *   @note
 *   Semestral work for Operating Systems course at MFF UK \n
 *   http://dsrg.mff.cuni.cz/~ceres/sch/osy/main.php
 *   
 *   @date 2008-2009
 */

/*!
 * @file 
 * @brief Contains definition of class FrameAllocator.
 *
 * Class FrameAllocator provides interface for physical memory 
 * allocation. Supports 7 frame sizes and allocation of more subsequent
 * frames (all of them of the same size).
 */

#include "FrameAllocator.h"
#include "structures/Bitset.h"
#include "cpp.h"
#include "InterruptDisabler.h"

//#define FRALLOC_DEBUG

#ifndef FRALLOC_DEBUG
#define PRINT_DEBUG(...)
#else
#define PRINT_DEBUG(ARGS...) \
	printf("[ FRALLOC_DEBUG ]: "); \
	printf(ARGS);
#endif

using namespace Processor;

/*----------------------------------------------------------------------------*/

uintptr_t FrameAllocator::init( size_t memory_size, const size_t kernel_end )
{
	InterruptDisabler interrupts;

	PRINT_DEBUG("Initializing Frame Allocator...\n");
	// frallocator needs the memory size and kernel size
	ASSERT(memory_size && kernel_end > ADDR_PREFIX_KSEG0);

#ifdef FRALLOC_DEBUG
	uint kernel_size = ADDR_TO_USEG(kernel_end);
	PRINT_DEBUG("Memory size: %u\n", memory_size);
	PRINT_DEBUG("Kernel size: %u\n", kernel_size);
#endif

	// the actual position where we will put our structures
	char* pos = (char*)kernel_end;
	ASSERT(kernel_end % 0x4 == 0);

	/*--------------------------------------------------------------------------
	  find the largest frame size to know how much memory I can use
	--------------------------------------------------------------------------*/

	PageSize frame = MIN_FRAME;

	// let's assume we have more memory than size of the smallest frame
	ASSERT(memory_size > frameSize(MIN_FRAME));

	// and that the KSEG0 segment's size is a multiple of the largest frame size
	while (largerFrameSize(frame) <= memory_size 
	        && largerFrame(frame) <= MAX_FRAME)
		frame = (PageSize)(frame + 1);
	m_maxFrame = frame;

	// if the memory is not aligned to a multiple of the smallest frame
	// the rest of the memory will not be used
	if (memory_size % frameSize(MIN_FRAME) != 0) {
		memory_size = memory_size - (memory_size % frameSize(MIN_FRAME));
		printf("FrameAllocator: Memory size not aligned to the smallest frame, \
only %d B of the memory will be used.\n", memory_size);
	}
	m_memorySize = memory_size;

	/*--------------------------------------------------------------------------
	  set the number of free frames and offset in the bitset of each frame size
	--------------------------------------------------------------------------*/

	// for used frame sizes, the largest frames will start on offset 0
	uint total_frames_kseg = 0;
	// we must separately count the offset to the KSEG0 and KUSEG maps
	uint total_frames_kuseg = 0;

	// KSEG and KUSEG sizes
	uint kseg_size = min<uint>(memory_size, KSEG0_SIZE);
	uint kuseg_size = memory_size - kseg_size;

	// round the sizes up to multiple of the largest frame size
	kseg_size = roundUp(kseg_size, frameSize(m_maxFrame));
	if (kuseg_size > 0) {
		// the kseg_size should not have been modified by the roundUp
		ASSERT(kseg_size == KSEG0_SIZE);

		kuseg_size = roundUp(kuseg_size, frameSize(m_maxFrame));
	}

	// determine the number of frames of each size
	for (frame = MAX_FRAME; frame >= MIN_FRAME; frame = (PageSize)(frame - 1)) {

		// number of frames of type 'frame'
		uint frames_kseg = kseg_size / frameSize(frame);
		uint frames_kuseg = kuseg_size / frameSize(frame);
	
		// set count of free frames on this level
		freeFrames(frame, KSEG) = frames_kseg;
		freeFrames(frame, KUSEG) = frames_kuseg;

		// set the offset of level-th map
		offset(frame, KSEG) = total_frames_kseg;
		offset(frame, KUSEG) = total_frames_kuseg;

		// set the offset of the end of level-th map
		if (frame < MAX_FRAME) {
			offsetEnd(largerFrame(frame), KSEG) = total_frames_kseg;
			offsetEnd(largerFrame(frame), KUSEG) = total_frames_kuseg;
		}

		total_frames_kseg += frames_kseg;
		total_frames_kuseg += frames_kuseg;
	}
	offsetEnd(MIN_FRAME, KSEG) = total_frames_kseg;
	offsetEnd(MIN_FRAME, KUSEG) = total_frames_kuseg;

	/*--------------------------------------------------------------------------
	  create and initialize bitmaps (for KSEG0/1 and other memory)
	--------------------------------------------------------------------------*/
	m_bitmap[KSEG] = new (pos) 
	                     Bitset(pos + sizeof(Bitset), total_frames_kseg);
	pos += (sizeof(Bitset) + Bitset::getContainerSize(total_frames_kseg));

	// do we have even to create the Bitset?
	if (total_frames_kuseg > 0) {
		m_bitmap[KUSEG] = new (pos) Bitset(pos + sizeof(Bitset),
                                            total_frames_kuseg);
		pos += (sizeof(Bitset) 
		        + Bitset::getContainerSize(total_frames_kuseg));
	} else {
		m_bitmap[KUSEG] = NULL;
	}

	// determine the count of frames used by kernel and my structures
	m_endOfBlockedFrames = roundUp(ADDR_TO_USEG((uint)pos), 
	                               frameSize(MIN_FRAME));
	uint used_frames =  m_endOfBlockedFrames / frameSize(MIN_FRAME);

#ifdef FRALLOC_DEBUG
	// check the offsets
	PRINT_DEBUG("Frame sizes to offsets mapping, KSEG:\n");
	for (PageSize i = MIN_FRAME; i <= m_maxFrame; i = (PageSize)(i + 1)) {
		PRINT_DEBUG("Frame size: %u, offset: %u\n", 
			frameSize(i), offset(i, KSEG));
	}
	PRINT_DEBUG("Frame sizes to offsets mapping, KUSEG:\n");
	for (PageSize i = MIN_FRAME; i <= m_maxFrame; i = (PageSize)(i + 1)) {
		PRINT_DEBUG("Frame size: %u, offset: %u\n", 
			frameSize(i), offset(i, KUSEG));
	}

	checkStructures();
#endif

	PRINT_DEBUG("Setting frames occupied by kernel and frallocator as used\n");

	// set all used frames as used in the bitmap
	setFramesAsUsed(offset(MIN_FRAME, KSEG), MIN_FRAME, used_frames, KSEG);

	PRINT_DEBUG("Setting frames beyond the memory size as used\n");

	// set all unavailable frames as used in the bitmap
	if (kuseg_size == 0) {
		uint end_of_memory = 
			offset(MIN_FRAME, KSEG) + (m_memorySize / frameSize(MIN_FRAME));

		uint unavailable_frames = 
			(kseg_size / frameSize(MIN_FRAME)) 
		      - (m_memorySize / frameSize(MIN_FRAME));
		
	/*	PRINT_DEBUG("Only %d smallest frames are available in KSEG, there are \
 %d unavailable frames\n", end_of_memory, unavailable_frames);*/

		ASSERT(unavailable_frames 
		        == offsetEnd(MIN_FRAME, KSEG) - end_of_memory);

		setFramesAsUsed(end_of_memory, MIN_FRAME, unavailable_frames, KSEG);
	} else {
		// no need to set anything in kseg
		uint end_of_memory = 
			offset(MIN_FRAME, KUSEG) 
		     + ((m_memorySize - KSEG0_SIZE) / frameSize(MIN_FRAME));

		uint unavailable_frames = 
			(kuseg_size / frameSize(MIN_FRAME)) 
		      - ((m_memorySize - KSEG0_SIZE) / frameSize(MIN_FRAME));

		/*PRINT_DEBUG("Only %d smallest frames are available in KUSEG, there are \
  %d unavailable frames\n", end_of_memory, unavailable_frames);*/

		ASSERT(unavailable_frames 
		        == offsetEnd(MIN_FRAME, KUSEG) - end_of_memory);

		setFramesAsUsed(end_of_memory, MIN_FRAME, unavailable_frames, KUSEG);
	}

#ifdef FRALLOC_DEBUG
	checkStructures();
	checkBitmaps();

	checkStructures();
#endif

	m_initialized = true;
	PRINT_DEBUG("Frame Allocator initialized. Returning: %p\n", pos);
	return (uintptr_t)pos;
}

/*----------------------------------------------------------------------------*/

uint FrameAllocator::frameAlloc( void** address, const size_t count, 
	const PageSize frame, const uint flags )
{
	ASSERT(m_initialized);

	PRINT_DEBUG("FrameAllocator::alloc() started...\n\n");
	PRINT_DEBUG("Address: %x, Count: %u, Frame size: %u Flags: %x\n",
		*address, count, frameSize(frame), flags);

	ASSERT(address && count);

	if (!checkBitmaps()) {
		printf("FrameAllocator: Bitmap inconsistent!\n");
		return 0;
	}

	if (requestAtKseg(flags))
		return allocateAtKseg0( address, count, frame );

	if (requestOutOfKseg(flags))
		return allocateAtKuseg( address, count, frame );

	// else 
	ASSERT(requestUser(flags));
	return allocateAtAddress(*address, count, frame );
}

/*---------------------------------------------------------------------------*/

uint FrameAllocator::allocateAtKuseg( 
	void** address, const uint count, const PageSize frame )
{
	InterruptDisabler interrupts;

	PRINT_DEBUG("allocateAtKuseg() started...\n");

	ASSERT(m_initialized);

	// if there is no KUSEG segment at all
	if (m_bitmap[KUSEG] == NULL) {
		// allocate the memory in the KSEG segment
		return allocateAtSegment(address, count, frame, KSEG);
	}
	// if there is some KSEG segment, try to allocate there
	uint free_kuseg = allocateAtSegment(address, count, frame, KUSEG);

	// if the allocation in KUSEG was successful, end
	if (free_kuseg == count)
		return free_kuseg;

	// if not, there should not be enough allocated frames
	ASSERT(free_kuseg < count);
	PRINT_DEBUG("Trying to allocate on the border between KSEG and KUSEG\n");

	/*
	 * try to allocate on the border
	 */

	// free frames from the beginning of this frame size's bitmap
	free_kuseg = emptyFrames(offset(frame, KUSEG), count, frame, KUSEG);
	// if there are some free frames, starting from the border, in KUSEG segment
	if (free_kuseg > 0) {
		uint count_kseg = count - free_kuseg;
		uint start_kseg = offsetEnd(frame, KSEG) - count_kseg;
		// check if there are enough frames on the KSEG side of the border
		uint free_kseg = emptyFrames(start_kseg, count_kseg, frame, KSEG);

		// if there are enough
		if (free_kseg == count_kseg) {
			// determine the address
			*address = (void*)getAddress(start_kseg, frame, KSEG);

			PRINT_DEBUG("Found enough free frames, at address %p\n", *address);

			ASSERT((uintptr_t)(*address) % frameSize(frame) == 0);

#ifdef FRALLOC_DEBUG
			int n = start_kseg - offset(frame, KSEG);
			PRINT_DEBUG("I.e. from the %u-th frame of size %u\n",
				n, frame);

			ASSERT(n >= 0);
#endif
			// set frames from global offset start_kseg
			// of size requested by user; count requested by user
			ASSERT(start_kseg = offset(frame, KSEG)
			                     + (uintptr_t)(*address) / frameSize(frame));

			setFramesAsUsed(start_kseg, frame, count_kseg, KSEG);
			setFramesAsUsed(offset(frame, KUSEG), frame, free_kuseg, KUSEG);

			// return the count of frames allocated
			return count;
		}
	}

	// else try to allocate at KSEG
	return allocateAtSegment(address, count, frame, KSEG);
}

/*---------------------------------------------------------------------------*/

uint FrameAllocator::allocateAtSegment( void** address, const uint count,
	const PageSize frame, const AddressType type )
{
	InterruptDisabler interrupts;

	PRINT_DEBUG("allocateAtSegment() started...\n");

	ASSERT(m_initialized);

	ASSERT(frame >= MIN_FRAME);
	ASSERT(count > 0);
	ASSERT(m_bitmap[type] != NULL);

	// else check the recently deallocated block
	if ( ((count * frameSize(frame)) <= m_lastFreedSize[type]) 
	     && (m_lastFreed[type] % frameSize(frame) == 0) ) {
		PRINT_DEBUG("Trying to use last freed block\n");
		// determine the offset from the address
		uint addr_offset = (uintptr_t)(m_lastFreed[type]) / frameSize(frame);
		// check if there is still enough free frames
		uint free = emptyFrames(
			offset(frame, type) + addr_offset, count, frame, type);

		if (free == count) {
			PRINT_DEBUG("Last freed block is large enough\n");
			// save the address
			*address = (void*)m_lastFreed[type];

			PRINT_DEBUG("Found enough free frames, at address %x\n", *address);

			ASSERT((uintptr_t)(*address) % frameSize(frame) == 0);
			ASSERT((uintptr_t)(*address) < m_memorySize);

			setFramesAsUsed(
				offset(frame, type) + addr_offset, frame, count, type);
			return count;
		} 

		// if we allocated the frames at the m_lastFreed position
		// or there are not enough free frames (i.e. somebody allocated there)
		// reset the m_lastFreed and m_lastFreedSize
		m_lastFreed[type] = NULL;
		m_lastFreedSize[type] = 0;
	}

	// else try to find enough free frames normally

	uint search_counts[7];
	search_counts[frame] = count;

	// determine the size of frame(s) we'll try to find
	PageSize search_size = defaultFrame(search_counts, frame, type);
	ASSERT(search_size >= frame);

	uint search_count = search_counts[search_size];
	uint search_start = offset(search_size, type);
	
	PRINT_DEBUG("Starting to search for %u frames of size %u, from offset %u\n",
		search_count, frameSize(search_size), search_start);

	// here we will store the maximum number of subsequent empty frames of 
	// the size user wants
	uint max_empty = 0;

	// loop until we are searching for frames large enough or 
	// until we're not on the end of the bitmap
	while ((search_size >= frame) && (search_start < theEnd(type) - 1)) {
		
		ASSERT(search_size >= MIN_FRAME);

		// find offset of the first empty frame
		search_start = findFirstEmpty(search_start, type);
		PRINT_DEBUG("First empty frame found on global offset %u\n", 
			search_start);
		
		// we're beyond the end of interesting frames' map
		if (search_start >= offsetEnd(frame, type))
			break;

		ASSERT(search_size >= MIN_FRAME);
		
		// we're beyond the end of the actually sought frame size's bitmap
		if (search_start >= offsetEnd(search_size, type)) {
			// determine what size is this frame and set proper search_count
			while (search_start >= offsetEnd(search_size, type)) {
				search_size = (PageSize)(search_size - 1);
			}
			search_count = search_counts[search_size];
			PRINT_DEBUG("New search size: %u\n", search_size);
			PRINT_DEBUG("New search count: %u\n", search_count);

			ASSERT(search_size >= frame);
			// else we can continue executing commands in this loop
		}

		// it's still the frame size we want
		ASSERT(search_start < offsetEnd(search_size, type));
			
		// try if there are enough frames free of this size
		uint free = emptyFrames(search_start, search_count, search_size, type);
		PRINT_DEBUG("Found %d free frames, wanted %d frames.\n", free, search_count);
		ASSERT(free <= search_count);
		ASSERT(search_start + free <= offsetEnd(MIN_FRAME, type));

		// if yes, we've found the space to allocate
		if (free == search_count) {
			// so determine the address
			*address = (void*)getAddress(search_start, search_size, type);

			PRINT_DEBUG("Found enough free frames, at address %x\n", *address);

			ASSERT((uintptr_t)(*address) % frameSize(frame) == 0);
			ASSERT((uintptr_t)(*address) < m_memorySize);

			uint addr_offset = (uintptr_t)(*address) / frameSize(frame);
#ifdef FRALLOC_DEBUG
			PRINT_DEBUG("I.e. from the %u-th frame of size %u\n",
				search_start - offset(search_size, type), 
				frameSize(search_size));

			ASSERT(search_start > offset(search_size, type));
#endif
			// set frames from global offset search_start
			// of size requested by user; count requested by user
			setFramesAsUsed(offset(frame, type) + addr_offset, 
				frame, count, type);
			// return the count of frames allocated
			return count;
		} 

		// else there are not enough free frames of this size
		PRINT_DEBUG("Not enough frames found.\n");

		// check if we haven't found more than maximum of empty frames
		if ((free * (frameSize(search_size) / frameSize(frame))) > max_empty) {
			max_empty = frameSize(search_size) / frameSize(frame);
			*address = (void*)getAddress(search_start, search_size, type);
		}

		// and in either case set the start of search to the first
		// frame after the free frames (which can be the first full
		// frame or the first frame of smaller size)
		search_start = search_start + free;
		PRINT_DEBUG("New start of search: %u\n", search_start);
		// if the new start is the first frame of smaller size
		
		ASSERT(search_start <= offsetEnd(search_size, type));

		if (search_start == offsetEnd(search_size, type)) {
			// set new search_size and search_count
			if (search_size == MIN_FRAME)
				break;
			ASSERT(search_size > MIN_FRAME);
			search_size = (PageSize)(search_size - 1);
			search_count = search_counts[search_size];
			PRINT_DEBUG("New search size: %u\n", frameSize(search_size));
			PRINT_DEBUG("New search count: %u\n", search_count);
		}
	} // end while

	/* the loop ended in one of these cases:
		1) search_size < frame, i.e. we haven't found enough empty frames
		   of the size user wanted
		2) search_start > theEnd(type): this means the same in case user wanted
		   to search for the smallest frames
	   in each case return the maximum number of found subsequent free frames
	   *address holds address of the largest segment of free frames
	   or the initial value
	*/

	PRINT_DEBUG("allocateAtSegment() ends, found only %u free frames.\n", max_empty);

	return max_empty;
}

/*---------------------------------------------------------------------------*/

uint FrameAllocator::allocateAtAddress( const void* address, 
	const uint count, const PageSize frame )

{
	InterruptDisabler interrupts;

	PRINT_DEBUG("allocateAtAddress() started...\n");

	ASSERT(m_initialized);

	/*
	 * Determine where the block lies (KSEG / KUSEG / their border)
	 */

	uintptr_t addr = (uintptr_t)address;
	ASSERT(addr % frameSize(frame) == 0);

	ASSERT(addr < m_memorySize);
	//ASSERT((addr + (count * frameSize(frame))) < m_memorySize);
	
	// if trying to allocate in the blocked frames
	if (addr < m_endOfBlockedFrames)
		return 0;

	AddressType type;

	if ((addr < KSEG0_SIZE) /* the block will start in KSEG */
		&& ((addr + (count * frameSize(frame))) > KSEG0_SIZE) /* and end in KUSEG */)
	{
		// so the block will lie on the border of KSEG and KUSEG segments
		PRINT_DEBUG("The allocated block will lie on the border between the \
KSEG and KUSEG segments.\n");

		// do we even have the KUSEG segment?
		ASSERT(m_bitmap[KUSEG]);

		// offset of the first frame in KSEG
		uint addr_offset_kseg = addr / frameSize(frame);
		// count of frames that will be in KSEG
		uint count_kseg = offsetEnd(frame, KSEG) - offset(frame, KSEG)
						  - addr_offset_kseg;
		ASSERT(count_kseg > 0);

		// offset of the first frame in KUSEG - the first frame of KUSEG
		uint addr_offset_kuseg = 0;
		// count of frames that will be in KUSEG
		uint count_kuseg = count - count_kseg;
		
		// is the count correct?
		ASSERT(count_kuseg == ((addr + (count * frameSize(frame))) - KSEG0_SIZE)
		                       / frameSize(frame));
		// is the address correct? (i.e. is the addr_offset within the range
		// of bitmap offsets for this frame size?)
		ASSERT(addr_offset_kuseg + count_kuseg 
			<= ( offsetEnd(frame, KUSEG) - offset(frame, KUSEG) ));
		
		// how many empty frames of this size are in the required place in KSEG?
		uint free_kseg = emptyFrames( 
			offset(frame, KSEG) + addr_offset_kseg, count_kseg, frame, KSEG );

		// if not enough free frames, return
		if (free_kseg < count_kseg)
			return free_kseg;

		ASSERT(free_kseg == count_kseg);

		// how many empty frames of this size are in the required place in KUSEG
		uint free_kuseg = emptyFrames(offset(frame, KUSEG) + addr_offset_kuseg,
			count_kuseg, frame, KUSEG);

		ASSERT(free_kuseg <= count_kuseg);

		if (free_kuseg == count_kuseg) {
			// enough free frames in both KSEG and KUSEG
			PRINT_DEBUG("Found enough free frames, setting them as used...\n");
			setFramesAsUsed(offset(frame, KSEG) + addr_offset_kseg, 
				frame, count_kseg, KSEG);
			setFramesAsUsed(
				offset(frame, KUSEG) + addr_offset_kuseg, 
				frame, count_kuseg, KUSEG);
		} else {
			PRINT_DEBUG("Not enough free frames found. Exiting\n");
		}
	
		return free_kseg + free_kuseg;

	} 
	// else (not on the border)
	
	// set proper help variables 
	if (addr >= KSEG0_SIZE) {
		// the whole block will lie within the KUSEG segment
		addr -= KSEG0_SIZE;
		type = KUSEG;
		ASSERT(m_bitmap[KUSEG]);
		PRINT_DEBUG("The allocated block will lie within KUSEG segment.\n");
	} else {
		// the whole block will lie within the KSEG segment
		type = KSEG;
		PRINT_DEBUG("The allocated block will lie within KSEG segment.\n");
	}

	// determine the offset of the first frame
	uint addr_offset = addr / frameSize(frame);

	// is the address correct? (i.e. is the addr_offset within the range
	// of bitmap offsets for this frame size?)
	ASSERT((offset(frame, type) + addr_offset) < offsetEnd(frame, type));

	// how many empty frames of this size are there, starting from this addr
	uint free = emptyFrames( offset(frame, type) 
	                          + addr_offset, count, frame, type );

	ASSERT(free <= count);
	// if there are enough empty frames of this size
	if (free == count) {
		PRINT_DEBUG("Found enough free frames, setting them as used...\n");
		setFramesAsUsed(offset(frame, type) + addr_offset, frame, count, type);
	} else {
		PRINT_DEBUG("Not enough free frames found. \
Found only %u frames free. Exiting..\n", free);
	}

	return free;
}

/*----------------------------------------------------------------------------*/

bool FrameAllocator::frameFree( 
	const void* address, const size_t count, const PageSize frame )
{
	InterruptDisabler interrupts;

	ASSERT(m_initialized);

	if (!checkBitmaps()) {
		printf("FrameAllocator: Bitmap inconsistent!\n");
		return false;
	}

	PRINT_DEBUG("Request to free %u frames of size %u, starting from address %x\n", count, frameSize(frame), address);

	uintptr_t addr = (uintptr_t)address;
	
	ASSERT(addr < m_memorySize);

	// don't allow freeing the kernel or my memory
	if (addr < m_endOfBlockedFrames) {
		PRINT_DEBUG("Address points to blocked memory (to %d or before)!\n", 
			m_endOfBlockedFrames);
		return false;
	}

	// don't allow freeing the non-existing memory (beyond m_memorySize)
	if (addr + (count * frameSize(frame)) > m_memorySize) {
		PRINT_DEBUG("The block is ending out of the available memory!\n");
		return false;
	}

	// check if the address is aligned at the frame start
	if (addr % frameSize(frame) != 0) {
		PRINT_DEBUG("Address is not alligned!\n");
		return false;
	}

	uint addr_offset_kseg = 0;
	uint count_kseg = 0;
	uint addr_offset_kuseg = 0;
	uint count_kuseg = 0;
	PageSize fr_size = MIN_FRAME;
	uint cnt = count * (frameSize(frame) / frameSize(fr_size));

	if (addr < KSEG0_SIZE) {
		// if part of the block or whole block lies in KSEG
		// determine the number of the first frame of size MIN_FRAME
		addr_offset_kseg = addr / frameSize(fr_size);
		// and count of the frames in KSEG
		count_kseg = min<uint>( (offsetEnd(fr_size, KSEG) - offset(fr_size, KSEG)
						  - addr_offset_kseg), cnt);
		PRINT_DEBUG("In KSEG we will free %d frames of size %d starting from \
local offset %d\n", count_kseg, frameSize(fr_size), addr_offset_kseg);
		ASSERT(count_kseg > 0);
	}
	
	if (addr + (cnt * frameSize(fr_size)) >= KSEG0_SIZE) {
		// if part of the block or whole block lies in KUSEG
		// determine the number of the first frame
		addr_offset_kuseg = (addr < KSEG0_SIZE) 
		                     ? 0 
							 : (addr - KSEG0_SIZE) / frameSize(fr_size);
		count_kuseg = cnt - count_kseg;

		PRINT_DEBUG("In KUSEG we will free %d frames of size %d starting from \
local offset %d\n", count_kuseg, frameSize(fr_size), addr_offset_kuseg);

		ASSERT(count_kuseg == (addr + (count * frameSize(fr_size)) - KSEG0_SIZE)
		                       / frameSize(fr_size));
		// is the address correct? (i.e. is the addr_offset within the range
		// of bitmap offsets for this frame size?)
		ASSERT(addr_offset_kuseg + count_kuseg 
			<= ( offsetEnd(fr_size, KUSEG) - offset(fr_size, KUSEG) ));
	}

	ASSERT(count_kseg + count_kuseg == cnt);

	if (count_kseg > 0) {
		ASSERT(m_bitmap[KSEG]);
		// how many full frames of this size are there, starting from this addr
		uint used_kseg = fullFrames( 
			offset(fr_size, KSEG) + addr_offset_kseg, count_kseg, fr_size, KSEG);
		PRINT_DEBUG("Found %u used frames from %u in KSEG.\n", 
			used_kseg, count_kseg);
		
		if (used_kseg != count_kseg) {
			PRINT_DEBUG("Not enough allocated frames found, exiting...\n");
			return false;
		}
		
		PRINT_DEBUG("Found enough allocated frames in KSEG. Setting them free.\n");
		setFramesAsFree(offset(fr_size, KSEG) + addr_offset_kseg, fr_size, 
			count_kseg, KSEG);

		m_lastFreed[KSEG] = addr_offset_kseg * frameSize(MIN_FRAME);
		m_lastFreedSize[KSEG] = count_kseg * frameSize(MIN_FRAME);

		return true;
	}

	if (count_kuseg > 0) {
		ASSERT(m_bitmap[KUSEG]);
		// how many full frames of this size are there, starting from this addr
		uint used_kuseg = fullFrames( 
			offset(fr_size, KUSEG) + addr_offset_kuseg, count_kuseg, fr_size, KUSEG);
		PRINT_DEBUG("Found %u used frames from %u in KUSEG.\n", 
			used_kuseg, count_kuseg);

		if (used_kuseg != count_kuseg) {
			PRINT_DEBUG("Not enough allocated frames found, exiting...\n");
			return false;
		}

		PRINT_DEBUG("Found enough allocated frames in KUSEG. Setting them free.\n");
		setFramesAsFree(offset(fr_size, KUSEG) + addr_offset_kuseg, fr_size, 
			count_kuseg, KUSEG);

		m_lastFreed[KUSEG] = addr_offset_kuseg * frameSize(MIN_FRAME);
		m_lastFreedSize[KUSEG] = count_kuseg * frameSize(MIN_FRAME);
		
		return true;
	}

	ASSERT(false);
	return false;
}

/*----------------------------------------------------------------------------*/

PageSize FrameAllocator::defaultFrame( uint* search_counts, 
	const PageSize frame, const AddressType type) const
{
	PageSize fr_size = frame;
	uint count = search_counts[frame];

	while (count > 1 && fr_size < m_maxFrame
		&& (freeFrames(fr_size, type) > count) ) {

		// save search_count for this frame_size
		search_counts[fr_size] = count;

		fr_size = (PageSize)(fr_size + 1);
		count = roundUp(count, FRAME_STEP) / FRAME_STEP;
	}
	/* the loop will stop in these cases:
		1) count == 1, i.e. we wan't to find only one frame; in this case 
		   the frame size not important, as we will rather try to find
		   1 frame of smaller size (one pass through the bitset) as 1 frame
		   of larger size (as it's probable this will not be empty)
		2) frame_size == m_maxFrameSize
		   in this case, it doesn't matter how many frames we need to find, 
		   as we can't search for larger frames
		3) count of free frames of this size is less or equal the count we'll
		   search for. in case there are less free frames than we want and
		   the actual frame size is larger than the original, we return
		   smaller size
	*/
	if ( (freeFrames(fr_size, type) < count) && fr_size > frame) {
		fr_size = (PageSize)(fr_size - 1);
	} else {
		// save the last search_count
		search_counts[fr_size] = count;
	}
	return fr_size;
}

/*----------------------------------------------------------------------------*/

void FrameAllocator::setFramesAsUsed( const uint global_offset, 
	const PageSize frame, const uint count, const AddressType type )
{
	if (count == 0)
		return;

	InterruptDisabler interrupts;

	/*--------------------------------------------------------------------------
	  Mark given frames as used in the map
	--------------------------------------------------------------------------*/

	PRINT_DEBUG("Setting %u frames of size %u as used in map %s, starting\
 from global offset %u\n", 
		count, frameSize(frame), (type == KSEG) ? "KSEG" : "KUSEG", global_offset);

	uint local_offset = global_offset - offset(frame, type);
	PRINT_DEBUG("Starting from %u-th frame of size %u\n", local_offset, 
		frameSize(frame));
	
	// set the given frames as used
	m_bitmap[type]->bits(global_offset, count, true);
	// decrease the count of free frames saved
	freeFrames(frame, type) -= count;

	/*--------------------------------------------------------------------------
	  Mark all their "children" and "children of their children", etc.
	--------------------------------------------------------------------------*/

	PRINT_DEBUG("Setting their children as used..\n");

	PageSize fr_size = frame;
	uint cnt = count;
	while (fr_size > MIN_FRAME) {
		cnt *= FRAME_STEP;

		PRINT_DEBUG("Setting %u frames of size %u as used\n", 
			cnt, smallerFrameSize(fr_size));
		m_bitmap[type]->bits(offsetOfChild(local_offset, fr_size, type), 
			cnt, true);

		fr_size = (PageSize)(fr_size - 1);
		ASSERT(fr_size >= MIN_FRAME);

		freeFrames(fr_size, type) -= cnt;
		local_offset *= FRAME_STEP;
	}

	/*--------------------------------------------------------------------------
	  Mark all their "parents" and "parents of their parents", etc.
	  This may end when no free parents are marked as used
	--------------------------------------------------------------------------*/

	PRINT_DEBUG("Setting their parents as used..\n");

	local_offset = global_offset - offset(frame, type);
	fr_size = frame;
	cnt = count;
	uint new_used = count;
	while (fr_size < m_maxFrame && new_used > 0) {

		/*
		 * Determine the number of affected parents
		 */

		uint first_parent = parent(local_offset);

		// relative offset of the last parent
		uint last_parent = parent(local_offset + cnt - 1);
	
		// number of parents
		cnt = last_parent - first_parent + 1;

		/*
		 * Set the parents as used
		 */

		fr_size = (PageSize)(fr_size + 1);

		ASSERT(fr_size <= m_maxFrame);
		// the number of parents we have to set as used, which were free before
		new_used = cnt;
		// only the first and last parent could have been free

		if (new_used > 0 && 
			m_bitmap[type]->bit(offset(fr_size, type) + first_parent))
			--new_used;
		if (new_used > 0 && 
			last_parent != first_parent && 
			m_bitmap[type]->bit(offset(fr_size, type) + last_parent))
			--new_used;

		PRINT_DEBUG("Setting %u frames of size %d from offset %u as used\n",
			cnt, frameSize(fr_size), offset(fr_size, type) + first_parent);
		if (new_used > 0) {
			m_bitmap[type]->bits(
				offset(fr_size, type) + first_parent, cnt, true);
			freeFrames(fr_size, type) -= new_used;
		}		

		local_offset /= FRAME_STEP;
	}
}

/*----------------------------------------------------------------------------*/

void FrameAllocator::setFramesAsFree( const uint global_offset, 
	const PageSize frame, const uint count, const AddressType type )
{
	if (count == 0)
		return;

	InterruptDisabler interrupts;

	/*--------------------------------------------------------------------------
	  Mark given frames as free in the bitmap
	--------------------------------------------------------------------------*/
	
	PRINT_DEBUG("Setting %u frames of size %u as free in map %s\n",
		count, frameSize(frame), (type == KSEG) ? "KSEG" : "KUSEG");

	uint local_offset = global_offset - offset(frame, type);

	// set the given frames as free in Kseg map
	m_bitmap[type]->bits(global_offset, count, false);
	// increase the count of free frames saved
	freeFrames(frame, type) += count;

	/*--------------------------------------------------------------------------
	  Mark all their "children" and "children of their children", etc.
	--------------------------------------------------------------------------*/

	PRINT_DEBUG("Setting their children free...\n");

	PageSize fr_size = frame;
	uint cnt = count;
	while (fr_size > MIN_FRAME) {
		cnt *= FRAME_STEP;

		PRINT_DEBUG("Setting %u frames of size %u as free in KSEG and KUSEG\n", 
			cnt, smallerFrameSize(fr_size));
		m_bitmap[type]->bits(offsetOfChild(local_offset, fr_size, type), 
			cnt, false);

		fr_size = (PageSize)(fr_size - 1);
		ASSERT(fr_size >= MIN_FRAME);

		freeFrames(fr_size, type) += cnt;
		local_offset *= FRAME_STEP;
	}

	/*--------------------------------------------------------------------------
	  Mark all their "parents" and "parents of their parents", etc.
	  This case is different from marking them as used
	  In parent we must check if all of his children are free, but it's
	  sufficient to check only the first and the last parent
	--------------------------------------------------------------------------*/

	PRINT_DEBUG("Setting their parents free...\n\n");

	local_offset = global_offset - offset(frame, type);
	fr_size = frame;
	cnt = count;
	bool freed = true;
	// if at least one of the parents was freed in this loop
	// we have to check their parents in the next loop
	while (fr_size < m_maxFrame && freed) {
		
		freed = false;

		/*
		 * Determine the number of affected parents
		 */

		// relative offset of the first parent's first child
		uint child_first = roundDown(local_offset, FRAME_STEP);

		// relative offset of the first parent
		uint first_parent = child_first / FRAME_STEP;

		// relative offset of the last parent's first child
		uint child_last = roundDown(local_offset + cnt - 1, FRAME_STEP);
		
		// relative offset of the last parent
		uint last_parent = child_last / FRAME_STEP;

		// number of parents
		cnt = last_parent - first_parent + 1;
		PRINT_DEBUG("Number of parents: %u\n", cnt);

		/*
		 * Check the first parent and eventually set it as free
		 */

		bool used = m_bitmap[type]->bit(offset(fr_size, type) + child_first);
		PRINT_DEBUG("1st parent's first child is %s\n", (used) ? "used" : "free");
		// check all other children until there is at least one used
		for (uint i = 1; i < FRAME_STEP && !used; ++i) {
			used = used || m_bitmap[type]->bit(
				offset(fr_size, type) + child_first + i);
		}

		PRINT_DEBUG("Setting 1st parent as %s\n", (used) ? "used" : "free" );
		if (!used) {
			m_bitmap[type]->bit( offset(largerFrame(fr_size), type) 
			                 + first_parent, used);
			++freeFrames(largerFrame(fr_size), type);
			freed = true;
		}

		// if there is only one parent, continue the loop
		// but adjust the variables at the end of the loop

		/*
		 * Check the last parent and eventually set it as free
		 */

		if (cnt > 1) {
			// there are at least 2 parents
			used = m_bitmap[type]->bit(offset(fr_size, type) + child_last);

			for (uint i = 1; i < FRAME_STEP && !used; ++i) {
				used = used || m_bitmap[type]->bit(
					offset(fr_size, type) + child_last + i);
			}

			PRINT_DEBUG("Setting last parent as %s\n", (used) ? "used" : "free" );
			if (!used) {
				ASSERT(offsetOfParent(child_last, fr_size, type) 
				  == offset(largerFrame(fr_size), type) + last_parent);
				m_bitmap[type]->bit( 
					offsetOfParent(child_last, fr_size, type), used);
				++freeFrames(largerFrame(fr_size), type);
				freed = true;
			}
		}

		// if there are only 2 parents, continue the loop (already handled)
		// but adjust the variables at the end of the loop

		/*
		 * Check all remaining parents and set all as free
		 */

		if (cnt > 2) {
			PRINT_DEBUG("Setting parents %u - %u as free\n", 2, cnt - 1);
			m_bitmap[type]->bits(
				offsetOfParent(local_offset, fr_size, type) + 1, 
				cnt - 2, false);
			freeFrames(largerFrame(fr_size), type) += cnt - 2;
			freed = true;
		}

		fr_size = (PageSize)(fr_size + 1);
		local_offset /= FRAME_STEP;
	}
}

/*---------------------------------------------------------------------------*/

void FrameAllocator::checkStructures() const
{
	printFree(KSEG); 
	if (m_bitmap[KUSEG]) 
		printFree(KUSEG);
}

/*---------------------------------------------------------------------------*/

void FrameAllocator::printFree( const AddressType type ) const
{
	PRINT_DEBUG("Checking the bitmap for %s...\n",
		(type == KSEG) ? "KSEG" : "KUSEG" );

	// check the bitmap
	for (PageSize i = MIN_FRAME; i <= m_maxFrame; i = (PageSize)(i + 1)) {
		uint free = 0;

		PRINT_DEBUG("Size: %d, offset: %d, end offset: %d\n",
			frameSize(i), offset(i, type), offsetEnd(i, type));

		for (uint j = offset(i, type); j < offsetEnd(i, type); ++j)
			free += !m_bitmap[type]->bit(j);

		PRINT_DEBUG("%u empty frames of size %u\n", free, frameSize(i));
	}

	PRINT_DEBUG("Checking the hash map for %s...\n",
		(type == KSEG) ? "KSEG" : "KUSEG");

	for (PageSize i = MIN_FRAME; i <= m_maxFrame; i = (PageSize)(i + 1)) {
		PRINT_DEBUG("%u empty frames of size %u\n", 
			freeFrames(i, type), frameSize(i));
	}

	for (PageSize i = MIN_FRAME; i <= m_maxFrame; i = (PageSize)(i + 1)) {
		uint free = 0;
		for (uint j = offset(i, type); j < offsetEnd(i, type); ++j)
			free += !m_bitmap[type]->bit(j);
		ASSERT(freeFrames(i, type) == free);
	}

	PRINT_DEBUG("DONE\n");
}

/*---------------------------------------------------------------------------*/

bool FrameAllocator::checkBitmaps() const
{
	bool ok = checkBitmap(KSEG);
	ok = ok && checkBitmap(KUSEG);
	ASSERT(ok);
	return ok;
}

/*---------------------------------------------------------------------------*/

bool FrameAllocator::checkBitmap( const AddressType type ) const
{
	bool ok = true;
	PageSize frame = m_maxFrame;
	// for all frames which have "children"
	for (uint i = 0; i < offset(MIN_FRAME, type); ++i)
	{
		// if we're on the next level
		if (i > offsetEnd(frame, type))
			// adjust the frame size
			frame = (PageSize)(frame - 1);

		// global offset of the first child
		uint first_child = offset(smallerFrame(frame), type)
			               + firstChild(i - offset(frame, type));
		bool used = m_bitmap[type]->bit(first_child);
		for (uint j = 1; j < FRAME_STEP; ++j)
			used = used || m_bitmap[type]->bit(first_child + j);

		// if the frame is used but none of its children is used
		if (m_bitmap[type]->bit(i) && !used) {
			PRINT_DEBUG("Parent set as used but all children not!\n");
			PRINT_DEBUG("\tFrame size: %u, local offset: %u\n",
				frameSize(frame), i - offset(frame, type));
			ok = false;
		// if the frame is free, but some of its children is used
		} else if (!m_bitmap[type]->bit(i) && used) {
			PRINT_DEBUG("Parent set as free but one or more children are used!\n");
			PRINT_DEBUG("\tFrame size: %u, local offset: %u\n",
				frameSize(frame), i - offset(frame, type));
			ok = false;
		}	// else it's ok, so continue
	}
	return ok;
}

/*---------------------------------------------------------------------------*/

#undef PRINT_DEBUG
