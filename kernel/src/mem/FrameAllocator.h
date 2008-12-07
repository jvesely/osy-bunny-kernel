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
 * @brief Contains both declaration and definition of class FrameAllocator.
 *
 * Class FrameAllocator provides interface for physical memory 
 * allocation. Supports up to 7 frame sizes and allocation of more subsequent
 * frames (all of them of same size).
 */

#pragma once

//#define FRALLOC_DEBUG

#ifndef FRALLOC_DEBUG
#define PRINT_DEBUG(...)
#else
#define PRINT_DEBUG(ARGS...) \
	printf("[ FRALLOC_DEBUG ]: "); \
	printf(ARGS);
#endif

#include "../structures/Bitset.h"
#include "../types.h"
#include "../cpp.h"
#include "../tools.h"
//#include "../api.h"
#include "../../../common/address.h"
#include "flags.h"
#include "InterruptDisabler.h"

/*!
 * @class FrameAllocator FrameAllocator.h "mem/FrameAllocator.h"
 * @brief Class FrameAllocator provides interface for physical memory 
 * allocation. Supports up to 7 frame sizes and allocation of more subsequent
 * frames (all of them of same size).
 *
 * Template class:
 * @param N Determines how many frame sizes will be supported. Should be an
 * integer from range 1..7. 
 *
 * @todo change init() function to distinguish between KSEG0 memory and other 
 *
 * @todo is combination of VF_VA_USER and VF_AT_KUSEG, etc. possible? 
 * if yes, change the allocateAtAddress() function
 * --SHOULD BE DONE--
 * (what if user wants to allocate on the border between KSEG and KUSEG?
 * 
 * @todo add function to check the whole bitmap if the used frames of 
 * different levels are properly set
 *
 * @todo setting parents as used may end before it reaches largest frames!!
 *
 * @todo !!IMPORTANT!! When setting frames as used, decrease count of free
 * frames only when the frame was free before.
 */
template <int N>
class FrameAllocator: public Singleton< FrameAllocator<N> >
{
public:

	/*!
	 * @brief Initializes Frame allocator structures. Must be called before 
	 * any alloc() or free() calls, otherwise the results are undefined.
	 *
	 * Determines largest frame size which can be used according to the total
	 * memory size. In case the memory size is not rounded to the largest frame
	 * size, the rest of the memory (from the end of the last largest frame)
	 * will be ignored.
	 *
	 * Creates and initializes Frame allocator structures and marks frames
	 * used by these structures and kernel as used.
	 *
	 * @return Address of the end of initially used frames. This address is 
	 * from the KSEG0 segment.
	 * 
	 * @todo (lowest priority) Add support for non-rouded RAM sizes
	 * (simply mark frames beyond the RAM size as used)
	 */
	uintptr_t init( size_t memory_size, const size_t kernel_end );

	/*!
	 * @brief Tries to find and allocate @a count frames of size @a frame_size 
	 * due to bit flags set in @a flags parameter.
	 *
	 * @param address Pointer to an integer where the <b>physicall address</b> 
	 * of the allocated block will be saved. If VF_VA_USER flag is set, 
	 * the function will try to allocate the memory on this address.
	 * @param count Count of subsequent frames requested by user to be allocated.
	 * @param frame_size Size of the frames requested by user to be allocated.
	 * @param flags Holds bit flags for influencing the function behaviour 
	 * (see below).
	 *
	 * Use bit flag @a VF_VIRT_ADDR to force FrameAllocator to allocate frames
	 * starting from address given by user in @a *address or to have it
	 * search for suitable position. Format of this flag is given by constants
	 * VF_VA_SIZE (size of the bit array) and VF_VA_SHIFT (position of the bit
	 * array in the @a flags word).
	 * Values: VF_VA_USER - Allocate at address given by user.\n
	 *         VF_VA_AUTO - Let FrameAllocator find the best place for the block.
	 *
	 * Bit flag VF_ADDR_TYPE may be used for telling FrameAllocator to try to
	 * allocate the block in specific memory segment. Format of this flag is 
	 * given by constants VF_AT_SIZE (size of the bit array) and VF_VA_SHIFT 
	 * (position of the bit array in the @a flags word).
	 * Values: VF_AT_KSEG0 - Allocate in physical address range to which
	 * KSEG0 segment is mapped.\n
	 *         VF_AT_KSEG1 - Dtto with KSEG1 segment.\n
	 *         VF_AT_KUSEG - Dtto with KUSEG segment.\n
	 *         VF_AT_KSSEG - Dtto with KSSEG segment.\n
	 *         VF_AT_KSEG3 - Dtto with KSEG3 segment.\n
	 *
	 * @retval @a count in case the allocation was successful. In this case
	 * @a *address holds physical address of start of the allocated block 
	 * of frames.
	 * @retval Count of largest block of subsequent free frames of size 
	 * @a frame_size in case the allocation was not successful. @a @address 
	 * will be the physical address of this block.
	 */
	uint frameAlloc( void** address, const size_t count, 
		const uint frame_size, const uint flags );

	uint allocateAtKseg0( 
		void** address, const uint count, const uint frame_size );

	/*!
	 * @note Not done!!!
	 */
	uint allocateAtKuseg( 
		void** address, const uint count, const uint frame_size );

	uint allocateAtAddress( 
		const void* address, const uint count, const uint frame_size );

	/*! 
	 * @brief Frees the memory previously allocated using FrameAllocator::alloc().
	 *
	 * @param address Physical address of the block of frames to be freed.
	 * @param count Count of subsequent frames to be freed.
	 * @param frame_size Size of the frames to be freed.
	 *
	 * @retval True if the deallocation was successful and the frames were freed.
	 * @retval False in case the address is not within the memory address range, 
	 * the address is not aligned at the start of a frame of size @a frame_size, 
	 * not all of the frames were allocated, or @a count is 0.
	 */
	bool frameFree( const void* address, const size_t count, 
		const uint frame_size );

	/*! @brief Returns the state of the frame allocator. Always use this function
	 * to check if FrameAllocator was initialized successfully, before calling
	 * alloc() or free().
	 */
	inline bool isInitialized() { return m_initialized; }

	inline void checkStructures() { printFreeKseg(); printFreeKuseg(); }

	// constants are set in such a way, that if flags == 0
	// then fralloc will find the place where to allocate and it will find it
	// in KSEG0 / KSEG1 segment

private:

	/*! @brief Size of the member hash table to be able to hash frame sizes
	 * without collisions.
	 */
	static const uint HASH_RANGE = 19;

	/*! @brief Minimum frame size given by the MIPS platform. */
	static const uint MIN_FRAME_SIZE = 4096;		// 4kB

	/*! @brief Maximum frame size given by the MIPS platform. */
	static const uint MAX_FRAME_SIZE = 16777216;	// 16MB

	/*! @brief Next frame size = FRAME_STEP * This frame size */
	static const uint FRAME_STEP = 4;

	static const uint KSEG0_SIZE = /*0x20000000;*/ 0x2000000;	// 32 MB
	
	enum AddressType { KERNEL, ALL };

	/*! @brief Default constructor. Only marks the FrameAllocator as 
	 * not initialized.
	 */
	FrameAllocator(): m_initialized(false) {};

	inline void setFramesAsFreeKseg( 
		const uint global_offset, const uint frame_size, const uint count );

	/*!
	 * @note Not done!!!
	 */
	inline void setFramesAsFreeKuseg( 
		const uint global_offset, const uint frame_size, const uint count );

	void setFramesAsUsedKseg( 
		const uint global_offset, const uint frame_size, const uint count);
	
	void setFramesAsUsedKuseg( 
		const uint global_offset, const uint frame_size, const uint count);

	uint defaultFrame( uint* search_counts, const uint frame_size,
		const AddressType type );

	void printFreeKseg() const;

	void printFreeKuseg() const;

	inline uint hash( const int key, const int range ) const;

	inline uint& offsetKseg( const uint frame_size );

	inline uint& offsetKuseg( const uint frame_size );

	inline uint& offsetKusegInAll( const uint frame_size );

	inline uint offsetOfChildKseg( 
		const uint local_offset, const uint frame_size );

	inline uint offsetOfChildKuseg( 
		const uint local_offset, const uint frame_size );

	inline uint offsetOfParentKseg( 
		const uint local_offset, const uint frame_size );

	inline uint offsetOfParentKuseg( 
		const uint local_offset, const uint frame_size );

	inline uint& freeFramesKseg( const uint frame_size );

	inline uint& freeFramesKuseg( const uint frame_size );

	inline uint emptyFramesKseg( const uint global_offset,
		const uint count, const uint frame_size );

	inline uint emptyFramesKuseg( const uint global_offset,
		const uint count, const uint frame_size );

	inline uint fullFramesKseg( 
		const uint global_offset, const uint count, const uint frame_size );

	inline uint fullFramesKuseg( 
		const uint global_offset, const uint count, const uint frame_size );

	inline uint min( const uint x, const uint y ) const;

	inline uint findFirstEmptyKseg( const uint offset ) const;

	inline uint findFirstEmptyKuseg( const uint offset ) const;

	inline uint findFirstFullKseg( const uint offset ) const;

	inline uint findFirstFullKuseg( const uint offset ) const;

	inline uint parent( const uint local_offset ) const;

	inline uint firstChild( const uint local_offset ) const;

	inline uint largerFrameSize( const uint frame_size ) const;

	inline uint smallerFrameSize( const uint frame_size ) const;

	inline uintptr_t getAddressKseg( 
		const uint off, const uint frame_size );

	inline uintptr_t getAddressKuseg( 
		const uint off, const uint frame_size );

	inline unsigned int alignUp(
		const unsigned int address, const unsigned int factor)
		{ return  (address + (factor - 1) ) & ~(factor - 1); };

	Bitset* m_buddyMapKseg;
	Bitset* m_buddyMapKuseg;

	/*! 
	 * Index meanings:
	 *	[hash(frame_size, 19), 0] ... count of free frames of size 
	 * @a frame_size,\n
	 *	[hash(frame_size, 19), 1] ... offset of the bitmap representing frames 
	 * of size @a frame_size.
	 */
	uint m_buddyInfoKseg[HASH_RANGE][2];

	/*! 
	 * Index meanings:
	 *	[hash(frame_size, 19), 0] ... count of free frames of size 
	 * @a frame_size,\n
	 *	[hash(frame_size, 19), 1] ... offset of the bitmap representing frames 
	 * of size @a frame_size,
	 *	[hash(frame_size, 19), 2] ... offset of the first frame of size 
	 * @a frame_size which lies in the KUSEG segment.
	 */
	uint m_buddyInfoKuseg[HASH_RANGE][3];
	
	uint m_maxFrameSize;
	uint m_theEndKseg;
	uint m_theEndKuseg;

	uintptr_t m_endOfBlockedFrames;
	bool m_initialized;


friend class Singleton< FrameAllocator<N> >;
};

/*----------------------------------------------------------------------------*/
/* DEFINITIONS ---------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

template <int N>
uintptr_t FrameAllocator<N>::init( size_t memory_size, const size_t kernel_end )
{
	InterruptDisabler interrupts;

	PRINT_DEBUG("Initializing Frame Allocator...\n");
	// frallocator needs the memory size and kernel size
	ASSERT(memory_size && kernel_end > ADDR_PREFIX_KSEG0);

	uint kernel_size = kernel_end - ADDR_PREFIX_KSEG0;

	PRINT_DEBUG("Memory size: %u\n", memory_size);
	PRINT_DEBUG("Kernel size: %u\n", kernel_size);
	
	// how many 4kB frames will the kernel take?
	uint used_frames = roundUp(kernel_size, MIN_FRAME_SIZE) / MIN_FRAME_SIZE;

	// the actual position where we will put our structures
	char* pos = (char*)roundUp(kernel_end, 0x4);
	
	PRINT_DEBUG("4 kB frames used for kernel: %u\n", used_frames);
	PRINT_DEBUG("Kernel ends on address %x\n", kernel_end);
	PRINT_DEBUG("Frame allocator structures will start on address %x\n", 
		        (uint)pos);

	/*--------------------------------------------------------------------------
	  find the largest frame size to know how much memory I can use
	--------------------------------------------------------------------------*/

	uint frame_size = MIN_FRAME_SIZE;

	// let's assume we have more than 4kB memory available
	// and that the KSEG0 segment's size is a multiple of the largest frame size
	ASSERT(memory_size > MIN_FRAME_SIZE);
	while (largerFrameSize(frame_size) <= memory_size 
		&& largerFrameSize(frame_size) <= MAX_FRAME_SIZE)
		frame_size *= FRAME_STEP;
	m_maxFrameSize = frame_size;

	PRINT_DEBUG("Largest frame size: %u kB\n", m_maxFrameSize / 1024);

	// if some of the memory will be beyond the last largest frame, 
	// we don't use it, it will be too complicated to handle
	if (memory_size % frame_size != 0) {
		PRINT_DEBUG("Memory size doesn't fit the largest frames, \
					so we won't use the rest of memory.\n");
		memory_size = memory_size - (memory_size % frame_size);
		PRINT_DEBUG("New memory size: %u\n", memory_size);
	}

	/*--------------------------------------------------------------------------
	  set the number of free frames and offset in the bitset of each frame size
	--------------------------------------------------------------------------*/
	
	// for not used frame sizes set these to 0
	for (frame_size = m_maxFrameSize;
	     frame_size <= MAX_FRAME_SIZE; 
		 frame_size *= 4) {

		freeFramesKseg(frame_size) = 0;
		freeFramesKuseg(frame_size) = 0;
		offsetKseg(frame_size) = 0;
		offsetKuseg(frame_size) = 0;
		offsetKusegInAll(frame_size) = 0;
	}

	// for used frame sizes, the largest frames will start on offset 0
	uint total_buddy_frames_kseg = 0;
	// we must separately count the offset to the KSEG0 and KUSEG maps
	uint total_buddy_frames_kuseg = 0;

	// as the size of the memory for KUSEG map, we can use the memory_size param
	// but for the size of the KSEG0 segment, we may need to use the size of 
	// KSEG0 segment
	uint kseg_size = min(memory_size, KSEG0_SIZE);
	uint kuseg_size = memory_size;

	for ( frame_size = m_maxFrameSize; 
	      frame_size >= MIN_FRAME_SIZE; 
		  frame_size /= FRAME_STEP) {

		// number of frames with size frame_size
		ASSERT(kseg_size % frame_size == 0);
		ASSERT(memory_size % frame_size == 0);		
		
		uint frames_kseg = kseg_size / frame_size;
		uint frames_kuseg = kuseg_size / frame_size;

		
		PRINT_DEBUG(
			"The KSEG map for frame size %u will have %u items at offset %u\n", 
			frame_size, frames_kseg, total_buddy_frames_kseg);
		PRINT_DEBUG(
			"The KUSEG map for frame size %u will have %u items at offset %u\n", 
			frame_size, frames_kuseg, total_buddy_frames_kuseg);
		PRINT_DEBUG(
			"The KUSEG map, KUSEG part for frame size %u will have %u items at offset %u\n", 
			frame_size, frames_kuseg - frames_kseg, total_buddy_frames_kuseg + frames_kseg);
		
		// set count of free frames on this level
		freeFramesKseg(frame_size) = frames_kseg;
		freeFramesKuseg(frame_size) = frames_kuseg;

		// set the offset of level-th map
		offsetKseg(frame_size) = total_buddy_frames_kseg;
		offsetKuseg(frame_size) = total_buddy_frames_kuseg;
		offsetKusegInAll(frame_size) = total_buddy_frames_kuseg + frames_kseg;

		total_buddy_frames_kseg += frames_kseg;
		total_buddy_frames_kuseg += frames_kuseg;
	}
	// keep the maximum offset as there is no other ofset to determine where
	// the bitmap for the smallest frames ends
	m_theEndKseg = total_buddy_frames_kseg;
	m_theEndKuseg = total_buddy_frames_kuseg;
	offsetKseg(frame_size) = total_buddy_frames_kseg;
	offsetKuseg(frame_size) = total_buddy_frames_kuseg;
	offsetKusegInAll(frame_size) = total_buddy_frames_kuseg;

	PRINT_DEBUG("The whole KSEG buddy map will have %u items\n", 
		total_buddy_frames_kseg);
	PRINT_DEBUG("The whole KUSEG buddy map will have %u items\n", 
		total_buddy_frames_kuseg);

	/*--------------------------------------------------------------------------
	  create and initialize bitmaps (for KSEG0/1 and other memory)
	--------------------------------------------------------------------------*/
	void* position = static_cast<void*>(pos);
	m_buddyMapKseg = static_cast<Bitset*>(new (position) 
	                     Bitset(pos + sizeof(Bitset), total_buddy_frames_kseg));
	// after the casts, it's guaranteed to be a valid pointer
	pos += (sizeof(Bitset) + Bitset::getContainerSize(total_buddy_frames_kseg));

	position = static_cast<void*>(pos);
	m_buddyMapKuseg = static_cast<Bitset*>(new (position) 
	                  Bitset(pos + sizeof(Bitset), total_buddy_frames_kuseg));
	pos += (sizeof(Bitset) + Bitset::getContainerSize(total_buddy_frames_kuseg));

	// pos points to the end of my bitset structures
	PRINT_DEBUG("Frame allocator structures end at address %x\n", pos);
	
	ASSERT( ( kernel_end + 2 * sizeof(Bitset)
	        + Bitset::getContainerSize(total_buddy_frames_kseg)
			+ Bitset::getContainerSize(total_buddy_frames_kuseg) )
			== (uintptr_t)pos );

	// determine the count of frames used by kernel and my structures
	used_frames = roundUp(((uint)pos - ADDR_PREFIX_KSEG0), MIN_FRAME_SIZE) 
	                        / MIN_FRAME_SIZE;

	/*while ( ((uint)pos - ADDR_PREFIX_KSEG0) > (used_frames * MIN_FRAME_SIZE) )
		++used_frames;*/
	PRINT_DEBUG("4 kB frames used for kernel and frame allocator: \
		         %u\n", used_frames);

#ifdef FRALLOC_DEBUG
	// check the offsets
	PRINT_DEBUG("Frame sizes to offsets mapping, KSEG:\n");
	for (uint i = MIN_FRAME_SIZE; i <= m_maxFrameSize; i *= FRAME_STEP) {
		PRINT_DEBUG("Frame size: %u, offset: %u\n", 
			i, m_buddyInfoKseg[hash(i, HASH_RANGE)][1]);
	}
	PRINT_DEBUG("Frame sizes to offsets mapping, KUSEG:\n");
	for (uint i = MIN_FRAME_SIZE; i <= m_maxFrameSize; i *= FRAME_STEP) {
		PRINT_DEBUG("Frame size: %u, offset: %u\n", 
			i, m_buddyInfoKuseg[hash(i, HASH_RANGE)][1]);
	}
	PRINT_DEBUG("Frame sizes to offsets mapping, KUSEG, part KUSEG:\n");
	for (uint i = MIN_FRAME_SIZE; i <= m_maxFrameSize; i *= FRAME_STEP) {
		PRINT_DEBUG("Frame size: %u, offset: %u\n", 
			i, m_buddyInfoKuseg[hash(i, HASH_RANGE)][2]);
	}

	printFreeKseg();
	printFreeKuseg();
#endif

	PRINT_DEBUG("Setting frames occupied by kernel and frame allocator as used\n");

	// set all used frames as used in the bitmaps
	frame_size = MIN_FRAME_SIZE;
	setFramesAsUsedKseg(offsetKseg(frame_size), frame_size, used_frames);
	//setFramesAsUsedKuseg(offsetKuseg(frame_size), frame_size, used_frames);

	PRINT_DEBUG("DONE\n");

	/*
	 TODO: add check for consistent used frames in whole bitmaps
	*/

#ifdef FRALLOC_DEBUG
	printFreeKseg();
	printFreeKuseg();
#endif

	// should the end of the structures be rounded up to 4kB??
	//pos = (char*)roundUp((uint)pos, MIN_FRAME_SIZE);

	m_endOfBlockedFrames = (uintptr_t)pos - ADDR_PREFIX_KSEG0;
	m_initialized = true;
	PRINT_DEBUG("Frame Allocator initialized. Returning: %x\n", (uint)pos);
	//Kernel::instance().halt();
	return (uintptr_t)pos;
}

/*----------------------------------------------------------------------------*/

template <int N>
uint FrameAllocator<N>::frameAlloc( 
	void** address, const size_t count, const uint frame_size, 
	const uint flags )
{
	PRINT_DEBUG("FrameAllocator::alloc() started...\n\n");

	PRINT_DEBUG("Address: %x, Count: %u, Frame size: %u Flags: %x\n",
		*address, count, frame_size, flags);

	ASSERT(address && count);

	PRINT_DEBUG("VF_VIRT_ADDR: %x\n", flags & VF_VA_MASK);

	PRINT_DEBUG("Checking VF_VIRT_ADDR bit sign...\n");

#ifdef FRALLOC_DEBUG

	switch (flags & VF_VA_MASK) {
		case VF_VA_AUTO << VF_VA_SHIFT:
			PRINT_DEBUG("VF_VIRT_ADDR == VF_VA_AUTO\n");
			break;
		case VF_VA_USER << VF_VA_SHIFT:
			PRINT_DEBUG("VF_VIRT_ADDR == VF_VA_USER\n");
			break;
		default:
			PRINT_DEBUG("Wrong format of VF_VIRT_ADDR!!\n");
			ASSERT(false);
			return 0;
			break;
	}

	PRINT_DEBUG("VF_ADDR_TYPE: %x\n", flags & VF_AT_MASK);

	PRINT_DEBUG("Checking VF_ADDR_TYPE bit sign...\n");
	switch (flags & VF_AT_MASK) {
		case VF_AT_KSEG0 << VF_AT_SHIFT:
			PRINT_DEBUG("VF_ADDR_TYPE == VF_AT_KSEG0\n");
			break;
		case VF_AT_KSEG1 << VF_AT_SHIFT:
			PRINT_DEBUG("VF_ADDR_TYPE == VF_AT_KSEG1\n");
			break;
		case VF_AT_KUSEG << VF_AT_SHIFT:
			PRINT_DEBUG("VF_ADDR_TYPE == VF_AT_KUSEG\n");
			break;
		case VF_AT_KSSEG << VF_AT_SHIFT:
			PRINT_DEBUG("VF_ADDR_TYPE == VF_AT_KSSEG\n");
			break;
		case VF_AT_KSEG3 << VF_AT_SHIFT:
			PRINT_DEBUG("VF_ADDR_TYPE == VF_AT_KSEG3\n");
			break;
		default:
			PRINT_DEBUG("Wrong format of VF_ADDR_TYPE!!\n");
			ASSERT(false);
			return 0;
			break;
	}

#endif

	// TODO: replace flag checks by inline functions
	if ((flags & VF_VA_MASK) == (VF_VA_AUTO << VF_VA_SHIFT)) {
		if ( ((flags & VF_AT_MASK) == (VF_AT_KSEG0 << VF_AT_SHIFT)) 
			|| ((flags & VF_AT_MASK) == (VF_AT_KSEG1 << VF_AT_SHIFT)) ) {
			return allocateAtKseg0( address, count, frame_size );
		} else {
			ASSERT( (flags & VF_AT_MASK) == (VF_AT_KUSEG << VF_AT_SHIFT)
				|| (flags & VF_AT_MASK) == (VF_AT_KSSEG << VF_AT_SHIFT)
				|| (flags & VF_AT_MASK) == (VF_AT_KSEG3 << VF_AT_SHIFT) );
			return allocateAtKuseg( address, count, frame_size );
		}			
	} else {
		ASSERT((flags & VF_VA_MASK) == (VF_VA_USER << VF_VA_SHIFT));
		return allocateAtAddress(*address, count, frame_size );
	}

	PRINT_DEBUG("FrameAllocator::alloc() ends...\n\n");
	return 0;	
}

/*---------------------------------------------------------------------------*/

template <int N>
inline void FrameAllocator<N>::setFramesAsFreeKseg( 
	const uint global_offset, const uint frame_size, const uint count )
{
	/*--------------------------------------------------------------------------
	  Mark given frames as free in the KSEG and KUSEG maps
	--------------------------------------------------------------------------*/
	
	PRINT_DEBUG("Setting %u frames of size %u as free in maps KSEG and KUSEG\n", 
			count, frame_size);

	uint local_offset = global_offset - offsetKseg(frame_size);

	// set the given frames as free in Kseg map
	m_buddyMapKseg->bits(global_offset, count, false);
	// increase the count of free frames saved
	freeFramesKseg(frame_size) += count;
	// determine the offset of the same frames in Kseg map
	uint offset_in_kuseg = offsetKuseg(frame_size) + local_offset;
	// and mark them as well
	m_buddyMapKuseg->bits(offset_in_kuseg, count, false);
	// increase the count of free frames saved
	freeFramesKuseg(frame_size) += count;

	/*--------------------------------------------------------------------------
	  Mark all their "children" and "children of their children", etc.
	--------------------------------------------------------------------------*/

	PRINT_DEBUG("Setting their children free...\n");

	uint fr_size = frame_size;
	uint cnt = count;
	while (fr_size > MIN_FRAME_SIZE) {
		cnt *= 4;
		PRINT_DEBUG("Setting %u frames of size %u as free in KSEG and KUSEG\n", 
			cnt, smallerFrameSize(fr_size));

		m_buddyMapKseg->bits(offsetOfChildKseg(local_offset, fr_size), 
			cnt, false);
		m_buddyMapKuseg->bits(offsetOfChildKuseg(local_offset, fr_size), 
			cnt, false);

		fr_size /= FRAME_STEP;

		ASSERT(fr_size >= MIN_FRAME_SIZE);

		freeFramesKseg(fr_size) += cnt;
		freeFramesKuseg(fr_size) += cnt;

		local_offset *= 4;
	}

	/*--------------------------------------------------------------------------
	  Mark all their "parents" and "parents of their parents", etc.
	  This case is different from marking them as used
	  In parent we must check if all of his children are free, but it's
	  sufficient to check only the first and the last parent
	--------------------------------------------------------------------------*/

	PRINT_DEBUG("Setting their parents free...\n\n");

	local_offset = global_offset - offsetKseg(frame_size);
	fr_size = frame_size;
	cnt = count;
	bool freed = true;
	// if at least one of the parents was freed in this loop
	// we have to check their parents in the next loop
	while (fr_size < m_maxFrameSize && freed) {
		
		freed = false;

		PRINT_DEBUG("Setting parents with size %u\n", largerFrameSize(fr_size));

		/*
		 * Determine the number of affected parents
		 */

		// relative offset of the first parent's first child
		uint child_first = roundDown(local_offset, FRAME_STEP);
		PRINT_DEBUG("First child of the first parent, offset: %u\n", 
			child_first);
		// relative offset of the first parent
		uint first_parent = child_first / FRAME_STEP;
		//uint first_parent = parent(local_offset);
		PRINT_DEBUG("First parent, offset: %u\n", first_parent);
		
		ASSERT(offsetOfParentKseg(local_offset, fr_size) 
			== offsetKseg(largerFrameSize(fr_size)) + first_parent);
		ASSERT(offsetOfParentKseg(child_first, fr_size) 
			== offsetKseg(fr_size * FRAME_STEP) + first_parent);

		// relative offset of the last parent's first child
		uint child_last = roundDown(local_offset + cnt - 1, FRAME_STEP);
		PRINT_DEBUG("First child of the last parent, offset: %u\n", child_last);
		// relative offset of the last parent
		uint last_parent = child_last / FRAME_STEP;
		//uint last_parent = parent(local_offset + cnt - 1);
		PRINT_DEBUG("Last parent, offset: %u\n", last_parent);

		ASSERT(offsetOfParentKseg(local_offset + cnt - 1, fr_size) 
		        == offsetKseg(largerFrameSize(fr_size)) + last_parent);
		ASSERT(offsetOfParentKseg(child_last, fr_size) 
		        == offsetKseg(fr_size * FRAME_STEP) + last_parent);

		// number of parents
		cnt = last_parent - first_parent + 1;
		PRINT_DEBUG("Number of parents: %u\n", cnt);

		/*
		 * Check the first parent and eventually set it as free
		 */

		PRINT_DEBUG("Checking 1st parent's children...\n");

		bool used = m_buddyMapKseg->bit(offsetKseg(fr_size) + child_first);
		PRINT_DEBUG("1st parent's first child is %s\n", (used) ? "used" : "free");
		// check all other children until there is at least one used
		for (uint i = 1; i < FRAME_STEP && !used; ++i) {
			used = used || m_buddyMapKseg->bit(
				offsetKseg(fr_size) + child_first + i);
			PRINT_DEBUG("1st parent's %u-th child is %s\n", 
				i + 1,
				(m_buddyMapKseg->bit(offsetKseg(fr_size) + child_first + i)) 
				? "used" 
				: "free");
		}

		// TODO: this can be optimized
		// we don't need to mark parent, which is used, as it was used before

		PRINT_DEBUG("Setting 1st parent as %s\n", (used) ? "used" : "free" );
		m_buddyMapKseg->bit(
			offsetKseg(largerFrameSize(fr_size)) + first_parent, used);
		m_buddyMapKuseg->bit(
			offsetKuseg(largerFrameSize(fr_size)) + first_parent, used);
		if (!used) {
			++freeFramesKseg(largerFrameSize(fr_size));
			++freeFramesKuseg(largerFrameSize(fr_size));
			freed = true;
		}

		// if there is only one parent, continue the loop
		// but adjust the variables at the end of the loop

		/*
		 * Check the last parent and eventually set it as free
		 */

		if (cnt > 1) {
			// there are at least 2 parents
			PRINT_DEBUG("Checking last parent's children...\n");

			used = m_buddyMapKseg->bit(offsetKseg(fr_size) + child_last);
			PRINT_DEBUG("Last parent's first child is %s\n", 
				(used) ? "used" : "free");

			for (uint i = 1; i < FRAME_STEP && !used; ++i) {
				used = used || m_buddyMapKseg->bit(
					offsetKseg(fr_size) + child_last + i);
				PRINT_DEBUG("Last parent's %u-th child is %s\n", 
					i + 1,
					(m_buddyMapKseg->bit(offsetKseg(fr_size) + child_last + i)) 
					? "used" 
					: "free");
			}

			PRINT_DEBUG("Setting last parent as %s\n", (used) ? "used" : "free" );
			m_buddyMapKseg->bit(
				offsetOfParentKseg(child_last, fr_size), used);
			m_buddyMapKuseg->bit(
				offsetOfParentKuseg(child_last, fr_size), used);
			if (!used) {
				++freeFramesKseg(largerFrameSize(fr_size));
				++freeFramesKuseg(largerFrameSize(fr_size));
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
			m_buddyMapKseg->bits(
				offsetOfParentKseg(local_offset, fr_size) + 1, cnt - 2, false);
			m_buddyMapKuseg->bits(
				offsetOfParentKuseg(local_offset, fr_size) + 1, cnt - 2, false);
			freeFramesKseg(largerFrameSize(fr_size)) += cnt - 2;
			freeFramesKuseg(largerFrameSize(fr_size)) += cnt - 2;
			freed = true;
		}

		fr_size *= FRAME_STEP;
		local_offset /= 4;

		PRINT_DEBUG("\n");
	}
}

/*---------------------------------------------------------------------------*/

template <int N>
inline void FrameAllocator<N>::setFramesAsFreeKuseg( 
	const uint global_offset, const uint frame_size, const uint count )
{
	// TODO: replace with separate function
	setFramesAsFreeKseg(global_offset, frame_size, count);
}

/*----------------------------------------------------------------------------*/

template <int N>
bool FrameAllocator<N>::frameFree( 
	const void* address, const size_t count, const uint frame_size )
{
	InterruptDisabler interrupts;

	PRINT_DEBUG("FrameAllocator::free() started...\n");
	PRINT_DEBUG("Request to free %u frames of size %u, starting from address %x\n", count, frame_size, address);

	uintptr_t addr = (uintptr_t)address;
	// don't allow freeing the kernel or my memory
	if (addr < m_endOfBlockedFrames) {
		PRINT_DEBUG("Address points to blocked memory!\n");
		return false;
	}

	// check if the address is aligned at the frame start
	if (addr % frame_size != 0) {
		PRINT_DEBUG("Address is not alligned!\n");
		return false;
	}
	
	PRINT_DEBUG("Checking if there is enough space allocated...\n");
	// else check if there are enough used frames
	uint local_offset = addr / frame_size;
	PRINT_DEBUG("The address points to a frame of size %u with offset %u\n", frame_size, local_offset);
	
	uint used = fullFramesKuseg(
		offsetKuseg(frame_size) + local_offset, count, frame_size);
	PRINT_DEBUG("Found %u used frames\n", used);

	if (used >= count) {// this check will change when full has "enough" param.
		// there are enough allocated frames, so free them
		setFramesAsFreeKseg(
			offsetKseg(frame_size) + local_offset, frame_size, count);
		/*setFramesAsFreeKuseg(
			offsetKuseg(frame_size) + local_offset, frame_size, count);*/
		return true;
	}

	/*
	 * What if there are enough used frames of this size, but not all of the
	 * corresponding smallest frames are used??? Should I free the frames? Or 
	 * return an error?
	 */

	PRINT_DEBUG("Not enough allocated frames!\n");
	PRINT_DEBUG("FrameAllocator::free() ends...\n");
	return false;
}

/*----------------------------------------------------------------------------*/

template <int N>
uint FrameAllocator<N>::defaultFrame( uint* search_counts, 
	const uint frame_size, const AddressType type )
{
	uint fr_size = frame_size;
	uint count = search_counts[hash(frame_size, HASH_RANGE)];
	while (count > 1 && fr_size < m_maxFrameSize 
		&& ((type == KERNEL) 
		    ? freeFramesKseg(fr_size) 
			: freeFramesKuseg(fr_size))   > count) {

		// save search_count for this frame_size
		search_counts[hash(fr_size, HASH_RANGE)] = count;
		fr_size *= FRAME_STEP;
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
	if ( (((type == KERNEL) 
		? freeFramesKseg(fr_size) 
		: freeFramesKuseg(fr_size))   < count) && fr_size < frame_size) {
		fr_size /= FRAME_STEP;
	} else {
		// save the last search_count
		search_counts[hash(fr_size, HASH_RANGE)] = count;
	}
	return fr_size;
}

/*----------------------------------------------------------------------------*/

template <int N>
void FrameAllocator<N>::setFramesAsUsedKseg( 
	const uint global_offset, const uint frame_size, const uint count )
{
	/*--------------------------------------------------------------------------
	  Mark given frames as used in the KSEG and KUSEG maps
	--------------------------------------------------------------------------*/

	PRINT_DEBUG("Setting %u frames of size %u as used in maps KSEG and KUSEG\n", 
			count, frame_size);

	uint local_offset = global_offset - offsetKseg(frame_size);
	PRINT_DEBUG("Starting from %u-th frame of size %u\n", local_offset, 
		frame_size);
	
	// set the given frames as used in Kseg map
	m_buddyMapKseg->bits(global_offset, count, true);
	// decrease the count of free frames saved
	freeFramesKseg(frame_size) -= count;
	// determine the offset of the same frames in Kuseg map
	uint offset_in_kuseg = offsetKuseg(frame_size) + local_offset;
	// and mark them as well
	m_buddyMapKuseg->bits(offset_in_kuseg, count, true);
	// decrease the count of free frames saved
	freeFramesKuseg(frame_size) -= count;

	/*--------------------------------------------------------------------------
	  Mark all their "children" and "children of their children", etc.
	--------------------------------------------------------------------------*/

	PRINT_DEBUG("Setting their children as used..\n");

	uint fr_size = frame_size;
	uint cnt = count;
	while (fr_size > MIN_FRAME_SIZE) {
		cnt *= 4;
		PRINT_DEBUG("Setting %u frames of size %u as used in KSEG and KUSEG\n", 
			cnt, smallerFrameSize(fr_size));

		m_buddyMapKseg->bits(offsetOfChildKseg(local_offset, fr_size), 
			cnt, true);
		m_buddyMapKuseg->bits(offsetOfChildKuseg(local_offset, fr_size), 
			cnt, true);

		fr_size /= FRAME_STEP;

		ASSERT(fr_size >= MIN_FRAME_SIZE);

		freeFramesKseg(fr_size) -= cnt;
		freeFramesKuseg(fr_size) -= cnt;

		local_offset *= 4;
	}

	/*--------------------------------------------------------------------------
	  Mark all their "parents" and "parents of their parents", etc.
	--------------------------------------------------------------------------*/

	PRINT_DEBUG("Setting their parents as used..\n");

	local_offset = global_offset - offsetKseg(frame_size);
	fr_size = frame_size;
	cnt = count;
	while (fr_size < m_maxFrameSize) {

		/*
		 * Determine the number of affected parents
		 */

		uint first_parent = parent(local_offset);
		PRINT_DEBUG("First parent, offset: %u\n", first_parent);
		
		ASSERT(offsetOfParentKseg(local_offset, fr_size) 
			== offsetKseg(largerFrameSize(fr_size)) + first_parent);

		// relative offset of the last parent
		uint last_parent = parent(local_offset + cnt - 1);
		PRINT_DEBUG("Last parent, offset: %u\n", last_parent);

		ASSERT(offsetOfParentKseg(local_offset + cnt - 1, fr_size) 
		        == offsetKseg(largerFrameSize(fr_size)) + last_parent);

		// number of parents
		cnt = last_parent - first_parent + 1;

		/*
		 * Set the parents as used
		 */

		fr_size *= FRAME_STEP;

		PRINT_DEBUG("Setting %u frames of size %u as used\n", 
			cnt, fr_size);

		ASSERT(fr_size <= m_maxFrameSize);

		// the first and the last parents could have not been used
		// but they should have been the same in Kseg and Kuseg
		/*printf("First parent's offset in KSEG map: %u\n", 
			offsetKseg(fr_size) + first_parent);
		printf("First parent's offset in KSEG map: %u\n", 
			offsetKuseg(fr_size) + first_parent);
		printf("Last offset of KSEG map: %u\n", m_theEndKseg);
		printf("Last offset of KUSEG map: %u\n", m_theEndKuseg);*/
		ASSERT(m_buddyMapKseg->bit(offsetKseg(fr_size) + first_parent)
			   == m_buddyMapKuseg->bit(offsetKuseg(fr_size) + first_parent)
			   );
		/*PRINT_DEBUG("Last parent:\n");
		PRINT_DEBUG("\tIndex in KSEG map: %u:\n", 
			offsetKseg(fr_size) + last_parent);
		PRINT_DEBUG("\tLast parent:\n");
		PRINT_DEBUG();*/
		/*printf("Last parent's offset in KSEG map: %u\n", 
			offsetKseg(fr_size) + last_parent);
		printf("Last parent's offset in KSEG map: %u\n", 
			offsetKuseg(fr_size) + last_parent);*/
		ASSERT(m_buddyMapKseg->bit(offsetKseg(fr_size) + last_parent)
			   == m_buddyMapKuseg->bit(offsetKuseg(fr_size) + last_parent)
			   );

		uint new_used = cnt;

		if (new_used > 0 && 
			m_buddyMapKseg->bit(offsetKseg(fr_size) + first_parent))
			--new_used;
		if (new_used > 0 && 
			last_parent != first_parent && 
			m_buddyMapKseg->bit(offsetKseg(fr_size) + last_parent))
			--new_used;

		PRINT_DEBUG("Setting %u frames from offset %u as used in KSEG map\n",
			cnt, offsetKseg(fr_size) + first_parent);
		m_buddyMapKseg->bits(
			offsetKseg(fr_size) + first_parent, cnt, true);
		PRINT_DEBUG("Setting %u frames from offset %u as used in KUSEG map\n",
			cnt, offsetKuseg(fr_size) + first_parent);
		m_buddyMapKuseg->bits(
			offsetKuseg(fr_size) + first_parent, cnt, true);
		
//#ifdef FRALLOC_DEBUG
		for (uint i = 0; i < cnt; ++i) {
			ASSERT(m_buddyMapKseg->bit(offsetKseg(fr_size) + first_parent + i)
			   == m_buddyMapKuseg->bit(offsetKuseg(fr_size) + first_parent + i)
			   );
		}
//#endif

		

		freeFramesKseg(fr_size) -= new_used;
		freeFramesKuseg(fr_size) -= new_used;

		local_offset /= 4;
	}
}

/*----------------------------------------------------------------------------*/

template <int N>
void FrameAllocator<N>::setFramesAsUsedKuseg( 
	const uint global_offset, const uint frame_size, const uint count )
{
	/*--------------------------------------------------------------------------
	  Mark given frames as used in the KUSEG map and if some of them are also
	  in KSEG, mark them also in KSEG map

	  TODO: reduce the count of free frames in the KUSEG part of KUSEG map
	  (count - count_kseg)
	--------------------------------------------------------------------------*/

	PRINT_DEBUG("Setting %u frames of size %u as used in map KUSEG\n", 
			count, frame_size);

	uint local_offset = global_offset - offsetKuseg(frame_size);
	PRINT_DEBUG("Starting from %u-th frame of size %u\n", local_offset, 
		frame_size);

	// set the given frames as used in KSEG map
	m_buddyMapKuseg->bits(global_offset, count, true);
	// decrease the count of free frames saved
	freeFramesKuseg(frame_size) -= count;

	// see if any of the frames are in KSEG
	// TODO: beware, this can be also < 0! can't use it for determining
	// the count of free frames in KUSEG segment
	int count_kseg = min( count, offsetKseg(smallerFrameSize(frame_size)) 
									/* end of frames with size frame_size */
	                          - (offsetKseg(frame_size) + local_offset) 
									/* global offset of the first frame */ );
	if (count_kseg > 0) {
		// set the given frames as used in KSEG map
		PRINT_DEBUG("Setting %u frames of size %u as used in map KSEG\n", 
			count_kseg, frame_size);
		m_buddyMapKseg->bits(
			offsetKseg(frame_size) + local_offset, count_kseg, true);
		// decrease the count of free frames saved
		freeFramesKseg(frame_size) -= count_kseg;
	}

	/*--------------------------------------------------------------------------
	  Mark all their "children" and "children of their children", etc.
	--------------------------------------------------------------------------*/

	PRINT_DEBUG("Setting their children as used..\n");

	uint fr_size = frame_size;
	uint cnt = count;
	uint cnt_kseg = count_kseg;
	while (fr_size > MIN_FRAME_SIZE) {
		cnt *= 4;
		cnt_kseg *= 4;	// if it was 0, it will remain 0

		PRINT_DEBUG("Setting %u frames of size %u as used in KUSEG\n", 
			cnt, smallerFrameSize(fr_size));
		
		m_buddyMapKuseg->bits(
			offsetOfChildKuseg(local_offset, fr_size), cnt, true);

		// if any of the children are in KSEG
		if (cnt_kseg > 0) {
			PRINT_DEBUG("Setting %u frames of size %u as used in KSEG\n", 
			cnt_kseg, smallerFrameSize(fr_size));

			m_buddyMapKseg->bits(
				offsetOfChildKseg(local_offset, fr_size), cnt_kseg, true);
			freeFramesKseg(smallerFrameSize(fr_size)) -= cnt_kseg;
		}

		fr_size /= FRAME_STEP;
		
		ASSERT(fr_size >= MIN_FRAME_SIZE);
		freeFramesKuseg(fr_size) -= cnt;
		
		local_offset *= 4;
	}

	/*--------------------------------------------------------------------------
	  Mark all their "parents" and "parents of their parents", etc.
	--------------------------------------------------------------------------*/

	PRINT_DEBUG("Setting their parents as used in KUSEG and KSEG..\n");

	local_offset = global_offset - offsetKuseg(frame_size);
	fr_size = frame_size;
	cnt = count;
	cnt_kseg = count_kseg;
	while (fr_size < m_maxFrameSize) {

		/*
		 * Parents in KUSEG
		 */

		PRINT_DEBUG("KUSEG...\n");

		/*
		 * Determine the number of affected parents
		 */

		uint first_parent = parent(local_offset);
		PRINT_DEBUG("First parent, offset: %u\n", first_parent);
		
		ASSERT(offsetOfParentKuseg(local_offset, fr_size) 
			== offsetKuseg(largerFrameSize(fr_size)) + first_parent);

		// relative offset of the last parent
		uint last_parent = parent(local_offset + cnt - 1);
		PRINT_DEBUG("Last parent, offset: %u\n", last_parent);

		ASSERT(offsetOfParentKuseg(local_offset + cnt - 1, fr_size) 
		        == offsetKuseg(largerFrameSize(fr_size)) + last_parent);

		// number of parents
		cnt = last_parent - first_parent + 1;

		/*
		 * Set the parents as used
		 */

		PRINT_DEBUG("Setting %u frames of size %u as used in KUSEG\n", 
			cnt, largerFrameSize(fr_size));

		ASSERT(fr_size < m_maxFrameSize);
		// the first and the last parents could have not been used
		// but they should have been the same in Kseg and Kuseg
		uint new_used = cnt;

		if (new_used > 0 && 
			m_buddyMapKuseg->bit(
				offsetKuseg(largerFrameSize(fr_size)) + first_parent))
			--new_used;
		if (new_used > 0
			&& first_parent != last_parent 
			&& m_buddyMapKuseg->bit(
				offsetKuseg(largerFrameSize(fr_size)) + last_parent))
			--new_used;

		m_buddyMapKuseg->bits(
			offsetOfParentKuseg(local_offset, fr_size), cnt, true);

		freeFramesKuseg(largerFrameSize(fr_size)) -= new_used;

		/*
		 * Parents in KSEG
		 */

		if (cnt_kseg > 0) {

			PRINT_DEBUG("KSEG...\n");

			/*
			 * Determine the number of affected parents
			 */

			uint first_parent_kseg = parent(local_offset);
			PRINT_DEBUG("First parent, offset: %u\n", first_parent_kseg);
			
			ASSERT(offsetOfParentKseg(local_offset, fr_size) 
				== offsetKseg(largerFrameSize(fr_size)) + first_parent_kseg);

			// relative offset of the last parent
			uint last_parent_kseg = parent(local_offset + cnt_kseg - 1);
			PRINT_DEBUG("Last parent, offset: %u\n", last_parent_kseg);

			ASSERT(offsetOfParentKseg(local_offset + cnt_kseg - 1, fr_size) 
					== offsetKseg(largerFrameSize(fr_size)) + last_parent_kseg);

			// number of parents
			cnt_kseg = last_parent_kseg - first_parent_kseg + 1;

			/*
			 * Set the parents as used
			 */

			PRINT_DEBUG("Setting %u frames of size %u as used in KSEG\n", 
				cnt_kseg, largerFrameSize(fr_size));

			ASSERT(fr_size < m_maxFrameSize);
			// the first and the last parents could have not been used
			// but they should have been the same in Kseg and Kuseg
			new_used = cnt_kseg;

			if (new_used > 0 
				&& m_buddyMapKseg->bit(
					offsetKseg(largerFrameSize(fr_size)) + first_parent_kseg))
				--new_used;
			if (new_used > 0 
				&& first_parent_kseg != last_parent_kseg
				&& m_buddyMapKseg->bit(
					offsetKseg(largerFrameSize(fr_size)) + last_parent_kseg))
				--new_used;

			m_buddyMapKseg->bits(
				offsetOfParentKseg(local_offset, fr_size), cnt_kseg, true);

			freeFramesKseg(largerFrameSize(fr_size)) -= new_used;
		}

		fr_size *= FRAME_STEP;
		local_offset /= 4;
	}
}

/*----------------------------------------------------------------------------*/

template <int N>
uint FrameAllocator<N>::allocateAtKseg0( 
	void** address, const uint count, const uint frame_size )
{
	InterruptDisabler interrupts;

	PRINT_DEBUG("allocateAtKseg0() started...\n");

	ASSERT(frame_size >= MIN_FRAME_SIZE);

	uint search_counts[HASH_RANGE];
	search_counts[hash(frame_size, HASH_RANGE)] = count;

	// determine the size of frame(s) we'll try to find
	uint search_size = defaultFrame(search_counts, frame_size, KERNEL);
	ASSERT(search_size >= frame_size);
	
	uint search_count = search_counts[hash(search_size, HASH_RANGE)];
	uint search_start = offsetKseg(search_size);
	
	PRINT_DEBUG("Starting to search for %u frames of size %u, from offset %u\n",
		search_count, search_size, search_start);

	// here we will store the maximum number of subsequent empty frames of 
	// the size we want
	uint max_empty = 0;

	// loop until we are searching for frames large enough or 
	// until we're on the end of the bitmap
	// (just to be sure we don't use memory which isn't ours
	while (search_size >= frame_size && search_start < m_theEndKseg - 1) {
		
		ASSERT(search_size >= MIN_FRAME_SIZE);
		
		PRINT_DEBUG("Searching for the first empty frame...\n");
		// find offset of the first empty frame
		search_start = findFirstEmptyKseg(search_start);
		PRINT_DEBUG("First empty frame found on global offset %u\n", 
			search_start);
		
		// we're beyond the end of interesting frames' map
		if (search_start >= offsetKseg(smallerFrameSize(frame_size))) {
			PRINT_DEBUG("We're beyond the bitmap of the required frame size, stopping the loop...\n");
			break;
		}

		ASSERT(search_size >= MIN_FRAME_SIZE);
		
		// we're beyond the end of the actually sought frame size's bitmap
		if (search_start >= offsetKseg(search_size / FRAME_STEP)) {
			PRINT_DEBUG("We're beyond the end of the actually sought frame size's bitmap\n");
			PRINT_DEBUG("Search start: %u\n", search_start);
			PRINT_DEBUG("Global offset of end of these frame size: %d\n", 
				offsetKseg(search_size / FRAME_STEP));

			PRINT_DEBUG("Determining the new search size...\n");
			// determine what size is this frame and set proper search_count
			while (search_start >= offsetKseg(search_size / FRAME_STEP)) {
				search_size /= FRAME_STEP;
				PRINT_DEBUG("Trying search size %d\n", search_size);
				PRINT_DEBUG("Global offset of this size's bitmap: %d\n",
					offsetKseg(search_size));
			}
			search_count = search_counts[hash(search_size, HASH_RANGE)];
			PRINT_DEBUG("New search size: %u\n", search_size);
			PRINT_DEBUG("New search count: %u\n", search_count);

			// if the found empty frame is of smaller size than user wanted
			if (search_size < frame_size) {
				// should not happen, because this option was already handled
				PRINT_DEBUG("We're beyond the bitmap of the required frame size, stopping the loop...\n");
				break;
			}

			// else we can continue executing commands in this loop
		}

		// it's still the frame size we want
		ASSERT(search_start < offsetKseg(search_size / FRAME_STEP));
			
		// try if there are enough frames free of this size
		uint free = emptyFramesKseg(search_start, search_count, search_size);
		PRINT_DEBUG("Found %d free frames, wanted %d frames.\n", free, search_count);
		ASSERT(free <= search_count);
		ASSERT( search_start + free
		          <= offsetKseg(smallerFrameSize(MIN_FRAME_SIZE)) );
		PRINT_DEBUG("Found %u free frames.\n", free);

		// if yes, we've found the space to allocate
		if (free == search_count) {
			// so determine the address
			*address = (void*)getAddressKseg(search_start, search_size);

			PRINT_DEBUG("Found enough free frames, at address %x\n", 
				(uintptr_t)(*address));

			ASSERT((uintptr_t)(*address) % frame_size == 0);

			uint addr_offset = (uintptr_t)(*address) / frame_size;

			int n = search_start - offsetKseg(search_size);
			PRINT_DEBUG("I.e. from the %u-th frame of size %u\n",
				n, search_size);

			ASSERT(n >= 0);

			// set frames from global offset search_start
			// of size requested by user; count requested by user
			setFramesAsUsedKseg(offsetKseg(frame_size) + addr_offset, 
				frame_size, count);
					
			// return the count of frames allocated
			return count;
		} 

		// else there are not enough free frames of this size
		PRINT_DEBUG("Not enough frames found.\n");

		// if the actual size of frame is the size user wanted,
		// check if we haven't found more than maximum of empty frames
		if ((search_size == frame_size) && (free > max_empty)) {
			max_empty = free;
			*address = (void*)getAddressKseg(search_start, search_size);
		}

		// and in either case set the start of search to the first
		// frame after the free frames (which can be the first full
		// frame or the first frame of smaller size)
		search_start = search_start + free;
		PRINT_DEBUG("New start of search: %u\n", search_start);
		// if the new start is the first frame of smaller size
		if (search_start == offsetKseg(search_size / FRAME_STEP)) {
			// set new search_size and search_count
			PRINT_DEBUG("New search start is a frame of smaller size.\n");
			search_size /= FRAME_STEP;
			search_count = search_counts[hash(search_size, HASH_RANGE)];
			PRINT_DEBUG("New search size: %u\n", search_size);
			PRINT_DEBUG("New search count: %u\n", search_count);
		}
	} // end while

	/* the loop ended in one of these cases:
		1) search_size < frame_size, i.e. we haven't found enough empty frames
		   of the size user wanted
		2) search_start > m_theEnd: this means the same in case user wanted
		   to search for the smallest frames
	   in each case return the maximum number of found subsequent free frames
	   *address holds address of the largest segment of free frames
	   or the initial value
	*/

	PRINT_DEBUG("allocateAtKseg0() ends, found only %u free frames.\n", max_empty);

	return max_empty;
}

/*----------------------------------------------------------------------------*/

template <int N>
uint FrameAllocator<N>::allocateAtKuseg( 
	void** address, const uint count, const uint frame_size )
{
	// TODO: will need searching in the bitmap from end to the front
	// until that, we'll use the same function as for allocating in KSEG
	//allocateAtKseg0(address, count, frame_size);

	PRINT_DEBUG("Allocation at KUSEG not functional yet :(\n");

	return 0;
}

/*----------------------------------------------------------------------------*/

template <int N>
uint FrameAllocator<N>::allocateAtAddress( const void* address, 
	const uint count, const uint frame_size )
{
	InterruptDisabler interrupts;

	// check if the given address is in the proper range:
	// starts in KSEG
	/*ASSERT(!(atKseg0 && ((uintptr_t)address > ADDR_SIZE_KSEG0)));*/

	// ends in KSEG
	/*ASSERT(atKseg0
	       && ( ((uintptr_t)address + count * frame_size) <= ADDR_SIZE_KSEG0 ));*/


	// or starts in KUSEG
	/*ASSERT(!(!atKseg0 && ((uintptr_t)address < ADDR_SIZE_KSEG0) ));*/


	// check if there is enough free frames
	// we must check it in the whole memory map, not only the KSEG0 map

	// check if the address is aligned at the frame size
	ASSERT((uintptr_t)address % frame_size == 0);

	// determine the number of the first frame
	uint addr_offset = ((uintptr_t)(address)) / frame_size;
	PRINT_DEBUG("Trying to allocate %u frames of size %u starting from address \
%x, which is the %u-th frame of this size\n", 
	count, frame_size, (uintptr_t)address, addr_offset + 1);

	// is the address correct? (i.e. is the addr_offset within the range
	// of bitmap offsets for this frame size?
	ASSERT((offsetKuseg(frame_size) + addr_offset) < offsetKuseg(frame_size / 4));

	// how many empty frames of this size are there, starting from this addr.
	uint free = emptyFramesKuseg(
		offsetKuseg(frame_size) + addr_offset, count, frame_size );

	ASSERT(free <= count);
	// if there are enough empty frames of this size
	if (free == count) {
		PRINT_DEBUG("Found enough free frames, setting them as allocated..\n");
		// this should mark also all frames in KSEG in case it's all in KSEG
		setFramesAsUsedKuseg(
			offsetKuseg(frame_size) + addr_offset, frame_size, count);
	} else {
		PRINT_DEBUG("Not enough free frames found. Found only %u frames free. Exiting..\n", free);
	}

	return free;
}

/*----------------------------------------------------------------------------*/

template <int N>
void FrameAllocator<N>::printFreeKseg() const
{
	PRINT_DEBUG("Checking the buddy bitmap for KSEG...\n");

	// check the buddy map
	for (uint i = MIN_FRAME_SIZE; i <= m_maxFrameSize; i *= FRAME_STEP) {
		uint free = 0;
		for (uint j = m_buddyInfoKseg[hash(i, HASH_RANGE)][1]; 
			j < ( (i == MIN_FRAME_SIZE) ? m_theEndKseg : m_buddyInfoKseg[hash(i/FRAME_STEP, HASH_RANGE)][1] );
			++j)
			free += !m_buddyMapKseg->bit(j);

		PRINT_DEBUG("%u empty frames of size %u\n", 
			free,
			i
		);
	}

	PRINT_DEBUG("Checking the hash map for KSEG...\n");

	for (uint i = MIN_FRAME_SIZE; i <= m_maxFrameSize; i *= FRAME_STEP) {
		PRINT_DEBUG("%u empty frames of size %u\n", 
			m_buddyInfoKseg[hash(i, HASH_RANGE)][0],
			i
		);
	}

	PRINT_DEBUG("DONE\n");
}

/*----------------------------------------------------------------------------*/

template <int N>
void FrameAllocator<N>::printFreeKuseg() const
{
	PRINT_DEBUG("Checking the buddy bitmap for KUSEG...\n");

	PRINT_DEBUG("Free frames in whole memory:\n");
	// check the buddy map
	for (uint i = MIN_FRAME_SIZE; i <= m_maxFrameSize; i *= FRAME_STEP) {
		uint free = 0;
		for (uint j = m_buddyInfoKuseg[hash(i, HASH_RANGE)][1]; 
			j < ( (i == MIN_FRAME_SIZE) ? m_theEndKuseg : m_buddyInfoKuseg[hash(i/FRAME_STEP, HASH_RANGE)][1] );
			++j)
			free += !m_buddyMapKuseg->bit(j);

		PRINT_DEBUG("%u empty frames of size %u in whole memory\n", 
			free,
			i
		);
	}

	PRINT_DEBUG("Free frames in the KUSEG segment:\n");
	for (uint i = MIN_FRAME_SIZE; i <= m_maxFrameSize; i *= FRAME_STEP) {
		uint free = 0;
		for (uint j = m_buddyInfoKuseg[hash(i, HASH_RANGE)][2]; 
			j < ( (i == MIN_FRAME_SIZE) ? m_theEndKuseg : m_buddyInfoKuseg[hash(i/FRAME_STEP, HASH_RANGE)][1] );
			++j)
			free += !m_buddyMapKuseg->bit(j);

		PRINT_DEBUG("%u empty frames of size %u in KUSEG segment\n", 
			free,
			i
		);
	}

	PRINT_DEBUG("Checking the hash map for KUSEG...\n");

	for (uint i = MIN_FRAME_SIZE; i <= m_maxFrameSize; i *= FRAME_STEP) {
		PRINT_DEBUG("%u empty frames of size %u\n", 
			m_buddyInfoKuseg[hash(i, HASH_RANGE)][0],
			i
		);
	}

	PRINT_DEBUG("DONE\n");
}


/*----------------------------------------------------------------------------*/

template <int N>
inline uint FrameAllocator<N>::hash( const int key, const int range ) const 
{ 
	return (key % range);
}

/*----------------------------------------------------------------------------*/

template <int N>
inline uint& FrameAllocator<N>::offsetKseg( const uint frame_size )
{ 
	ASSERT(frame_size >= (MIN_FRAME_SIZE / FRAME_STEP));
	return m_buddyInfoKseg[hash(frame_size, HASH_RANGE)][1];
}

/*----------------------------------------------------------------------------*/

template <int N>
inline uint& FrameAllocator<N>::offsetKuseg( const uint frame_size )
{ 
	ASSERT(frame_size >= MIN_FRAME_SIZE / FRAME_STEP);
	return m_buddyInfoKuseg[hash(frame_size, HASH_RANGE)][1];
}

/*----------------------------------------------------------------------------*/

template <int N>
inline uint& FrameAllocator<N>::offsetKusegInAll( const uint frame_size )
{
	ASSERT(frame_size >= MIN_FRAME_SIZE / FRAME_STEP);
	return m_buddyInfoKuseg[hash(frame_size, HASH_RANGE)][2];
}

/*---------------------------------------------------------------------------*/

template <int N>
inline uint FrameAllocator<N>::offsetOfChildKseg( 
		const uint local_offset, const uint frame_size )
{
	return offsetKseg(smallerFrameSize(frame_size)) 
	                   + (local_offset * FRAME_STEP);
}

/*---------------------------------------------------------------------------*/

template <int N>
inline uint FrameAllocator<N>::offsetOfChildKuseg( 
	const uint local_offset, const uint frame_size )
{
	return offsetKuseg(smallerFrameSize(frame_size)) 
	                    + (local_offset * FRAME_STEP);
}

/*---------------------------------------------------------------------------*/

template <int N>
inline uint FrameAllocator<N>::offsetOfParentKseg( 
		const uint local_offset, const uint frame_size )
{
	return offsetKseg(largerFrameSize(frame_size)) 
	                   + (local_offset / FRAME_STEP);
}

/*---------------------------------------------------------------------------*/

template <int N>
inline uint FrameAllocator<N>::offsetOfParentKuseg( 
		const uint local_offset, const uint frame_size )
{
	return offsetKuseg(largerFrameSize(frame_size)) + (local_offset / FRAME_STEP);
}

/*---------------------------------------------------------------------------*/

template <int N>
inline uint& FrameAllocator<N>::freeFramesKseg( const uint frame_size )
{ 
	return m_buddyInfoKseg[hash(frame_size, HASH_RANGE)][0];
}

/*---------------------------------------------------------------------------*/

template <int N>
inline uint& FrameAllocator<N>::freeFramesKuseg( const uint frame_size )
{ 
	return m_buddyInfoKuseg[hash(frame_size, HASH_RANGE)][0];
}

/*---------------------------------------------------------------------------*/

template <int N>
inline uint FrameAllocator<N>::emptyFramesKseg( const uint global_offset,
		const uint count, const uint frame_size )
{
	PRINT_DEBUG("emptyFramesKseg() started...\n");
	PRINT_DEBUG("Global offset: %u, count: %u, frame size: %u\n", 
		global_offset, count, frame_size);
	PRINT_DEBUG("Enough will be min of %u and %u, which is %u\n",
		count, offsetKseg(smallerFrameSize(frame_size)) - global_offset,
		min(count, (offsetKseg(smallerFrameSize(frame_size)) - global_offset)));

	uint free = m_buddyMapKseg->empty(global_offset, 
		min(count, (offsetKseg(smallerFrameSize(frame_size)) - global_offset)) );
	PRINT_DEBUG("Returning %u\n", free);
	return free;
}

/*---------------------------------------------------------------------------*/

template <int N>
inline uint FrameAllocator<N>::emptyFramesKuseg( const uint global_offset,
	const uint count, const uint frame_size )
{
	return m_buddyMapKuseg->empty(global_offset, 
		min(count, (offsetKuseg(smallerFrameSize(frame_size)) - global_offset)) );
}

/*---------------------------------------------------------------------------*/

template <int N>
inline uint FrameAllocator<N>::fullFramesKseg( const uint global_offset,
		const uint count, const uint frame_size )
{
	return m_buddyMapKseg->full(global_offset/*, 
		min(count, (offsetKseg(smallerFrameSize(frame_size)) - global_offset))*/ );
}

/*---------------------------------------------------------------------------*/

template <int N>
inline uint FrameAllocator<N>::fullFramesKuseg( const uint global_offset,
	const uint count, const uint frame_size )
{
	return m_buddyMapKuseg->full(global_offset/*, 
		min(count, (offsetKuseg(smallerFrameSize(frame_size)) - global_offset))*/ );
}

/*---------------------------------------------------------------------------*/

template <int N>
inline uint FrameAllocator<N>::min( const uint x, const uint y ) const
{
	return (x < y) ? x : y;
}

/*---------------------------------------------------------------------------*/

template <int N>
inline uint FrameAllocator<N>::findFirstEmptyKseg( const uint offset ) const
{
	return offset + m_buddyMapKseg->full(offset);
}

/*---------------------------------------------------------------------------*/

template <int N>
inline uint FrameAllocator<N>::findFirstEmptyKuseg( const uint offset ) const
{
	return offset + m_buddyMapKuseg->full(offset);
}

/*---------------------------------------------------------------------------*/

template <int N>
inline uint FrameAllocator<N>::findFirstFullKseg( const uint offset ) const
{
	return offset + m_buddyMapKseg->empty(offset);
}

/*---------------------------------------------------------------------------*/

template <int N>
inline uint FrameAllocator<N>::findFirstFullKuseg( const uint offset ) const
{
	return offset + m_buddyMapKuseg->empty(offset);
}

/*---------------------------------------------------------------------------*/

template <int N>
inline uint FrameAllocator<N>::parent( const uint local_offset ) const
{
	return roundDown(local_offset, FRAME_STEP) / FRAME_STEP;
}

/*---------------------------------------------------------------------------*/

template <int N>
inline uint FrameAllocator<N>::firstChild( const uint local_offset ) const
{
	return local_offset * FRAME_STEP;
}

/*---------------------------------------------------------------------------*/

template <int N>
inline uint FrameAllocator<N>::largerFrameSize( const uint frame_size ) const
{
	return frame_size * FRAME_STEP;
}

/*---------------------------------------------------------------------------*/

template <int N>
inline uint FrameAllocator<N>::smallerFrameSize( const uint frame_size ) const
{
	return frame_size / FRAME_STEP;
}

/*---------------------------------------------------------------------------*/

template <int N>
inline uintptr_t FrameAllocator<N>::getAddressKseg( 
	const uint off, const uint frame_size )
{
	ASSERT(frame_size >= MIN_FRAME_SIZE);
	return ( (off - offsetKseg(frame_size)) * frame_size );
}

/*---------------------------------------------------------------------------*/

template <int N>
inline uintptr_t FrameAllocator<N>::getAddressKuseg( 
	const uint off, const uint frame_size )
{
	ASSERT(frame_size >= MIN_FRAME_SIZE);
	return ( (off - offsetKuseg(frame_size)) * frame_size );
}



typedef FrameAllocator<7> MyFrameAllocator;
