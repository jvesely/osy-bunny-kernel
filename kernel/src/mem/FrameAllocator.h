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
 * @brief Contains declaration of class FrameAllocator 
 * and some inline functions as well.
 *
 * Class FrameAllocator provides interface for physical memory 
 * allocation and supports multiple frame sizes.
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
#include "tools.h"
#include "address.h"
#include "Singleton.h"
#include "drivers/Processor.h"

using namespace Processor;

/*!
 * @class FrameAllocator FrameAllocator.h "mem/FrameAllocator.h"
 * 
 * @brief Class FrameAllocator provides interface for physical memory 
 * allocation and supports multiple frame sizes.
 *
 * For storing information about used/free frames, FrameAllocator uses class
 * Bitset, with optimized search functions. It also remembers last freed 
 * block of frames and tries to use it when a new allocation request comes.
 *
 * This class is a singleton, and all calls to it are done using 
 * FrameAllocator::instance()
 */
class FrameAllocator: public Singleton<FrameAllocator>
{
public:

	/*!
	 * @brief Initializes Frame allocator structures. Must be called before 
	 * any frameAlloc() or frameFree() calls, otherwise the results are 
	 * undefined.
	 *
	 * Determines largest frame size which can be used according to the total
	 * memory size. 
	 * 
	 * In case the memory size is not rounded to the smallest frame
	 * size, the rest of the memory (from the end of the last smallest frame)
	 * will be ignored.
	 *
	 * Creates and initializes Frame allocator structures and marks frames
	 * used by these structures and kernel as used.
	 *
	 * @return Virtual address of the end of initially used frames. 
	 * This address is from the KSEG0 segment.
	 */
	uintptr_t init( size_t memory_size, const size_t kernel_end );

	/*!
	 * @brief Tries to allocate @a count frames of type @a frame 
	 * due to bit flags set in @a flags parameter.
	 *
	 * @param address Pointer to a pointer where the <b>physicall address</b> 
	 *                of the allocated block will be saved. If VF_VA_USER flag 
	 *                is set, the function will try to allocate the memory on
	 *                this address.
	 * @param count Count of subsequent frames requested by user to be allocated.
	 * @param frame Type of the frames requested by user to be allocated.
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
	 *                  allocated block of frames.\n
	 *         <i>Count of frames in the largest block</i> of subsequent free 
	 *                  frames of size @a frame in case the allocation was not 
	 *                  successful. @a *address will be the physical address 
	 *                  of this block.
	 */
	uint frameAlloc( void** address, const size_t count, const PageSize frame,
		const uint flags );

	/*!
	 * @brief Tries to allocate @a count frames of type @a frame inside
	 * the KSEG segment of the physical address space (0 - 512 MB).
	 *
	 * @param address Pointer to a pointer where the <b>physicall address</b> 
	 * of the allocated block will be saved.
	 * @param count Count of subsequent frames requested by user to be allocated.
	 * @param frame Type of the frames requested by user to be allocated.
	 *
	 * @note Use this function to avoid using flags in function frameAlloc().
	 * 
	 * @return @a count in case the allocation was successful. In this case
	 *                  @a *address holds physical address of start of the 
	 *                  allocated block of frames.\n
	 *         <i>Count of largest block of subsequent free frames of size 
	 *                  @a frame</i> in case the allocation was not successful.
	 *                  @a *address will be the physical address of this block.
	 */
	inline uint allocateAtKseg0( 
		void** address, const uint count, const PageSize frame );

	/*!
	 * @brief Tries to allocate @a count frames of type @a frame primarily
	 * outside the KSEG segment of the physical address space (512 MB and 
	 * higher).
	 *
	 * If it fails to find enough empty frames in the KUSEG segment, 
	 * the function then tries to allocate them on the border or in the 
	 * KSEG segment.
	 *
	 * @param address Pointer to a pointer where the <b>physicall address</b> 
	 * of the allocated block will be saved.
	 * @param count Count of subsequent frames requested by user to be allocated.
	 * @param frame Type of the frames requested by user to be allocated.
	 *
	 * @note Use this function to avoid using flags in function frameAlloc().
	 * 
	 * @return @a count in case the allocation was successful. In this case
	 *                  @a *address holds physical address of start of the 
	 *                  allocated block of frames.\n
	 *         <i>Count of largest block of subsequent free frames of size 
	 *                  @a frame</i> in case the allocation was not successful.
	 *                  @a *address will be the physical address of this block.
	 */
	uint allocateAtKuseg( 
		void** address, const uint count, const PageSize frame );

	/*!
	 * @brief Tries to allocate @a count frames of type @a frame at
	 * the given address.
	 *
	 * @param address Pointer to a pointer where the allocated frames should 
	 * start.
	 * @param count Count of subsequent frames requested by user to be allocated.
	 * @param frame Type of the frames requested by user to be allocated.
	 *
	 * @note Use this function to avoid using flags in function frameAlloc().
	 * 
	 * @return @a count in case the allocation was successful.\n
	 *         <i>Count of largest block of subsequent free frames of size 
	 *                  @a frame</i> starting from the given address otherwise.
	 */
	uint allocateAtAddress( 
		const void* address, const uint count, const PageSize frame );

	/*! 
	 * @brief Frees the previously allocated physical memory.
	 *
	 * This function frees any memory which was allocated using frameAlloc(),
	 * allocateAtKseg0(), allocateAtKuseg() or allocateAtAddress() functions.
	 * 
	 * @param address Physical address of the block of frames to be freed.
	 * @param count Count of subsequent frames to be freed.
	 * @param frame Size of the frames to be freed.
	 *
	 * @retval True if the deallocation was successful and the frames were freed.
	 * @retval False in case the address is not within the memory address range,
	 * the address is not aligned at the start of a frame of size @a frame,
	 * not all of the frames were allocated, or @a count is 0.
	 */
	bool frameFree( const void* address, const size_t count, 
		const PageSize frame );

	/*! 
	 * @brief Returns the state of the frame allocator. Always use this 
	 * function to check if FrameAllocator was initialized successfully, 
	 * before calling frameAlloc() or frameFree().
	 */
	inline bool isInitialized() const { return m_initialized; }

private:

	/*
	 * Constants
	 */

	/*! @brief Minimum frame size. */
	static const PageSize MIN_FRAME = PAGE_MIN;	// 8kB

	/*! @brief Maximum frame size. */
	static const PageSize MAX_FRAME = PAGE_MAX;	// 32MB

	/*! @brief Next frame size = (FRAME_STEP * This frame size) */
	static const uint FRAME_STEP = 4;

	/*! @brief Size of the KSEG0 segment. */
	static const uint KSEG0_SIZE = ADDR_SIZE_KSEG0;
	
	/*! @brief Used to index some member variables. */
	enum AddressType { KSEG, KUSEG };

	/*
	 * Member functions
	 */

	/*! @brief Default constructor. Only marks the FrameAllocator as 
	 * not initialized.
	 */
	FrameAllocator(): m_initialized(false) {};

	/*!
	 * @brief Common function for allocation. Called by both allocateAtKseg0()
	 * and allocateAtKuseg().
	 *
	 * For more info see functions allocateAtKseg0() and allocateAtKuseg().
	 *
	 * @param address Pointer to a pointer where the <b>physicall address</b> 
	 * of the allocated block will be saved.
	 * @param count Count of subsequent frames requested by user to be allocated.
	 * @param frame Type of the frames requested by user to be allocated.
	 * @param type Determines the segment where the frames are to be allocated.
	 *
	 * @return 
	 * @a count in case the allocation was successful. In this case
	 * @a *address holds physical address of start of the allocated block 
	 * of frames.\n
	 * <i>Count of largest block of subsequent free frames of size 
	 * @a frame</i> in case the allocation was not successful. @a *address 
	 * will be the physical address of this block.
	 */
	uint allocateAtSegment( void** address, const uint count,
		const PageSize frame, const AddressType type );

	/*! 
	 * @brief Recommends the best frame type to start search for and counts
	 * number of frames of each size which have to be searched for.
	 *
	 * The largest frame size of which there are enough (not necessarily 
	 * subsequent) free frames to satisfy the need is considered the best.
	 *
	 * @param search_counts Array to be filled with counts of frames of each
	 * type from the requested type to the best type, which satisfy the request.
	 * @param frame Requested type of frames.
	 * @param type Requested segment to search for the free frames.
	 *
	 * @return Recommended frame type to search for. Corresponding count of
	 * frames to be searched for is saved in the @a search_counts array
	 * for each frame type.
	 */
	PageSize defaultFrame( uint* search_counts, const PageSize frame,
		const AddressType type ) const;

	/*! 
	 * @brief Marks frames as used in the bitmap.
	 *
	 * @param global_offset Start of the frames in the whole bitmap to be 
	 * marked as used.
	 * @param frame Type of the frames to be marked as used.
	 * @param count How many frames should be marked.
	 * @param type Determines the bitmap to use.
	 */
	void setFramesAsUsed( const uint global_offset, const PageSize frame,
		const uint count, const AddressType type );

	/*! 
	 * @brief Marks frames as free in the bitmap.
	 *
	 * @param global_offset Start of the frames in the whole bitmap to be 
	 * marked as free.
	 * @param frame Type of the frames to be marked as free.
	 * @param count How many frames should be marked.
	 * @param type Determines the bitmap to use.
	 */
	void setFramesAsFree( const uint global_offset, 
		const PageSize frame, const uint count, const AddressType type );

	/*! 
	 * @brief Finds the first empty frame starting from @a offset.
	 * 
	 * @param offset Global offset of the frame to start searching from in 
	 * the proper bitmap.
	 * @param type Determines the bitmap to use.
	 *
	 * @return Global offset of the first empty frame in the proper bitmap.
	 */
	inline uint findFirstEmpty( 
		const uint offset, const AddressType type ) const;

	/*! 
	 * @brief Returns the beginning of the bitmap part representing the given
	 * frame type as an offset in the proper bitmap.
	 *
	 * @param frame Type of frames.
	 * @param type Determines the bitmap to use.
	 *
	 * @return Offset of the proper part of bitmap as a value.
	 */
	inline uint offset( 
		const PageSize frame, const AddressType type ) const;

	/*! 
	 * @brief Returns the beginning of the bitmap part representing the given
	 * frame type as an offset in the proper bitmap.
	 *
	 * @param frame Type of frames.
	 * @param type Determines the bitmap to use.
	 *
	 * @return Offset of the proper part of bitmap as a modifiable reference.
	 */
	inline uint& offset( const PageSize frame, const AddressType type );

	/*! 
	 * @brief Returns the end of the bitmap part representing the given
	 * frame type as an offset in the proper bitmap.
	 *
	 * @param frame Type of frames.
	 * @param type Determines the bitmap to use.
	 *
	 * @return Offset of the end of the proper part of bitmap as a value.
	 */
	inline uint offsetEnd( 
		const PageSize frame, const AddressType type ) const;

	/*! 
	 * @brief Returns the end of the bitmap part representing the given
	 * frame type as an offset in the proper bitmap.
	 *
	 * @param frame Type of frames.
	 * @param type Determines the bitmap to use.
	 *
	 * @return Offset of the end of the proper part of bitmap 
	 * as a modifiable reference.
	 */
	inline uint& offsetEnd( const PageSize frame, const AddressType type );

	/*! 
	 * @brief Returns the count of total free frames of given type as a value.
	 */
	inline uint freeFrames( 
		const PageSize frame, const AddressType type ) const;
	
	/*! 
	 * @brief Returns the count of total free frames of given type as a 
	 * modifiable reference.
	 */
	inline uint& freeFrames( const PageSize frame, const AddressType type );

	/*!
	 * @brief Returns the absolute offset of the first child 
	 * of frame with size @a frame and relative offset @a local_offset.
	 *
	 * @note First child of a frame is the first smaller frame contained in it.
	 */
	inline uint offsetOfChild( const uint local_offset, 
		const PageSize frame, const AddressType type ) const;

	/*!
	 * @brief Returns the absolute offset of the parent of frame with size 
	 * @a frame and relative offset @a local_offset.
	 *
	 * @note Parent of a frame is the larger frame which contains it.
	 */
	inline uint offsetOfParent( const uint local_offset, 
		const PageSize frame, const AddressType type ) const;

	/*!
	 * @return Number of empty frames of size @a frame, starting from 
	 * offset @a global_offset in the whole bitmap.
	 */
	inline uint emptyFrames( const uint global_offset, const uint count, 
		const PageSize frame, const AddressType type ) const;

	/*!
	 * @return Number of used frames of size @a frame, starting from 
	 * offset @a global_offset in the whole bitmap.
	 */
	inline uint fullFrames( const uint global_offset, const uint count, 
		const PageSize frame, const AddressType type ) const;

	/*!
	 * @return Relative offset of parent (larger frame containing this frame).
	 */
	inline uint parent( const uint local_offset ) const;

	/*!
	 * @return Relative offset of the first child (first smaller frame 
	 * contained in this frame).
	 */
	inline uint firstChild( const uint local_offset ) const;

	/*!
	 * @return Size in bytes of a larger frame 
	 * (even if such size is not supported).
	 */
	inline uint largerFrameSize( const PageSize frame ) const;

	/*!
	 * @return Size in bytes of a smaller frame 
	 * (even if such size is not supported).
	 */
	inline uint smallerFrameSize( const PageSize frame ) const;

	/*!
	 * @return Size of the frame of type @a frame in bytes.
	 */
	inline uint frameSize( const PageSize frame ) const;

	/*! 
	 * @brief Returns the next larger frame type.
	 */
	inline PageSize largerFrame( const PageSize frame ) const;

	/*! 
	 * @brief Returns the next smaller frame type.
	 */
	inline PageSize smallerFrame( const PageSize frame ) const;

	/*! 
	 * @brief Calculates the physical address of a frame of type @a frame,
	 * with absolute offset @a off in bitmap given by @a type.
	 */
	inline uintptr_t getAddress( const uint off, const PageSize frame,
		const AddressType type ) const;

	/*! 
	 * @brief Returns the offset behind the last frame in the bitmap given by
	 * @a type.
	 */
	inline uint theEnd( const AddressType type ) const;

	/*! 
	 * @brief Checks if the flags require allocation at not given position in
	 * the KSEG segments.
	 *
	 * I.e. checks if there are VF_VA_AUTO and VF_AT_KSEG0 or VF_AT_KSEG1 
	 * flags set.
	 */
	inline bool requestAtKseg( const uint flags ) const;

	/*! 
	 * @brief Checks if the flags require allocation at not given position 
	 * outside the KSEG segments.
	 *
	 * I.e. checks if there are VF_VA_AUTO and VF_AT_KUSEG, VF_AT_KSSEG or
	 * VF_AT_KSEG3 flags set.
	 */
	inline bool requestOutOfKseg( const uint flags ) const;

	/*! 
	 * @brief Checks if the flags require allocation at a given position.
	 *
	 * I.e. checks if there are VF_VA_AUTO and VF_AT_KUSEG, VF_AT_KSSEG or
	 * VF_AT_KSEG3 flags set.
	 */
	inline bool requestUser( const uint flags ) const;

	/*!
	 * @brief Prints out the state of the internal structures of the 
	 * FrameAllocator for both address ranges (KSEG0/1 and other).
	 */
	inline void checkStructures() const;

	/*! 
	 * @brief Prints out the state of the internal structures of the 
	 * FrameAllocator for the given address range.
	 */
	void printFree( const AddressType type ) const;

	/*! 
	 * @brief Checks if the internal structures of the FrameAllocator 
	 * for both address ranges (KSEG0/1 and other) are consistent.
	 *
	 * @sa checkBitmap().
	 */
	bool checkBitmaps() const;

	/*! 
	 * @brief Checks if the internal structures of the FrameAllocator 
	 * for the given address range are consistent.
	 *
	 * The bitmap is consistent if for every frame it is true that: \n
	 * 1) if the frame is used, then at least one of its children is used 
	 * as well\n
	 * 2) if the frame is free, all of its children are free too
	 */
	bool checkBitmap( const AddressType type ) const;

	/*------------------------------------------------------------------------*/
	/* MEMBERS                                                                */
	/*------------------------------------------------------------------------*/

	/*! 
	 * @brief Holds all important information about each level of the bitmap.
	 */
	class FramesInfo {
	public:
		/*! 
		 * @brief Offset of the begin of the bitmap of a particular frame size.
		 */
		uint beginOffset;

		/*! 
		 * @brief Offset of the end of the bitmap of a particular frame size.
		 */
		uint endOffset;

		/*! @brief Number of free frames of a particular frame size. */
		uint freeFrames;
	};

	/*! 
	 * @brief Holds information about each frame size's bitmap. 
	 *
	 * Indexed by Processor::PageSize.
	 */
	typedef FramesInfo FramesInfoTable[7];
	
	/*! 
	 * @brief Holds bitmap information for both segments. 
	 * 
	 * Indexed by AddressType.
	 */
	FramesInfoTable m_framesInfo[2];

	/*! 
	 * @brief Bitmaps for both address ranges (KSEG0/1 and other). 
	 *
	 * Holds info about free and full frames of each size.
	 */
	Bitset* m_bitmap[2];	

	/*! @brief Type of the largest frame which fits into the memory. */
	PageSize m_maxFrame;

	/*! 
	 * @brief Physical address of the end of last frame used by kernel and 
	 * FrameAllocator structures. 
	 */
	uintptr_t m_endOfBlockedFrames;

	/*! @brief Total size of the physical memory. */
	size_t m_memorySize;


	/*! 
	 * @brief Physical address of the last freed block of frames.
	 * 
	 * Indexed by AddressType.
	 */
	uintptr_t m_lastFreed[2];

	/*! 
	 * @brief Size of the last freed block. 
	 * 
	 * Indexed by AddressType.
	 */
	uint m_lastFreedSize[2];

	/*! 
	 * @brief Indicates whether the FrameAllocator was initialized successfuly.
	 */
	bool m_initialized;

friend class Singleton<FrameAllocator>;
};

/*----------------------------------------------------------------------------*/
/* DEFINITIONS ---------------------------------------------------------------*/
/*----------------------------------------------------------------------------*/

inline uint FrameAllocator::allocateAtKseg0( 
	void** address, const uint count, const PageSize frame )
{
	return allocateAtSegment(address, count, frame, KSEG);
}

/*----------------------------------------------------------------------------*/

inline uint FrameAllocator::findFirstEmpty( 
	const uint offset, const AddressType type ) const
{
	return offset + m_bitmap[type]->full(offset);
}

/*----------------------------------------------------------------------------*/

inline uint FrameAllocator::offset( 
	const PageSize frame, const AddressType type ) const
{ 
	return m_framesInfo[type][frame].beginOffset;
}

/*----------------------------------------------------------------------------*/

inline uint& FrameAllocator::offset( 
	const PageSize frame, const AddressType type )
{ 
	return m_framesInfo[type][frame].beginOffset;
}

/*----------------------------------------------------------------------------*/

inline uint FrameAllocator::offsetEnd( 
	const PageSize frame, const AddressType type ) const
{ 
	return m_framesInfo[type][frame].endOffset;
}

/*----------------------------------------------------------------------------*/

inline uint& FrameAllocator::offsetEnd( 
	const PageSize frame, const AddressType type )
{ 
	return m_framesInfo[type][frame].endOffset;
}

/*----------------------------------------------------------------------------*/

inline uint FrameAllocator::freeFrames( 
	const PageSize frame, const AddressType type ) const
{ 
	return m_framesInfo[type][frame].freeFrames;
}

/*----------------------------------------------------------------------------*/

inline uint& FrameAllocator::freeFrames( 
	const PageSize frame, const AddressType type )
{ 
	return m_framesInfo[type][frame].freeFrames;
}

/*---------------------------------------------------------------------------*/

inline uint FrameAllocator::offsetOfChild( const uint local_offset, 
	const PageSize frame, const AddressType type ) const
{
	ASSERT(frame > MIN_FRAME);
	return offset(smallerFrame(frame), type) + (local_offset * FRAME_STEP);
}

/*---------------------------------------------------------------------------*/

inline uint FrameAllocator::offsetOfParent( const uint local_offset, 
	const PageSize frame, const AddressType type ) const
{
	ASSERT(frame < MAX_FRAME);
	return offset(largerFrame(frame), type) + (local_offset / FRAME_STEP);
}

/*---------------------------------------------------------------------------*/

inline uint FrameAllocator::emptyFrames( const uint global_offset,
	const uint count, const PageSize frame, const AddressType type ) const
{
	return m_bitmap[type]->empty(global_offset, 
			min<uint>(count, (offsetEnd(frame, type) - global_offset)) );
}

/*---------------------------------------------------------------------------*/

inline uint FrameAllocator::fullFrames( const uint global_offset,
		const uint count, const PageSize frame, const AddressType type ) const
{
	return m_bitmap[type]->full(global_offset, 
		min<uint>(count, (offsetEnd(frame, type) - global_offset)) );
}

/*---------------------------------------------------------------------------*/

inline uint FrameAllocator::parent( const uint local_offset ) const
{
	return roundDown(local_offset, FRAME_STEP) / FRAME_STEP;
}

/*---------------------------------------------------------------------------*/

inline uint FrameAllocator::firstChild( const uint local_offset ) const
{
	return local_offset * FRAME_STEP;
}

/*---------------------------------------------------------------------------*/

inline uint FrameAllocator::largerFrameSize( const PageSize frame ) const
{
	return frameSize(frame) * FRAME_STEP;
}

/*---------------------------------------------------------------------------*/

inline uint FrameAllocator::smallerFrameSize( const PageSize frame ) const
{
	return frameSize(frame) / FRAME_STEP;
}

/*---------------------------------------------------------------------------*/

inline uint FrameAllocator::frameSize( const PageSize frame ) const
{
	return pages[frame].size;
}

/*---------------------------------------------------------------------------*/

inline PageSize FrameAllocator::largerFrame( const PageSize frame ) const
{
	ASSERT(frame < MAX_FRAME);
	return (PageSize)(frame + 1);
}

/*---------------------------------------------------------------------------*/

inline PageSize FrameAllocator::smallerFrame( const PageSize frame ) const
{
	ASSERT(frame > MIN_FRAME);
	return (PageSize)(frame - 1);
}

/*---------------------------------------------------------------------------*/

inline uintptr_t FrameAllocator::getAddress( const uint off, 
	const PageSize frame, const AddressType type ) const
{
	ASSERT(frame >= MIN_FRAME);
	return ( (off - offset(frame, type)) * frameSize(frame) );
}

/*---------------------------------------------------------------------------*/

inline uint FrameAllocator::theEnd( const AddressType type ) const
{
	return offsetEnd(MIN_FRAME, type);
}

/*--------------------------------------------------------------------------*/

inline bool FrameAllocator::requestAtKseg( const uint flags ) const
{
	return ( (flags & VF_VA_MASK) == (VF_VA_AUTO << VF_VA_SHIFT) 
	         && (    ((flags & VF_AT_MASK) == (VF_AT_KSEG0 << VF_AT_SHIFT)) 
	              || ((flags & VF_AT_MASK) == (VF_AT_KSEG1 << VF_AT_SHIFT)) )
	        );
}

/*--------------------------------------------------------------------------*/

inline bool FrameAllocator::requestOutOfKseg( const uint flags ) const
{
	return ( (flags & VF_VA_MASK) == (VF_VA_AUTO << VF_VA_SHIFT) 
	         && (    ((flags & VF_AT_MASK) == (VF_AT_KUSEG << VF_AT_SHIFT))
	              || ((flags & VF_AT_MASK) == (VF_AT_KSSEG << VF_AT_SHIFT))
	              || ((flags & VF_AT_MASK) == (VF_AT_KSEG3 << VF_AT_SHIFT)) )
	        );
}

/*--------------------------------------------------------------------------*/

inline bool FrameAllocator::requestUser( const uint flags ) const
{
	return ((flags & VF_VA_MASK) == (VF_VA_USER << VF_VA_SHIFT));
}

/*--------------------------------------------------------------------------*/

#undef PRINT_DEBUG
