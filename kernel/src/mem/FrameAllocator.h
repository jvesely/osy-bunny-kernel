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

#include "structures/Bitset.h"
#include "types.h"
#include "cpp.h"
#include "tools.h"
#include "../../common/address.h"
#include "flags.h"
#include "InterruptDisabler.h"
#include "Singleton.h"

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
 * @todo add function to check the whole bitmap if the used frames of 
 * different levels are properly set
 *
 * @todo check defaultFrame
 */
template <uint N>
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
	 * @brief Tries to allocate @a count frames of size @a frame_size 
	 * due to bit flags set in @a flags parameter.
	 *
	 * @param address Pointer to a pointer where the <b>physicall address</b> 
	 *                of the allocated block will be saved. If VF_VA_USER flag 
	 *                is set, the function will try to allocate the memory on
	 *                this address.
	 * @param count Count of subsequent frames requested by user to be allocated.
	 * @param frame_size Size of the frames requested by user to be allocated.
	 * @param flags Holds bit flags for influencing the function behaviour 
	 *              (see below).
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
	 * @return @a count in case the allocation was successful. In this case
	 *		            @a *address holds physical address of start of the 
	 *                  allocated block of frames.
	 *         <i>Count of largest block</i> of subsequent free frames of size 
	 *                  @a frame_size in case the allocation was not successful. 
	 *                  @a @address will be the physical address of this block.
	 */
	uint frameAlloc( void** address, const size_t count, 
		const uint frame_size, const uint flags );

	/*!
	 * @brief Tries to allocate @a count frames of size @a frame_size inside
	 * the KSEG segment physical address space (0 - 512 MB).
	 *
	 * @param address Pointer to a pointer where the <b>physicall address</b> 
	 * of the allocated block will be saved.
	 * @param count Count of subsequent frames requested by user to be allocated.
	 * @param frame_size Size of the frames requested by user to be allocated.
	 *
	 * @note Use this function to avoid using flags in function frameAlloc().
	 * 
	 * @retval @a count in case the allocation was successful. In this case
	 * @a *address holds physical address of start of the allocated block 
	 * of frames.
	 * @retval Count of largest block of subsequent free frames of size 
	 * @a frame_size in case the allocation was not successful. @a @address 
	 * will be the physical address of this block.
	 */
	inline uint allocateAtKseg0( 
		void** address, const uint count, const uint frame_size );

	/*!
	 * 
	 */
	inline uint allocateAtKuseg( 
		void** address, const uint count, const uint frame_size );

	/*!
	 * @brief Tries to allocate @a count frames of size @a frame_size at
	 * the given address.
	 *
	 * @param address Pointer to a pointer where the allocated frames should 
	 * start.
	 * @param count Count of subsequent frames requested by user to be allocated.
	 * @param frame_size Size of the frames requested by user to be allocated.
	 *
	 * @note Use this function to avoid using flags in function frameAlloc().
	 * 
	 * @retval @a count in case the allocation was successful. 
	 * @retval Count of largest block of subsequent free frames of size 
	 * @a frame_size in case the allocation was not successful.
	 */
	uint allocateAtAddress( 
		const void* address, const uint count, const uint frame_size );

	/*! 
	 * @brief Frees the memory previously allocated using frameAlloc(),
	 * allocateAtKseg0(), allocateAtKuseg() or allocateAtAddress().
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

	/*! 
	 * @brief Returns the state of the frame allocator. Always use this 
	 * function to check if FrameAllocator was initialized successfully, 
	 * before calling frameAlloc() or frameFree().
	 */
	inline bool isInitialized() const { return m_initialized; }

	void test();

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

	static const uint KSEG0_SIZE = ADDR_SIZE_KSEG0;//0x1000000; // 16 MB
	
	enum AddressType { KSEG, KUSEG };

	typedef uint InfoHashTable[HASH_RANGE][2];

	/*! @brief Default constructor. Only marks the FrameAllocator as 
	 * not initialized.
	 */
	FrameAllocator(): m_initialized(false) {};

	uint allocateAtSegment( void** address, const uint count,
		const uint frame_size, const AddressType type );

	uint defaultFrame( uint* search_counts, const uint frame_size,
		const InfoHashTable* info_table ) const;

	void setFramesAsUsed( const uint global_offset, const uint frame_size,
		const uint count, const AddressType type );

	inline void setFramesAsFree( const uint global_offset, 
		const uint frame_size, const uint count, const AddressType type );

	inline uint findFirstEmpty( 
		const uint offset, const Bitset* buddy_map ) const;

	inline uint findFirstFull( 
		const uint offset, const Bitset* buddy_map ) const;

	inline uint hash( const uint key, const uint range ) const;

	inline uint& offset( const uint frame_size, InfoHashTable* info_table );

	inline uint offset( 
		const uint frame_size, const InfoHashTable* info_table ) const;

	inline uint& freeFrames( const uint frame_size, InfoHashTable* info_table );

	inline uint freeFrames( 
		const uint frame_size, const InfoHashTable* info_table ) const;

	inline uint offsetOfChild( const uint local_offset, 
		const uint frame_size, const InfoHashTable* info_table ) const;

	inline uint offsetOfParent( const uint local_offset, 
		const uint frame_size, const InfoHashTable* info_table ) const;

	inline uint emptyFrames( const uint global_offset, const uint count, 
		const uint frame_size, const AddressType type ) const;

	inline uint emptyFramesKseg( const uint global_offset, 
		const uint count, const uint frame_size ) const;

	inline uint emptyFramesKuseg( const uint global_offset, 
		const uint count, const uint frame_size ) const;

	inline uint fullFrames( const uint global_offset, const uint count, 
		const uint frame_size, const AddressType type ) const;

	inline uint fullFramesKseg( const uint global_offset, 
		const uint count, const uint frame_size ) const;

	inline uint fullFramesKuseg( const uint global_offset, 
		const uint count, const uint frame_size ) const;

	inline uint min( const uint x, const uint y ) const;

	inline uint parent( const uint local_offset ) const;

	inline uint firstChild( const uint local_offset ) const;

	inline uint largerFrameSize( const uint frame_size ) const;

	inline uint smallerFrameSize( const uint frame_size ) const;

	inline uintptr_t getAddress( const uint off, const uint frame_size,
		const InfoHashTable* info_table ) const;

	inline uint theEnd( const InfoHashTable* info_table ) const;

	/*!
	 * @brief If the symbol FRALLOC_DEBUG is defined, this function prints out
	 * the state of the internal structures of the frame allocator.
	 */
	inline void checkStructures() const;

	void printFree( 
		const Bitset* buddy_map, const InfoHashTable* info_table ) const;

	bool checkBitmap( 
		const Bitset* buddy_map, const InfoHashTable* info_table ) const;

	bool checkBitmaps() const;

	bool testAllocationAtAddress();

	bool testAllocationAtSegment( const AddressType type );

	//inline unsigned long rand();
		

	/*------------------------------------------------------------------------*/
	/* MEMBERS                                                                */
	/*------------------------------------------------------------------------*/

	Bitset* m_buddyMapKseg;
	Bitset* m_buddyMapKuseg;

	/*! 
	 * Index meanings:
	 *	[hash(frame_size, 19), 0] ... count of free frames of size 
	 * @a frame_size,\n
	 *	[hash(frame_size, 19), 1] ... offset of the bitmap representing frames 
	 * of size @a frame_size.
	 */
	InfoHashTable m_buddyInfoKseg;

	/*! 
	 * Index meanings:
	 *	[hash(frame_size, 19), 0] ... count of free frames of size 
	 * @a frame_size,\n
	 *	[hash(frame_size, 19), 1] ... offset of the bitmap representing frames 
	 * of size @a frame_size,
	 *	[hash(frame_size, 19), 2] ... offset of the first frame of size 
	 * @a frame_size which lies in the KUSEG segment.
	 */
	InfoHashTable m_buddyInfoKuseg;
	
	uint m_maxFrameSize;

	uintptr_t m_endOfBlockedFrames;
	size_t m_memorySize;
	bool m_initialized;


friend class Singleton< FrameAllocator<N> >;
};


/*----------------------------------------------------------------------------*/
/* DEFINITIONS ---------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

template <uint N>
uintptr_t FrameAllocator<N>::init( size_t memory_size, const size_t kernel_end )
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
	//char* pos = (char*)roundUp(kernel_end, 0x4);
	
	//PRINT_DEBUG("4 kB frames used for kernel: %u\n", used_frames);
	PRINT_DEBUG("Kernel ends on address %x\n", kernel_end);
	PRINT_DEBUG("Frame allocator structures will start on address %x\n", 
		        (uint)pos);

	/*--------------------------------------------------------------------------
	  find the largest frame size to know how much memory I can use
	--------------------------------------------------------------------------*/

	uint frame_size = MIN_FRAME_SIZE;

	// let's assume we have more than 4kB memory available
	ASSERT(memory_size > MIN_FRAME_SIZE);

	// and that the KSEG0 segment's size is a multiple of the largest frame size
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

	m_memorySize = memory_size;
	/*--------------------------------------------------------------------------
	  set the number of free frames and offset in the bitset of each frame size
	--------------------------------------------------------------------------*/

	// for used frame sizes, the largest frames will start on offset 0
	uint total_buddy_frames_kseg = 0;
	// we must separately count the offset to the KSEG0 and KUSEG maps
	uint total_buddy_frames_kuseg = 0;

	// as the size of the memory for KUSEG map, we can use the memory_size param
	// but for the size of the KSEG0 segment, we may need to use the size of 
	// KSEG0 segment
	uint kseg_size = min(memory_size, KSEG0_SIZE);
	uint kuseg_size = memory_size - kseg_size;

	for ( frame_size = MAX_FRAME_SIZE; 
	      frame_size >= MIN_FRAME_SIZE; 
		  frame_size /= FRAME_STEP) {

		// number of frames with size frame_size
		uint frames_kseg = kseg_size / frame_size;
		uint frames_kuseg = kuseg_size / frame_size;
		
		PRINT_DEBUG(
			"The KSEG map for frame size %u will have %u items at offset %u\n", 
			frame_size, frames_kseg, total_buddy_frames_kseg);
		PRINT_DEBUG(
			"The KUSEG map for frame size %u will have %u items at offset %u\n", 
			frame_size, frames_kuseg, total_buddy_frames_kuseg);
		
		// set count of free frames on this level
		freeFrames(frame_size, &m_buddyInfoKseg) = frames_kseg;
		freeFrames(frame_size, &m_buddyInfoKuseg) = frames_kuseg;

		// set the offset of level-th map
		offset(frame_size, &m_buddyInfoKseg) = total_buddy_frames_kseg;
		offset(frame_size, &m_buddyInfoKuseg) = total_buddy_frames_kuseg;

		total_buddy_frames_kseg += frames_kseg;
		total_buddy_frames_kuseg += frames_kuseg;
	}
	// keep the maximum offset as the offset of MIN_FRAME_SIZE / FRAME_STEP
	ASSERT(frame_size == MIN_FRAME_SIZE / FRAME_STEP);
	offset(frame_size, &m_buddyInfoKseg) = total_buddy_frames_kseg;
	offset(frame_size, &m_buddyInfoKuseg) = total_buddy_frames_kuseg;

	PRINT_DEBUG("The whole KSEG buddy map will have %u items\n", 
		total_buddy_frames_kseg);
	PRINT_DEBUG("The whole KUSEG buddy map will have %u items\n", 
		total_buddy_frames_kuseg);

	/*--------------------------------------------------------------------------
	  create and initialize bitmaps (for KSEG0/1 and other memory)
	--------------------------------------------------------------------------*/
	m_buddyMapKseg = new (pos) 
	                     Bitset(pos + sizeof(Bitset), total_buddy_frames_kseg);
	PRINT_DEBUG("Current position: %p\n", pos);
	PRINT_DEBUG("Sizeof(Bitset): %d\n", sizeof(Bitset));
	PRINT_DEBUG("Bitset structures size: %d\n", 
		Bitset::getContainerSize(total_buddy_frames_kseg));
	pos += (sizeof(Bitset) + Bitset::getContainerSize(total_buddy_frames_kseg));

	// do we have even to create the Bitset?
	if (total_buddy_frames_kuseg > 0) {
		m_buddyMapKuseg = new (pos) Bitset(pos + sizeof(Bitset),
                                            total_buddy_frames_kuseg);
		pos += (sizeof(Bitset) 
		        + Bitset::getContainerSize(total_buddy_frames_kuseg));
	} else {
		m_buddyMapKuseg = NULL;
	}

	// pos points to the end of my bitset structures
	PRINT_DEBUG("Frame allocator structures end at address %x\n", pos);
	
	ASSERT( ( kernel_end + sizeof(Bitset)
	        + Bitset::getContainerSize(total_buddy_frames_kseg)
			+ ((total_buddy_frames_kuseg > 0)
				? (sizeof(Bitset) 
				    + Bitset::getContainerSize(total_buddy_frames_kuseg))
				: 0) )
			== (uintptr_t)pos );

	// determine the count of frames used by kernel and my structures
	m_endOfBlockedFrames = roundUp(ADDR_TO_USEG((uint)pos), MIN_FRAME_SIZE);
	uint used_frames =  m_endOfBlockedFrames / MIN_FRAME_SIZE;

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

	checkStructures();
#endif

	PRINT_DEBUG("Setting frames occupied by kernel and fr. allocator as used\n");

	// set all used frames as used in the bitmap
	setFramesAsUsed(offset(MIN_FRAME_SIZE, &m_buddyInfoKseg), MIN_FRAME_SIZE,
		used_frames, KSEG);

	PRINT_DEBUG("DONE\n");

#ifdef FRALLOC_DEBUG
	checkStructures();
	checkBitmap(m_buddyMapKseg, &m_buddyInfoKseg);
	checkBitmap(m_buddyMapKuseg, &m_buddyInfoKuseg);
#endif

	checkStructures();

	//m_endOfBlockedFrames = ADDR_TO_USEG(roundUp((uintptr_t)pos, MIN_FRAME_SIZE));
	
	m_initialized = true;
	PRINT_DEBUG("Buddy maps pointers: KSEG: %p, KUSEG: %p\n", 
		m_buddyMapKseg, m_buddyMapKuseg);
	PRINT_DEBUG("Frame Allocator initialized. Returning: %p\n", pos);
	return (uintptr_t)pos;
}

/*----------------------------------------------------------------------------*/

template <uint N>
uint FrameAllocator<N>::frameAlloc( void** address, const size_t count, 
	const uint frame_size, const uint flags )
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
	}
	// else 
	ASSERT((flags & VF_VA_MASK) == (VF_VA_USER << VF_VA_SHIFT));
	return allocateAtAddress(*address, count, frame_size );
}

/*---------------------------------------------------------------------------*/

template <uint N>
inline uint FrameAllocator<N>::allocateAtKseg0( 
	void** address, const uint count, const uint frame_size )
{
	return allocateAtSegment(address, count, frame_size, KSEG);
}

/*---------------------------------------------------------------------------*/

template <uint N>
uint FrameAllocator<N>::allocateAtKuseg( 
	void** address, const uint count, const uint frame_size )
{
	uint free_kuseg = allocateAtSegment(address, count, frame_size, KUSEG);

	// if the allocation in KUSEG was successful, end
	if (free_kuseg == count)
		return free_kuseg;
	// there should not be more allocated frames
	ASSERT(free_kuseg < count);

	/*
	 * try to allocate on the border
	 */

	// free frames from the beginning of this frame size's bitmap
	free_kuseg = emptyFramesKuseg(
		offset(frame_size, &m_buddyInfoKuseg), count, frame_size);
	// if there are some free frames, starting from the border, in KUSEG segment
	if (free_kuseg > 0) {
		ASSERT(free_kuseg < count);
		uint count_kseg = count - free_kuseg;
		uint start_kseg = offset(smallerFrameSize(frame_size), &m_buddyInfoKseg)
							- count_kseg;
		// check if there are enough frames on the KSEG side of the border
		uint free_kseg = emptyFramesKseg(start_kseg, count_kseg, frame_size);
		ASSERT(free_kseg <= count_kseg);

		// if there are enough
		if (free_kseg == count_kseg) {
			// allocate at the border
			
			// so determine the address
			*address = (void*)getAddress(offset(start_kseg, &m_buddyInfoKseg),
			                              frame_size, &m_buddyInfoKseg);

			PRINT_DEBUG("Found enough free frames, at address %p\n", *address);

			ASSERT((uintptr_t)(*address) % frame_size == 0);

			uint addr_offset = (uintptr_t)(*address) / frame_size;

#ifdef FRALLOC_DEBUG
			int n = start_kseg - offset(frame_size, &m_buddyInfoKseg);
			PRINT_DEBUG("I.e. from the %u-th frame of size %u\n",
				n, frame_size);

			ASSERT(n >= 0);
#endif
			// set frames from global offset start_kseg
			// of size requested by user; count requested by user
			ASSERT(start_kseg 
			        = offset(frame_size, &m_buddyInfoKseg) + addr_offset);

			setFramesAsUsed(start_kseg, frame_size, count_kseg, KSEG);
			setFramesAsUsed(offset(frame_size, &m_buddyInfoKuseg), frame_size,
			                free_kuseg, KUSEG);

			// return the count of frames allocated
			return count;

		}
		// else, try to allocate at KSEG
	}
	// else try to allocate at KSEG

	return allocateAtSegment(address, count, frame_size, KSEG);
}

/*---------------------------------------------------------------------------*/

template <uint N>
uint FrameAllocator<N>::allocateAtSegment( void** address, const uint count,
	const uint frame_size, const AddressType type )
{
	InterruptDisabler interrupts;

	PRINT_DEBUG("allocateAtSegment() started...\n");

	ASSERT(frame_size >= MIN_FRAME_SIZE);
	ASSERT(count > 0);

	Bitset* buddy_map;
	InfoHashTable* info_table;

	if (type == KSEG) {
		buddy_map = m_buddyMapKseg;
		info_table = &m_buddyInfoKseg;
	} else {
		buddy_map = m_buddyMapKuseg;
		info_table = &m_buddyInfoKuseg;
	}

	uint search_counts[HASH_RANGE];
	search_counts[hash(frame_size, HASH_RANGE)] = count;

	// determine the size of frame(s) we'll try to find
	uint search_size = defaultFrame(search_counts, frame_size, info_table);
	ASSERT(search_size >= frame_size);
	
	uint search_count = search_counts[hash(search_size, HASH_RANGE)];
	uint search_start = offset(search_size, info_table);
	
	PRINT_DEBUG("Starting to search for %u frames of size %u, from offset %u\n",
		search_count, search_size, search_start);

	// here we will store the maximum number of subsequent empty frames of 
	// the size we want
	uint max_empty = 0;

	// loop until we are searching for frames large enough or 
	// until we're on the end of the bitmap
	// (just to be sure we don't use memory which isn't ours
	while (search_size >= frame_size 
	       && search_start < theEnd(info_table) - 1) {
		
		ASSERT(search_size >= MIN_FRAME_SIZE);
		
		PRINT_DEBUG("Searching for the first empty frame...\n");
		// find offset of the first empty frame
		search_start = findFirstEmpty(search_start, buddy_map);
		PRINT_DEBUG("First empty frame found on global offset %u\n", 
			search_start);
		
		// we're beyond the end of interesting frames' map
		if (search_start >= offset(smallerFrameSize(frame_size), info_table)) {
			PRINT_DEBUG("We're beyond the bitmap of the required frame size, stopping the loop...\n");
			break;
		}

		ASSERT(search_size >= MIN_FRAME_SIZE);
		
		// we're beyond the end of the actually sought frame size's bitmap
		if (search_start >= offset(smallerFrameSize(search_size), info_table)) {
			PRINT_DEBUG("We're beyond the end of the actually sought frame size's bitmap\n");
			PRINT_DEBUG("Search start: %u\n", search_start);
			PRINT_DEBUG("Global offset of end of these frame size: %d\n", 
				offset(smallerFrameSize(search_size), info_table));

			PRINT_DEBUG("Determining the new search size...\n");
			// determine what size is this frame and set proper search_count
			while (search_start 
			        >= offset(smallerFrameSize(search_size), info_table)) {
				search_size /= FRAME_STEP;
				PRINT_DEBUG("Trying search size %d\n", search_size);
				PRINT_DEBUG("Global offset of this size's bitmap: %d\n",
					offset(search_size, info_table));
			}
			search_count = search_counts[hash(search_size, HASH_RANGE)];
			PRINT_DEBUG("New search size: %u\n", search_size);
			PRINT_DEBUG("New search count: %u\n", search_count);

			ASSERT(search_size >= frame_size);
			// else we can continue executing commands in this loop
		}

		// it's still the frame size we want
		ASSERT(search_start < offset(smallerFrameSize(search_size), info_table));
			
		// try if there are enough frames free of this size
		uint free = emptyFrames(search_start, search_count, search_size, type);
		PRINT_DEBUG("Found %d free frames, wanted %d frames.\n", free, search_count);
		ASSERT(free <= search_count);
		ASSERT( search_start + free
		          <= offset(smallerFrameSize(MIN_FRAME_SIZE), info_table) );

		// if yes, we've found the space to allocate
		if (free == search_count) {
			// so determine the address
			*address = (void*)getAddress(search_start, search_size, info_table);
	
			PRINT_DEBUG("Found enough free frames, at address %x\n", *address);

			ASSERT((uintptr_t)(*address) % frame_size == 0);
			ASSERT((uintptr_t)(*address) < m_memorySize);

			uint addr_offset = (uintptr_t)(*address) / frame_size;

#ifdef FRALLOC_DEBUG
			int n = search_start - offset(search_size, info_table);
			PRINT_DEBUG("I.e. from the %u-th frame of size %u\n",
				n, search_size);

			ASSERT(n >= 0);

			n = 0;	// just to be used
#endif
			// set frames from global offset search_start
			// of size requested by user; count requested by user
			//ASSERT(search_start == offset(frame_size, info_table) + addr_offset);

			setFramesAsUsed(offset(frame_size, info_table) + addr_offset, 
				frame_size, count, type);
			// return the count of frames allocated
			return count;
		} 

		// else there are not enough free frames of this size
		PRINT_DEBUG("Not enough frames found.\n");

		// if the actual size of frame is the size user wanted,
		// check if we haven't found more than maximum of empty frames
		if ((search_size == frame_size) && (free > max_empty)) {
			max_empty = free;
			*address = (void*)getAddress(search_start, search_size, info_table);
		}

		// and in either case set the start of search to the first
		// frame after the free frames (which can be the first full
		// frame or the first frame of smaller size)
		search_start = search_start + free;
		PRINT_DEBUG("New start of search: %u\n", search_start);
		// if the new start is the first frame of smaller size
		
		ASSERT(search_start <= offset(smallerFrameSize(search_size), info_table));

		if (search_start == offset(smallerFrameSize(search_size), info_table)) {
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

	PRINT_DEBUG("allocateAtSegment() ends, found only %u free frames.\n", max_empty);

	return max_empty;
}

/*---------------------------------------------------------------------------*/

template <uint N>
uint FrameAllocator<N>::allocateAtAddress( const void* address, 
	const uint count, const uint frame_size )

{
	InterruptDisabler interrupts;

	/*
	 * TODO: TEST!!!
	 */

	/*
	 * Determine where the block lies (KSEG / KUSEG / their border)
	 */

	uintptr_t addr = (uintptr_t)address;
	ASSERT(addr % frame_size == 0);

	ASSERT(addr < m_memorySize);
	
	// if trying to allocate in the blocked frames
	if (addr < m_endOfBlockedFrames)
		return 0;

	AddressType type;
	InfoHashTable* info_table = 0;

	if ((addr < KSEG0_SIZE) /* the block will start in KSEG */
		&& ((addr + (count * frame_size)) > KSEG0_SIZE) /* and end in KUSEG */)
	{
		// so the block will lie on the border of KSEG and KUSEG segments
		PRINT_DEBUG("The allocated block will lie on the border between the \
KSEG and KUSEG segments.\n");

		// do we even have the KUSEG segment?
		ASSERT(m_buddyMapKuseg);

		// determine the number of the first frame
		uint addr_offset_kseg = addr / frame_size;
		PRINT_DEBUG("Trying to allocate %u frames of size %u starting from \
address	%x, which is the %u-th frame of this size %u, on the border.\n", count, 
			frame_size, (uintptr_t)address, addr_offset_kseg, frame_size);

		uint count_kseg = offset(smallerFrameSize(frame_size), &m_buddyInfoKseg)
		                   - offset(frame_size, &m_buddyInfoKseg)
						  - addr_offset_kseg;
		ASSERT(count_kseg > 0);

		uint addr_offset_kuseg = 0;
		PRINT_DEBUG("The KUSEG part of the block will start from the %u-th \
frame of size %u", addr_offset_kuseg, frame_size);

		uint count_kuseg = count - count_kseg;
		// is the count correct?
		ASSERT(count_kuseg == ((addr + (count * frame_size)) - KSEG0_SIZE) 
		                       / frame_size);
		// is the address correct? (i.e. is the addr_offset within the range
		// of bitmap offsets for this frame size?)
		ASSERT(addr_offset_kuseg + count_kuseg 
			<= ( offset(smallerFrameSize(frame_size), &m_buddyInfoKuseg)
			      - offset(frame_size, &m_buddyInfoKuseg) ));
		
		// how many empty frames of this size are there, starting from this addr
		uint free_kseg = emptyFrames( 
			offset(frame_size, &m_buddyInfoKseg) + addr_offset_kseg, 
			count_kseg, frame_size, KSEG );
		PRINT_DEBUG("Found %u free frames from %u in KSEG.\n", 
			free_kseg, count_kseg);

		if (free_kseg < count_kseg)
			return free_kseg;

		ASSERT(free_kseg == count_kseg);

		uint free_kuseg = emptyFrames( 
			offset(frame_size, &m_buddyInfoKuseg) + addr_offset_kuseg, 
			count_kuseg, frame_size, KUSEG );
		PRINT_DEBUG("Found %u free frames from %u in KUSEG.\n", 
			free_kuseg, count_kuseg);

		ASSERT(free_kuseg <= count_kuseg);

		if (free_kuseg == count_kuseg) {
			PRINT_DEBUG("Found enough free frames, setting them as used...\n");
			setFramesAsUsed(
				offset(frame_size, &m_buddyInfoKseg) + addr_offset_kseg, 
				frame_size, count_kseg, KSEG);
			setFramesAsUsed(
				offset(frame_size, &m_buddyInfoKuseg) + addr_offset_kuseg, 
				frame_size, count_kuseg, KUSEG);
		} else {
			PRINT_DEBUG("Not enough free frames found. Exiting\n");
		}
	
		return free_kseg + free_kuseg;

	} 
	// else
	
	// set proper help variables 
	if (addr >= KSEG0_SIZE) {
		// the whole block will lie within the KUSEG segment
		addr -= KSEG0_SIZE;
		info_table = &m_buddyInfoKuseg;
		type = KUSEG;
		ASSERT(m_buddyMapKuseg);
		PRINT_DEBUG("The allocated block will lie within KUSEG segment.\n");
	} else {
		// the whole block will lie within the KSEG segment
		info_table = &m_buddyInfoKseg;
		type = KSEG;
		PRINT_DEBUG("The allocated block will lie within KSEG segment.\n");
	}

	// determine the number of the first frame
	uint addr_offset = addr / frame_size;
	PRINT_DEBUG("Trying to allocate %u frames of size %u starting from \
address	%x, which is the %u-th frame of this size\n",
		count, frame_size, (uintptr_t)address, addr_offset);

	// is the address correct? (i.e. is the addr_offset within the range
	// of bitmap offsets for this frame size?)
	ASSERT((offset(frame_size, info_table) + addr_offset) 
	         < offset(frame_size / FRAME_STEP, info_table));

	// how many empty frames of this size are there, starting from this addr
	uint free = emptyFrames( offset(frame_size, info_table) 
	                          + addr_offset, count, frame_size, type );

	ASSERT(free <= count);
	// if there are enough empty frames of this size
	if (free == count) {
		PRINT_DEBUG("Found enough free frames, setting them as used...\n");
		setFramesAsUsed(
			offset(frame_size, info_table) + addr_offset, 
			frame_size, count, type);
	} else {
		PRINT_DEBUG("Not enough free frames found. \
Found only %u frames free. Exiting..\n", free);
	}

	return free;
}

/*----------------------------------------------------------------------------*/

template <uint N>
bool FrameAllocator<N>::frameFree( 
	const void* address, const size_t count, const uint frame_size )
{
	InterruptDisabler interrupts;

	PRINT_DEBUG("FrameAllocator::frameFree() started...\n");
	PRINT_DEBUG("Request to free %u frames of size %u, starting from address %x\n", count, frame_size, address);

	uintptr_t addr = (uintptr_t)address;
	
	ASSERT(addr < m_memorySize);

	// don't allow freeing the kernel or my memory
	if (addr < m_endOfBlockedFrames) {
		PRINT_DEBUG("Address points to blocked memory (to %d or before)!\n", 
			m_endOfBlockedFrames);
		return false;
	}

	// check if the address is aligned at the frame start
	if (addr % frame_size != 0) {
		PRINT_DEBUG("Address is not alligned!\n");
		return false;
	}

	/*
	 * Determine where the block lies (KSEG / KUSEG / their border)
	 */

	if ((addr < KSEG0_SIZE) /* the block will start in KSEG */
		&& (((addr + (count * frame_size)) > KSEG0_SIZE)) /*and end in KUSEG*/)
	{
		// so the block will lie on the border of KSEG and KUSEG segments
		PRINT_DEBUG("The block lies on the border between the \
KSEG and KUSEG segments.\n");
		

		// do we even have the KUSEG segment?
		ASSERT(m_buddyMapKuseg);

		// determine the number of the first frame
		uint addr_offset_kseg = addr / frame_size;
		/*printf("Trying to free %u frames of size %u starting from \
address	%x, which is the %u-th frame of this size %u, on the border.\n", count, 
			frame_size, (uintptr_t)address, addr_offset_kseg, frame_size);

		ASSERT(false);*/
		uint count_kseg = offset(smallerFrameSize(frame_size), &m_buddyInfoKseg)
		                   - offset(frame_size, &m_buddyInfoKseg)
						  - addr_offset_kseg;
		ASSERT(count_kseg > 0);

		uint addr_offset_kuseg = 0;
		PRINT_DEBUG("The KUSEG part of the block will start from the %u-th \
frame of size %u", addr_offset_kuseg, frame_size);

		uint count_kuseg = count - count_kseg;
		// is the count correct?
		ASSERT(count_kuseg == (addr + (count * frame_size) - KSEG0_SIZE) 
			                     / frame_size);
		// is the address correct? (i.e. is the addr_offset within the range
		// of bitmap offsets for this frame size?)
		ASSERT(addr_offset_kuseg + count_kuseg 
			<= ( offset(smallerFrameSize(frame_size), &m_buddyInfoKuseg)
			      - offset(frame_size, &m_buddyInfoKuseg) ));
		
		// how many full frames of this size are there, starting from this addr
		uint used_kseg = fullFrames( 
			offset(frame_size, &m_buddyInfoKseg) + addr_offset_kseg, 
			count, frame_size, KSEG);
		PRINT_DEBUG("Found %u used frames from %u in KSEG.\n", 
			used_kseg, count_kuseg);
		
		uint used_kuseg = fullFrames( 
			offset(frame_size, &m_buddyInfoKuseg) + addr_offset_kuseg, 
			count, frame_size, KUSEG);
		PRINT_DEBUG("Found %u used frames from %u in KUSEG.\n", 
			used_kuseg, count_kuseg);

		if ((used_kseg == count_kseg) && (used_kuseg == count_kuseg)) {
			PRINT_DEBUG("Found enough allocated frames. Setting them free.\n");
			setFramesAsFree( 
				offset(frame_size, &m_buddyInfoKseg) + addr_offset_kseg,
				frame_size, count, KSEG);
			setFramesAsFree( 
				offset(frame_size, &m_buddyInfoKuseg) + addr_offset_kuseg,
				frame_size, count, KUSEG);
			return true;
		} else {
			PRINT_DEBUG("Not enough allocated frames found, exiting...\n");
			return false;
		}

	} else {
		Bitset* buddy_map;
		InfoHashTable* info_table;
		AddressType type;

		// determine what map we have to use:
		if (addr >= KSEG0_SIZE) {
			// the whole block lies within the KUSEG segment
			type = KUSEG;
			info_table = &m_buddyInfoKuseg;
			buddy_map = m_buddyMapKuseg;
			addr -= KSEG0_SIZE;
		} else {
			// the whole block lies within the KSEG segment
			type = KSEG;
			info_table = &m_buddyInfoKseg;
			buddy_map = m_buddyMapKseg;
		}

		ASSERT(type == KSEG || type == KUSEG);
		ASSERT(info_table == &m_buddyInfoKseg || info_table == &m_buddyInfoKuseg);
		ASSERT(buddy_map == m_buddyMapKseg || buddy_map == m_buddyMapKuseg);
		
		PRINT_DEBUG("Checking if there is enough space allocated...\n");
		// else check if there are enough used frames
		uint local_offset = addr / frame_size;

		//if ((uintptr_t)address == 0x01000000)
		//	printf("The address points to a frame of size %u with offset %u in map %s\n", 
		//	frame_size, local_offset, (type == KSEG) ? "KSEG" : "KUSEG");
		PRINT_DEBUG("The address points to a frame of size %u with offset %u in map %s\n", 
			frame_size, local_offset, (type == KSEG) ? "KSEG" : "KUSEG");	
		
		uint used = fullFrames( offset(frame_size, info_table) + local_offset, 
			count, frame_size, type);
		PRINT_DEBUG("Found %u used frames\n", used);

		if (used >= count) {
			// this check will change when full has "enough" param.

			// there are enough allocated frames, so free them
			setFramesAsFree( offset(frame_size, info_table) + local_offset,
				frame_size, count, type);
			return true;
		} else {
			return false;
		}

	}

	/* TODO:
	 * What if there are enough used frames of this size, but not all of the
	 * corresponding smallest frames are used??? Should I free the frames? Or 
	 * return an error?
	 */

	ASSERT(false);
	return false;
}

/*----------------------------------------------------------------------------*/

template <uint N>
uint FrameAllocator<N>::defaultFrame( uint* search_counts, 
	const uint frame_size, const InfoHashTable* info_table) const
{
	uint fr_size = frame_size;
	uint count = search_counts[hash(frame_size, HASH_RANGE)];
	while (count > 1 && fr_size < m_maxFrameSize 
		&& (freeFrames(fr_size, info_table) > count) ) {

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
	if ( (freeFrames(fr_size, info_table) < count) && fr_size > frame_size) {
		fr_size /= FRAME_STEP;
	} else {
		// save the last search_count
		search_counts[hash(fr_size, HASH_RANGE)] = count;
	}
	return fr_size;
}

/*----------------------------------------------------------------------------*/

template <uint N>
void FrameAllocator<N>::setFramesAsUsed( const uint global_offset, 
	const uint frame_size, const uint count, const AddressType type )
{

	/*--------------------------------------------------------------------------
	  Set helping variables
	--------------------------------------------------------------------------*/

	Bitset* buddy_map;
	InfoHashTable* info_table;

	if (type == KSEG) {
		buddy_map = m_buddyMapKseg;
		info_table = &m_buddyInfoKseg;
	} else {
		buddy_map = m_buddyMapKuseg;
		info_table = &m_buddyInfoKuseg;
	}

	/*--------------------------------------------------------------------------
	  Mark given frames as used in the map
	--------------------------------------------------------------------------*/

	PRINT_DEBUG("Setting %u frames of size %u as used in map %s, starting\
 from global offset %u\n", 
		count, frame_size, (type == KSEG) ? "KSEG" : "KUSEG", global_offset);

	uint local_offset = global_offset - offset(frame_size, info_table);
	PRINT_DEBUG("Starting from %u-th frame of size %u\n", local_offset, 
		frame_size);
	
	// set the given frames as used
	buddy_map->bits(global_offset, count, true);
	// decrease the count of free frames saved
	freeFrames(frame_size, info_table) -= count;

	/*--------------------------------------------------------------------------
	  Mark all their "children" and "children of their children", etc.
	--------------------------------------------------------------------------*/

	PRINT_DEBUG("Setting their children as used..\n");

	uint fr_size = frame_size;
	uint cnt = count;
	while (fr_size > MIN_FRAME_SIZE) {
		cnt *= FRAME_STEP;

		PRINT_DEBUG("Setting %u frames of size %u as used\n", 
			cnt, smallerFrameSize(fr_size));
		buddy_map->bits(offsetOfChild(local_offset, fr_size, info_table), 
			cnt, true);

		fr_size /= FRAME_STEP;
		ASSERT(fr_size >= MIN_FRAME_SIZE);

		freeFrames(fr_size, info_table) -= cnt;
		local_offset *= 4;
	}

	/*--------------------------------------------------------------------------
	  Mark all their "parents" and "parents of their parents", etc.
	  This may end when no free parents are marked as used
	--------------------------------------------------------------------------*/

	PRINT_DEBUG("Setting their parents as used..\n");

	local_offset = global_offset - offset(frame_size, info_table);
	fr_size = frame_size;
	cnt = count;
	uint new_used = count;
	while (fr_size < m_maxFrameSize && new_used > 0) {

		/*
		 * Determine the number of affected parents
		 */

		uint first_parent = parent(local_offset);
		PRINT_DEBUG("First parent, offset: %u\n", first_parent);
		
		ASSERT(offsetOfParent(local_offset, fr_size, info_table) 
			== offset(largerFrameSize(fr_size), info_table) + first_parent);

		// relative offset of the last parent
		uint last_parent = parent(local_offset + cnt - 1);
		PRINT_DEBUG("Last parent, offset: %u\n", last_parent);

		ASSERT(offsetOfParent(local_offset + cnt - 1, fr_size, info_table) 
		        == offset(largerFrameSize(fr_size), info_table) + last_parent);

		// number of parents
		cnt = last_parent - first_parent + 1;

		/*
		 * Set the parents as used
		 */

		fr_size *= FRAME_STEP;

		PRINT_DEBUG("Setting %u frames of size %u as used\n", 
			cnt, fr_size);

		ASSERT(fr_size <= m_maxFrameSize);

		// the number of parents we have to set as used, which were free before
		new_used = cnt;
		// only the first and last parent could have been free

		if (new_used > 0 && 
			buddy_map->bit(offset(fr_size, info_table) + first_parent))
			--new_used;
		if (new_used > 0 && 
			last_parent != first_parent && 
			buddy_map->bit(offset(fr_size, info_table) + last_parent))
			--new_used;

		PRINT_DEBUG("Setting %u frames from offset %u as used\n",
			cnt, offset(fr_size, info_table) + first_parent);
		if (new_used > 0) {
			buddy_map->bits(
				offset(fr_size, info_table) + first_parent, cnt, true);
			freeFrames(fr_size, info_table) -= new_used;
		}		

		local_offset /= 4;
	}
}

/*----------------------------------------------------------------------------*/

template <uint N>
inline void FrameAllocator<N>::setFramesAsFree( const uint global_offset, 
	const uint frame_size, const uint count, const AddressType type )
{
	/*--------------------------------------------------------------------------
	  Set helping variables
	--------------------------------------------------------------------------*/

	Bitset* buddy_map;
	InfoHashTable* info_table;

	if (type == KSEG) {
		buddy_map = m_buddyMapKseg;
		info_table = &m_buddyInfoKseg;
	} else {
		buddy_map = m_buddyMapKuseg;
		info_table = &m_buddyInfoKuseg;
	}

	/*--------------------------------------------------------------------------
	  Mark given frames as free in the buddy map
	--------------------------------------------------------------------------*/
	
	PRINT_DEBUG("Setting %u frames of size %u as free in map %s\n",
		count, frame_size, (buddy_map == m_buddyMapKseg) ? "KSEG" : "KUSEG");

	uint local_offset = global_offset - offset(frame_size, info_table);

	// set the given frames as free in Kseg map
	buddy_map->bits(global_offset, count, false);
	// increase the count of free frames saved
	freeFrames(frame_size, info_table) += count;

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
		buddy_map->bits(offsetOfChild(local_offset, fr_size, info_table), 
			cnt, false);

		fr_size /= FRAME_STEP;
		ASSERT(fr_size >= MIN_FRAME_SIZE);

		freeFrames(fr_size, info_table) += cnt;
		local_offset *= 4;
	}

	/*--------------------------------------------------------------------------
	  Mark all their "parents" and "parents of their parents", etc.
	  This case is different from marking them as used
	  In parent we must check if all of his children are free, but it's
	  sufficient to check only the first and the last parent
	--------------------------------------------------------------------------*/

	PRINT_DEBUG("Setting their parents free...\n\n");

	local_offset = global_offset - offset(frame_size, info_table);
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
		
		ASSERT(offsetOfParent(local_offset, fr_size, info_table) 
			== offset(largerFrameSize(fr_size), info_table) + first_parent);
		ASSERT(offsetOfParent(child_first, fr_size, info_table) 
			== offset(fr_size * FRAME_STEP, info_table) + first_parent);

		// relative offset of the last parent's first child
		uint child_last = roundDown(local_offset + cnt - 1, FRAME_STEP);
		PRINT_DEBUG("First child of the last parent, offset: %u\n", child_last);
		// relative offset of the last parent
		uint last_parent = child_last / FRAME_STEP;
		//uint last_parent = parent(local_offset + cnt - 1);
		PRINT_DEBUG("Last parent, offset: %u\n", last_parent);

		ASSERT(offsetOfParent(local_offset + cnt - 1, fr_size, info_table) 
		        == offset(largerFrameSize(fr_size), info_table) + last_parent);
		ASSERT(offsetOfParent(child_last, fr_size, info_table) 
		        == offset(fr_size * FRAME_STEP, info_table) + last_parent);

		// number of parents
		cnt = last_parent - first_parent + 1;
		PRINT_DEBUG("Number of parents: %u\n", cnt);

		/*
		 * Check the first parent and eventually set it as free
		 */

		PRINT_DEBUG("Checking 1st parent's children...\n");

		bool used = buddy_map->bit(offset(fr_size, info_table) + child_first);
		PRINT_DEBUG("1st parent's first child is %s\n", (used) ? "used" : "free");
		// check all other children until there is at least one used
		for (uint i = 1; i < FRAME_STEP && !used; ++i) {
			used = used || buddy_map->bit(
				offset(fr_size, info_table) + child_first + i);
			PRINT_DEBUG("1st parent's %u-th child is %s\n", 
				i + 1,
				(buddy_map->bit(offset(fr_size, info_table) + child_first + i)) 
				? "used" 
				: "free");
		}

		PRINT_DEBUG("Setting 1st parent as %s\n", (used) ? "used" : "free" );
		if (!used) {
			buddy_map->bit( offset(largerFrameSize(fr_size), info_table) 
			                 + first_parent, used);
			++freeFrames(largerFrameSize(fr_size), info_table);
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

			used = buddy_map->bit(offset(fr_size, info_table) + child_last);
			PRINT_DEBUG("Last parent's first child is %s\n", 
				(used) ? "used" : "free");

			for (uint i = 1; i < FRAME_STEP && !used; ++i) {
				used = used || buddy_map->bit(
					offset(fr_size, info_table) + child_last + i);
				PRINT_DEBUG("Last parent's %u-th child is %s\n", 
					i + 1,
					(buddy_map->bit(offset(fr_size, info_table) + child_last + i)) 
					? "used" 
					: "free");
			}

			PRINT_DEBUG("Setting last parent as %s\n", (used) ? "used" : "free" );
			if (!used) {
				ASSERT(offsetOfParent(child_last, fr_size, info_table) 
				  == offset(largerFrameSize(fr_size), info_table) + last_parent);
				buddy_map->bit( 
					offsetOfParent(child_last, fr_size, info_table), used);
				++freeFrames(largerFrameSize(fr_size), info_table);
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
			buddy_map->bits(
				offsetOfParent(local_offset, fr_size, info_table) + 1, 
				cnt - 2, false);
			freeFrames(largerFrameSize(fr_size), info_table) += cnt - 2;
			freed = true;
		}

		fr_size *= FRAME_STEP;
		local_offset /= FRAME_STEP;

		PRINT_DEBUG("\n");
	}
}

/*----------------------------------------------------------------------------*/

template <uint N>
inline uint FrameAllocator<N>::findFirstEmpty( 
	const uint offset, const Bitset* buddy_map ) const
{
	/*PRINT_DEBUG("findFirstEmpty started...offset %u, bitmap: %p, which is: %s\n",
	offset, buddy_map, (buddy_map == m_buddyMapKseg) ? "KSEG" : "KUSEG");*/

	uint f = offset + buddy_map->full(offset);
	/*PRINT_DEBUG("Returning %u\n", f);*/
	return f;
}

/*---------------------------------------------------------------------------*/

template <uint N>
inline uint FrameAllocator<N>::findFirstFull( 
	const uint offset, const Bitset* buddy_map ) const
{
	return offset + buddy_map->empty(offset);
}

/*---------------------------------------------------------------------------*/

template <uint N>
inline uint FrameAllocator<N>::hash( 
	const uint key, const uint range ) const 
{ 
	return (key % range);
}

/*----------------------------------------------------------------------------*/

template <uint N>
inline uint& FrameAllocator<N>::offset( 
	const uint frame_size, InfoHashTable* info_table )
{ 
	ASSERT(frame_size >= (MIN_FRAME_SIZE / FRAME_STEP));
	return (*info_table)[hash(frame_size, HASH_RANGE)][1];
}

/*----------------------------------------------------------------------------*/

template <uint N>
inline uint FrameAllocator<N>::offset( 
	const uint frame_size, const InfoHashTable* info_table ) const
{ 
	ASSERT(frame_size >= (MIN_FRAME_SIZE / FRAME_STEP));
	return (*info_table)[hash(frame_size, HASH_RANGE)][1];
}

/*----------------------------------------------------------------------------*/

template <uint N>
inline uint& FrameAllocator<N>::freeFrames( 
	const uint frame_size, InfoHashTable* info_table )
{ 
	return (*info_table)[hash(frame_size, HASH_RANGE)][0];
}

/*----------------------------------------------------------------------------*/

template <uint N>
inline uint FrameAllocator<N>::freeFrames( 
	const uint frame_size, const InfoHashTable* info_table ) const
{ 
	return (*info_table)[hash(frame_size, HASH_RANGE)][0];
}

/*---------------------------------------------------------------------------*/

template <uint N>
inline uint FrameAllocator<N>::offsetOfChild( const uint local_offset, 
	const uint frame_size, const InfoHashTable* info_table ) const
{
	return offset(smallerFrameSize(frame_size), info_table) 
	                   + (local_offset * FRAME_STEP);
}

/*---------------------------------------------------------------------------*/

template <uint N>
inline uint FrameAllocator<N>::offsetOfParent( const uint local_offset, 
	const uint frame_size, const InfoHashTable* info_table ) const
{
	return offset(largerFrameSize(frame_size), info_table) 
	                   + (local_offset / FRAME_STEP);
}

/*---------------------------------------------------------------------------*/

template <uint N>
inline uint FrameAllocator<N>::emptyFrames( const uint global_offset,
	const uint count, const uint frame_size, const AddressType type ) const
{
	return (type == KSEG)
	        ? emptyFramesKseg(global_offset, count, frame_size)
	        : emptyFramesKuseg(global_offset, count, frame_size);
}

/*---------------------------------------------------------------------------*/

template <uint N>
inline uint FrameAllocator<N>::emptyFramesKseg( const uint global_offset,
	const uint count, const uint frame_size ) const
{
	return m_buddyMapKseg->empty(global_offset, 
			min(count, (offset(smallerFrameSize(frame_size), &m_buddyInfoKseg) 
		             - global_offset)) );
}

/*---------------------------------------------------------------------------*/

template <uint N>
inline uint FrameAllocator<N>::emptyFramesKuseg( const uint global_offset,
	const uint count, const uint frame_size ) const
{
	return m_buddyMapKuseg->empty(global_offset, 
			min(count, (offset(smallerFrameSize(frame_size), &m_buddyInfoKuseg) 
		             - global_offset)) );
}
/*---------------------------------------------------------------------------*/

template <uint N>
inline uint FrameAllocator<N>::fullFrames( const uint global_offset,
		const uint count, const uint frame_size, const AddressType type ) const
{
	return (type == KSEG)
	        ? fullFramesKseg(global_offset, count, frame_size)
			: fullFramesKuseg(global_offset, count, frame_size);
}

/*---------------------------------------------------------------------------*/

template <uint N>
inline uint FrameAllocator<N>::fullFramesKseg( 
	const uint global_offset, const uint count, const uint frame_size ) const
{
	return m_buddyMapKseg->full(global_offset, 
		min(count, (offset(smallerFrameSize(frame_size), &m_buddyInfoKseg) 
					- global_offset)) );
}

/*---------------------------------------------------------------------------*/

template <uint N>
inline uint FrameAllocator<N>::fullFramesKuseg( 
	const uint global_offset, const uint count, const uint frame_size ) const
{
	return m_buddyMapKuseg->full(global_offset, 
		min(count, (offset(smallerFrameSize(frame_size), &m_buddyInfoKuseg) 
					- global_offset)) );
}

/*---------------------------------------------------------------------------*/

template <uint N>
inline uint FrameAllocator<N>::min( const uint x, const uint y ) const
{
	return (x < y) ? x : y;
}

/*---------------------------------------------------------------------------*/

template <uint N>
inline uint FrameAllocator<N>::parent( const uint local_offset ) const
{
	return roundDown(local_offset, FRAME_STEP) / FRAME_STEP;
}

/*---------------------------------------------------------------------------*/

template <uint N>
inline uint FrameAllocator<N>::firstChild( const uint local_offset ) const
{
	return local_offset * FRAME_STEP;
}

/*---------------------------------------------------------------------------*/

template <uint N>
inline uint FrameAllocator<N>::largerFrameSize( 
	const uint frame_size ) const
{
	return frame_size * FRAME_STEP;
}

/*---------------------------------------------------------------------------*/

template <uint N>
inline uint FrameAllocator<N>::smallerFrameSize( 
	const uint frame_size ) const
{
	return frame_size / FRAME_STEP;
}

/*---------------------------------------------------------------------------*/

template <uint N>
inline uintptr_t FrameAllocator<N>::getAddress( const uint off, 
	const uint frame_size, const InfoHashTable* info_table ) const
{
	ASSERT(frame_size >= MIN_FRAME_SIZE);
	return ( (off - offset(frame_size, info_table)) * frame_size );
}

/*---------------------------------------------------------------------------*/

template <uint N>
inline uint FrameAllocator<N>::theEnd( 
	const InfoHashTable* info_table ) const
{
	ASSERT((info_table == &m_buddyInfoKseg) || (info_table == &m_buddyInfoKuseg));
	return (offset(smallerFrameSize(MIN_FRAME_SIZE), info_table));
}

/*---------------------------------------------------------------------------*/

template <uint N>
void FrameAllocator<N>::checkStructures() const
{
	printFree(m_buddyMapKseg, &m_buddyInfoKseg); 
	if (m_buddyMapKuseg) 
		printFree(m_buddyMapKuseg, &m_buddyInfoKuseg);
}

/*---------------------------------------------------------------------------*/

template <uint N>
void FrameAllocator<N>::printFree( 
	const Bitset* buddy_map, const InfoHashTable* info_table ) const
{
	PRINT_DEBUG("Checking the buddy bitmap for %s...\n",
		(buddy_map == m_buddyMapKseg) ? "KSEG" : "KUSEG" );

	// check the buddy map
	for (uint i = MIN_FRAME_SIZE; i <= m_maxFrameSize; i *= FRAME_STEP) {
		uint free = 0;
		for (uint j = offset(i, info_table); 
			 j < offset(smallerFrameSize(i), info_table);
			 ++j)
			free += !buddy_map->bit(j);

		PRINT_DEBUG("%u empty frames of size %u\n", 
			free,
			i
		);
		PRINT_DEBUG("\tFirst frame of this size set: %s\n", 
			(buddy_map->bit(offset(i, info_table))) ? "Yes" : "No");
		PRINT_DEBUG("\tLast frame of this size set: %s\n", 
			(buddy_map->bit(offset(smallerFrameSize(i), info_table) - 1)) 
			? "Yes" : "No");
		if (buddy_map == m_buddyMapKseg) {
			PRINT_DEBUG("\tLast blocked frame of this size, what is %d, set: %s\n", 
				(roundUp(m_endOfBlockedFrames, i) / i) - 1,
				(buddy_map->bit(offset(i, info_table) + 
					(roundUp(m_endOfBlockedFrames, i) / i) - 1)) 
				? "Yes" : "No");
		}
	}

	PRINT_DEBUG("Checking the hash map for %s...\n",
		(buddy_map == m_buddyMapKseg) ? "KSEG" : "KUSEG");

	for (uint i = MIN_FRAME_SIZE; i <= m_maxFrameSize; i *= FRAME_STEP) {
		PRINT_DEBUG("%u empty frames of size %u\n", 
			freeFrames(i, info_table),
			i
		);
	}

	for (uint i = MIN_FRAME_SIZE; i <= m_maxFrameSize; i *= FRAME_STEP) {
		uint free = 0;
		for (uint j = offset(i, info_table); 
			 j < offset(smallerFrameSize(i), info_table);
			 ++j)
			free += !buddy_map->bit(j);
		ASSERT(freeFrames(i, info_table) == free);
	}

	PRINT_DEBUG("DONE\n");
}

/*---------------------------------------------------------------------------*/

template <uint N>
bool FrameAllocator<N>::checkBitmap( 
	const Bitset* buddy_map, const InfoHashTable* info_table ) const
{
	bool ok = true;
	uint frame_size = m_maxFrameSize;
	// for all frames which have "children"
	for (uint i = 0; i < offset(MIN_FRAME_SIZE, info_table); ++i)
	{
		// if we're on the next level
		if (i > offset(smallerFrameSize(frame_size), info_table))
			// adjust the frame size
			frame_size /= FRAME_STEP;

		// global offset of the first child
		uint first_child = offset(smallerFrameSize(frame_size), info_table)
			               + firstChild(i - offset(frame_size, info_table));
		bool used = buddy_map->bit(first_child);
		for (uint j = 1; j < FRAME_STEP; ++j)
			used = used || buddy_map->bit(first_child + j);

		// if the frame is used but none of its children is used
		if (buddy_map->bit(i) && !used) {
			PRINT_DEBUG("Parent set as used but all children not!\n");
			PRINT_DEBUG("\tFrame size: %u, local offset: %u\n",
				frame_size, i - offset(frame_size, info_table));
			ok = false;
		// if the frame is free, but some of its children is used
		} else if (!buddy_map->bit(i) && used) {
			PRINT_DEBUG("Parent set as free but one or more children are used!\n");
			PRINT_DEBUG("\tFrame size: %u, local offset: %u\n",
				frame_size, i - offset(frame_size, info_table));
			ok = false;
		}	// else it's ok, so continue
	}
	return ok;
}

/*---------------------------------------------------------------------------*/

template <uint N>
bool FrameAllocator<N>::checkBitmaps() const
{
	bool ok = checkBitmap(m_buddyMapKseg, &m_buddyInfoKseg);
	ok = ok && checkBitmap(m_buddyMapKuseg, &m_buddyInfoKuseg);
	ASSERT(ok);
	return ok;
}

/*---------------------------------------------------------------------------*/

template <uint N>
void FrameAllocator<N>::test()
{
	// test allocation at KSEG segment
	printf("Testing allocation in KSEG...\n");
	printf("KSEG map pointer: %p\n", m_buddyMapKseg);
	bool test_kseg = testAllocationAtSegment(KSEG);
	
	bool test_kuseg = false;
	
	printf("Testing allocation in KUSEG...\n");
	printf("KUSEG map pointer: %p\n", m_buddyMapKuseg);

	if (m_buddyMapKuseg) {
		// test allocation in KUSEG segment
		test_kuseg = testAllocationAtSegment(KUSEG);
	}

	printf("Testing allocation at address..\n");
	bool test_addr = testAllocationAtAddress();

	printf("Test: allocation in KSEG...%s\n", (test_kseg) ? "OK" : "Failed" );
	if (m_buddyMapKuseg) {
		printf("Test: allocation in KUSEG...%s\n", 
			(test_kuseg) ? "OK" : "Failed" );
	}
	printf("Test: allocation at address...%s\n", (test_addr) ? "OK" : "Failed" );

	if (test_kseg && test_kuseg && test_addr)
		printf("Tests PASSED.\n");
	else 
		printf("Tests FAILED.\n");

}

/*---------------------------------------------------------------------------*/

template <uint N>
bool FrameAllocator<N>::testAllocationAtAddress()
{
	bool ok = true;

	for (uint loop = 0; loop < 100; ++loop) {

		uintptr_t address = 0;

		PRINT_DEBUG("Loop %u\n", loop);
		
		uint allocated_frames = 0;

		printf("Allocation...\n");
	
	
		// while we're not at the end of the physical memory
		while (address < m_memorySize - 0x500000) {
			
			uint exp = (rand() % 4);

			uint frame_size = MIN_FRAME_SIZE;
			while (exp-- > 0 && frame_size < m_maxFrameSize) {
				frame_size *= FRAME_STEP;
			}

			PRINT_DEBUG("Address: %x\n", address);

			if (roundUp(address, frame_size) >= m_memorySize)
				continue;

			address = roundUp(address, frame_size);
			PRINT_DEBUG("Address: %x\n", address);

			PRINT_DEBUG("Count range: %u\n",
				(roundUp((offset(smallerFrameSize(frame_size), &m_buddyInfoKseg) 
					 - offset(frame_size, &m_buddyInfoKseg)), 100) / 100)
				 + (roundUp((offset(smallerFrameSize(frame_size), 
						&m_buddyInfoKuseg) 
					 - offset(frame_size, &m_buddyInfoKuseg)), 100) / 100)
				);
			
			// choose the number of frames (less than all frames of this size)
			uint count = ( rand() 
				%  ((roundUp((offset(smallerFrameSize(frame_size), &m_buddyInfoKseg) 
					 - offset(frame_size, &m_buddyInfoKseg)), 100) / 100)
					 +
					 (roundUp((offset(smallerFrameSize(frame_size), &m_buddyInfoKuseg) 
					 - offset(frame_size, &m_buddyInfoKuseg)), 100) / 100)
					 ) )+ 1;
			// TODO: adjust the test by keeping the address of allocated block
			// together with frame size and number of frames
			// and check if all levels of btmap are properly set
			
			//void* address = NULL;
			// if the allocation was successfull
			PRINT_DEBUG("\tFrame size: %u, count: %u\n", frame_size, count);

			if (count == allocateAtAddress((void*)address, count, frame_size)) {
				PRINT_DEBUG("Allocation successfull..\n");
				// increase the count of allocated 4kB frames;
				allocated_frames += (count * (frame_size / MIN_FRAME_SIZE));
				address += (count * frame_size);
				address += (rand() % (roundUp(m_memorySize - 0x500000, 100) / 100));
			} else {
				PRINT_DEBUG("Allocation unsuccessfull..\n");
				address += (rand() % (roundUp(m_memorySize - 0x500000, 100) / 100));
			} // end if
		}	// end while

		checkBitmaps();
		checkStructures();

		printf("Deallocation...\n");

		uint freed_frames = 0;

		// MUST BE CHANGED FOR KUSEG
		// the allocated frames could be in KSEG also!

		PRINT_DEBUG("Counting allocated frames in KUSEG...\n");
		PRINT_DEBUG("Starting from offset %u, ending at offset %u...\n",
			offset(MIN_FRAME_SIZE, &m_buddyInfoKuseg), 
			theEnd(&m_buddyInfoKuseg) - 1);
		//if (type == KUSEG) {
			for (uint i = offset(MIN_FRAME_SIZE, &m_buddyInfoKuseg); 
			     i < theEnd(&m_buddyInfoKuseg); ++i) {
				if (m_buddyMapKuseg->bit(i)) {
					if (i == offset(MIN_FRAME_SIZE, &m_buddyInfoKuseg)) {
						PRINT_DEBUG("Freeing 1st frame...address: %p\n",
						(void*)(((i-offset(MIN_FRAME_SIZE, &m_buddyInfoKuseg))*MIN_FRAME_SIZE) + KSEG0_SIZE));
					}
					if (frameFree((void*)(((i-offset(MIN_FRAME_SIZE, &m_buddyInfoKuseg))*MIN_FRAME_SIZE) + KSEG0_SIZE), 1, MIN_FRAME_SIZE))
						freed_frames++;
					//freeFrames(MIN_FRAME_SIZE, &m_buddyInfoKuseg)++;
				}
			}
	//	} // end if

		// if either case, check the KSEG map
		PRINT_DEBUG("Counting allocated frames in KSEG...\n");
		PRINT_DEBUG("Starting from offset %u, ending at offset %u...\n",
			offset(MIN_FRAME_SIZE, &m_buddyInfoKseg), 
			theEnd(&m_buddyInfoKseg) - 1);
		for (uint i = offset(MIN_FRAME_SIZE, &m_buddyInfoKseg); 
		     i < theEnd(&m_buddyInfoKseg); ++i) {
			/*bool freed = false;*/ /*frameFree((void*)(i * MIN_FRAME_SIZE), 1, MIN_FRAME_SIZE);*/
			if (m_buddyMapKseg->bit(i) 
				&& ((i-offset(MIN_FRAME_SIZE, &m_buddyInfoKseg))*MIN_FRAME_SIZE) 
					>= m_endOfBlockedFrames) {
				if (frameFree((void*)((i-offset(MIN_FRAME_SIZE, &m_buddyInfoKseg))*MIN_FRAME_SIZE), 1, MIN_FRAME_SIZE))
					freed_frames++;
			}
				
		} // end for

		if (allocated_frames == freed_frames) {
			printf("Loop %u successfull! Allocated frames: %u, freed\
frames: %u\n", loop, allocated_frames, freed_frames);
			//checkBitmaps();
			//checkStructures();
		} else {
			printf("Loop %u unsuccessfull! Allocated frames: %u, freed\
frames: %u\n", loop, allocated_frames, freed_frames);
			checkBitmaps();
			checkStructures();
			ASSERT(false);
			ok = false;
		}	// end if
	}
	
	return ok;

}

/*---------------------------------------------------------------------------*/

template <uint N>
bool FrameAllocator<N>::testAllocationAtSegment( const AddressType type )
{
	/*
	 * We will allocate random sized blocks of random sized frames
	 * and keep the count of currently allocated 4kB frames.
	 * Afterwards, we'll deallocate smallest frames one-by one and count the
	 * freed frames. If the counts are equal, the allocation and deallocation
	 * are ok.
	 * In each step we'll check the integrity of the map using checkBitmap().
	 */

	InterruptDisabler interrupts;

	bool ok = true;

	PRINT_DEBUG("Testing allocation in segment %s...\n", 
		(type == KSEG) ? "KSEG" : "KUSEG");

	InfoHashTable* info_table = (type == KSEG)
	                           ? &m_buddyInfoKseg
	                           : &m_buddyInfoKuseg;
	/*Bitset* buddy_map = (type == KSEG)
	                           ? m_buddyMapKseg
	                           : m_buddyMapKuseg;*/


	//const uint LOOPS = 100;

	for (uint loop = 0; loop < 100; ++loop) {
		PRINT_DEBUG("Loop %u\n", loop);
		
		uint allocations = rand() % 100;
		uint allocated_frames = 0;

		printf("Allocation...\n");

		// randomly allocate blocks of frames
		for (uint a = 0; a < allocations; ++a) {
			PRINT_DEBUG("Allocation #%u\n", a);
			// choose a random frame size
			uint exp = rand() % 6;
			
			uint frame_size = MIN_FRAME_SIZE;
			while (exp-- > 0 && frame_size < m_maxFrameSize) {
				frame_size *= FRAME_STEP;
			}
			
			// choose the number of frames (less than all frames of this size)
			uint count = ( rand() 
				%  (roundUp((offset(smallerFrameSize(frame_size), info_table) 
				     - offset(frame_size, info_table)), 2) / 2)) + 1;
			// TODO: adjust the test by keeping the address of allocated block
			// together with frame size and number of frames
			// and check if all levels of btmap are properly set
			
			void* address = NULL;
			// if the allocation was successfull
			PRINT_DEBUG("\tFrame size: %u, count: %u\n", frame_size, count);
			if (count == ((type == KSEG)
				? allocateAtKseg0(&address, count, frame_size)
				: allocateAtKuseg(&address, count, frame_size))) {
				// increase the count of allocated 4kB frames;
				allocated_frames += (count * (frame_size / MIN_FRAME_SIZE));
			}
			// if the allocation was not successfull, just continue the loop
	
			//checkBitmaps();
			//checkStructures();
		}

		checkBitmaps();
		checkStructures();

		printf("Deallocation...\n");

		uint freed_frames = 0;

		// MUST BE CHANGED FOR KUSEG
		// the allocated frames could be in KSEG also!

		PRINT_DEBUG("Counting allocated frames in KUSEG...\n");
		PRINT_DEBUG("Starting from offset %u, ending at offset %u...\n",
			offset(MIN_FRAME_SIZE, &m_buddyInfoKuseg), 
			theEnd(&m_buddyInfoKuseg) - 1);
		if (type == KUSEG) {
			for (uint i = offset(MIN_FRAME_SIZE, &m_buddyInfoKuseg); 
			     i < theEnd(&m_buddyInfoKuseg); ++i) {
				if (m_buddyMapKuseg->bit(i)) {
					if (i == offset(MIN_FRAME_SIZE, &m_buddyInfoKuseg)) {
						PRINT_DEBUG("Freeing 1st frame...address: %p\n",
						(void*)(((i-offset(MIN_FRAME_SIZE, &m_buddyInfoKuseg))*MIN_FRAME_SIZE) + KSEG0_SIZE));
					}
					if (frameFree((void*)(((i-offset(MIN_FRAME_SIZE, &m_buddyInfoKuseg))*MIN_FRAME_SIZE) + KSEG0_SIZE), 1, MIN_FRAME_SIZE))
						freed_frames++;
				}
			}
		}

		// if either case, check the KSEG map
		PRINT_DEBUG("Counting allocated frames in KSEG...\n");
		PRINT_DEBUG("Starting from offset %u, ending at offset %u...\n",
			offset(MIN_FRAME_SIZE, &m_buddyInfoKseg), 
			theEnd(&m_buddyInfoKseg) - 1);
		for (uint i = offset(MIN_FRAME_SIZE, &m_buddyInfoKseg); 
		     i < theEnd(&m_buddyInfoKseg); ++i) {
			if (m_buddyMapKseg->bit(i) 
				&& ((i-offset(MIN_FRAME_SIZE, &m_buddyInfoKseg))*MIN_FRAME_SIZE) 
					>= m_endOfBlockedFrames) {
				if (frameFree((void*)((i-offset(MIN_FRAME_SIZE, &m_buddyInfoKseg))*MIN_FRAME_SIZE), 1, MIN_FRAME_SIZE))
					freed_frames++;
			}
				
		}

		if (allocated_frames == freed_frames) {
			printf("Loop %u successfull! Allocated frames: %u, freed\
frames: %u\n", loop, allocated_frames, freed_frames);
			//checkBitmaps();
			//checkStructures();
		} else {
			printf("Loop %u unsuccessfull! Allocated frames: %u, freed\
frames: %u\n", loop, allocated_frames, freed_frames);
			checkBitmaps();
			checkStructures();
			ASSERT(false);
			ok = false;
		}
	}
	
	return ok;
}

/*---------------------------------------------------------------------------*/

typedef FrameAllocator<7> MyFrameAllocator;

#undef PRINT_DEBUG
