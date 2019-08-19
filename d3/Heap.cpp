// Copyright( C ) 2004 Id Software, Inc.
//

#include "precompiled.h"
#pragma hdrstop

/*
================
Heap::Init
================
*/
void Heap::Init( ) {
	OSAllocs			= 0;
	pageRequests		= 0;
	pageSize			= 65536 - sizeof( Heap::page_s );
	pagesAllocated		= 0;								// reset page allocation counter

	largeFirstUsedPage	= NULL;								// init large heap manager
	swapPage			= NULL;

	Common::Com_Memset( smallFirstFree, 0, sizeof( smallFirstFree ) );	// init small heap manager
	smallFirstUsedPage	= NULL;
	smallCurPage		= AllocatePage( pageSize );
	assert( smallCurPage );
	smallCurPageOffset	= SMALL_ALIGN( 0 );

	defragBlock = NULL;

	mediumFirstFreePage	= NULL;								// init medium heap manager
	mediumLastFreePage	= NULL;
	mediumFirstUsedPage	= NULL;

	c_heapAllocRunningCount = 0;
}

/*
================
Heap::Heap
================
*/
Heap::Heap( ) {
	Init( );
}

/*
================
Heap::~Heap

  returns all allocated memory back to OS
================
*/
Heap::~Heap( ) {

	Heap::page_s	* p;

	if( smallCurPage ) {
		FreePage( smallCurPage );			// free small-heap current allocation page
	}
	p = smallFirstUsedPage;					// free small-heap allocated pages 
	while( p ) {
		Heap::page_s * next = p->next;
		FreePage( p );
		p = next;
	}

	p = largeFirstUsedPage;					// free large-heap allocated pages
	while( p ) {
		Heap::page_s * next = p->next;
		FreePage( p );
		p = next;
	}

	p = mediumFirstFreePage;				// free medium-heap allocated pages
	while( p ) {
		Heap::page_s * next = p->next;
		FreePage( p );
		p = next;
	}

	p = mediumFirstUsedPage;				// free medium-heap allocated completely used pages
	while( p ) {
		Heap::page_s * next = p->next;
		FreePage( p );
		p = next;
	}

	ReleaseSwappedPages( );			

	if( defragBlock ) {
		free( defragBlock );
	}

	assert( pagesAllocated == 0 );
}

/*
================
Heap::AllocDefragBlock
================
*/
void Heap::AllocDefragBlock( ) {
	int		size = 0x40000000;

	if( defragBlock ) {
		return;
	}
	while( 1 ) {
		defragBlock = malloc( size );
		if( defragBlock ) {
			break;
		}
		size >>= 1;
	}
	Common::Com_DPrintf( "Allocated a %i mb defrag block\n", size /( 1024* 1024 ) );
}

/*
================
Heap::Allocate
================
*/
void * Heap::Allocate( const size_t bytes ) {
	if( !bytes ) {
		return NULL;
	}
	c_heapAllocRunningCount++;

#if USE_LIBC_MALLOC
	return malloc( bytes );
#else
	if( !( bytes & ~255 ) ) {
		return SmallAllocate( bytes );
	}
	if( !( bytes & ~32767 ) ) {
		return MediumAllocate( bytes );
	}
	return LargeAllocate( bytes );
#endif
}

/*
================
Heap::Free
================
*/
void Heap::Free( void * p ) {
	if( !p ) {
		return;
	}
	c_heapAllocRunningCount--;

#if USE_LIBC_MALLOC
	free( p );
#else
	switch( ( ( byte * )( p ) )[ -1 ] ) {
		case SMALL_ALLOC: {
			SmallFree( p );
			break;
		}
		case MEDIUM_ALLOC: {
			MediumFree( p );
			break;
		}
		case LARGE_ALLOC: {
			LargeFree( p );
			break;
		}
		default: {
			Common::Com_Error( ERR_DISCONNECT, "Heap::Free: invalid memory block( %s )", NULL );
			break;
		}
	}
#endif
}

/*
================
Heap::Allocate16
================
*/
void * Heap::Allocate16( const size_t bytes ) {
	byte * ptr, * alignedPtr;

	ptr =( byte * ) malloc( bytes + 16 + 4 );
	if( !ptr ) {
		if( defragBlock ) {
			Common::Com_DPrintf( "Freeing defragBlock on alloc of %i.\n", bytes );
			free( defragBlock );
			defragBlock = NULL;
			ptr =( byte * ) malloc( bytes + 16 + 4 );			
			AllocDefragBlock( );
		}
		if( !ptr ) {
			Common::Com_Error( ERR_DISCONNECT, "malloc failure for %i", bytes );
		}
	}
	alignedPtr =( byte * )( ( ( int ) ptr ) + 15 & ~15 );
	if( alignedPtr - ptr < 4 ) {
		alignedPtr += 16;
	}
	* ( ( int * )( alignedPtr - 4 ) ) =( int ) ptr;
	return( void * ) alignedPtr;
}

/*
================
Heap::Free16
================
*/
void Heap::Free16( void * p ) {
	free( ( void * ) * ( ( int * )( ( ( byte * ) p ) - 4 ) ) );
}

/*
================
Heap::Msize

  returns size of allocated memory block
  p	= pointer to memory block
  Notes:	size may not be the same as the size in the original
			allocation request( due to block alignment reasons ).
================
*/
size_t Heap::Msize( void * p ) {

	if( !p ) {
		return 0;
	}

#if USE_LIBC_MALLOC
	#ifdef _WIN32
		return _msize( p );
	#else
		return 0;
	#endif
#else
	switch( ( ( byte * )( p ) )[ -1 ] ) {
		case SMALL_ALLOC: {
			//return SMALL_ALIGN( ( ( byte * )( p ) )[ -SMALL_HEADER_SIZE ] * ALIGN );
			return SMALL_ALIGN( *( ( ( byte * )( p ) ) -SMALL_HEADER_SIZE ) * ALIGN );
		}
		case MEDIUM_ALLOC: {
			return( ( mediumHeapEntry_s * )( ( ( byte * )( p ) ) - ALIGN_SIZE( MEDIUM_HEADER_SIZE ) ) )->size - ALIGN_SIZE( MEDIUM_HEADER_SIZE );
		}
		case LARGE_ALLOC: {
			return( ( Heap::page_s* )( *( ( dword * )( ( ( byte * )p ) - ALIGN_SIZE( LARGE_HEADER_SIZE ) ) ) ) )->dataSize - ALIGN_SIZE( LARGE_HEADER_SIZE );
		}
		default: {
			Common::Com_Error( ERR_DISCONNECT, "Heap::Msize: invalid memory block( %s )", NULL );
			return 0;
		}
	}
#endif
}

/*
================
Heap::Dump

  dump contents of the heap
================
*/
void Heap::Dump( ) {
	Heap::page_s	* pg;

	for( pg = smallFirstUsedPage; pg; pg = pg->next ) {
		Common::Com_DPrintf( "%p  bytes %-8d ( in use by small heap )\n", pg->data, pg->dataSize );
	}

	if( smallCurPage ) {
		pg = smallCurPage;
		Common::Com_DPrintf( "%p  bytes %-8d ( small heap active page )\n", pg->data, pg->dataSize );
	}

	for( pg = mediumFirstUsedPage; pg; pg = pg->next ) {
		Common::Com_DPrintf( "%p  bytes %-8d ( completely used by medium heap )\n", pg->data, pg->dataSize );
	}

	for( pg = mediumFirstFreePage; pg; pg = pg->next ) {
		Common::Com_DPrintf( "%p  bytes %-8d ( partially used by medium heap )\n", pg->data, pg->dataSize );
	}
	
	for( pg = largeFirstUsedPage; pg; pg = pg->next ) {
		Common::Com_DPrintf( "%p  bytes %-8d ( fully used by large heap )\n", pg->data, pg->dataSize );
	}

	Common::Com_DPrintf( "pages allocated : %d\n", pagesAllocated );
}

/*
================
Heap::FreePageReal

  frees page to be used by the OS
  p	= page to free
================
*/
void Heap::FreePageReal( Heap::page_s * p ) {
	assert( p );
	::free( p );
}

/*
================
Heap::ReleaseSwappedPages

  releases the swap page to OS
================
*/
void Heap::ReleaseSwappedPages( ) {
	if( swapPage ) {
		FreePageReal( swapPage );
	}
	swapPage = NULL;
}

/*
================
Heap::AllocatePage

  allocates memory from the OS
  bytes	= page size in bytes
  returns pointer to page
================
*/
Heap::page_s* Heap::AllocatePage( size_t bytes ) {
	Heap::page_s* 	p;

	pageRequests++;

	if( swapPage && swapPage->dataSize == bytes ) {			// if we've got a swap page somewhere
		p			= swapPage;
		swapPage	= NULL;
	}
	else {
		size_t size;

		size = bytes + sizeof( Heap::page_s );

		p =( Heap::page_s * ) ::malloc( size + ALIGN - 1 );
		if( !p ) {
			if( defragBlock ) {
				Common::Com_DPrintf( "Freeing defragBlock on alloc of %i.\n", size + ALIGN - 1 );
				free( defragBlock );
				defragBlock = NULL;
				p =( Heap::page_s * ) ::malloc( size + ALIGN - 1 );			
				AllocDefragBlock( );
			}
			if( !p ) {
				Common::Com_Error( ERR_DISCONNECT, "malloc failure for %i", bytes );
			}
		}

		p->data		=( void * ) ALIGN_SIZE( ( int )( ( byte * )( p ) ) + sizeof( Heap::page_s ) );
		p->dataSize	= size - sizeof( Heap::page_s );
		p->firstFree = NULL;
		p->largestFree = 0;
		OSAllocs++;
	}

	p->prev = NULL;
	p->next = NULL;

	pagesAllocated++;
	
	return p;
}

/*
================
Heap::FreePage

  frees a page back to the operating system
  p	= pointer to page
================
*/
void Heap::FreePage( Heap::page_s * p ) {
	assert( p );

	if( p->dataSize == pageSize && !swapPage ) {			// add to swap list?
		swapPage = p;
	}
	else {
		FreePageReal( p );
	}

	pagesAllocated--;
}

//===============================================================
//
//	small heap code
//
//===============================================================

/*
================
Heap::SmallAllocate

  allocate memory( 1-255 bytes ) from the small heap manager
  bytes = number of bytes to allocate
  returns pointer to allocated memory
================
*/
void * Heap::SmallAllocate( size_t bytes ) {
	// we need the at least sizeof( dword ) bytes for the free list
	if( bytes < sizeof( dword ) ) {
		bytes = sizeof( dword );
	}

	// increase the number of bytes if necessary to make sure the next small allocation is aligned
	bytes = SMALL_ALIGN( bytes );

	byte * smallBlock =( byte * )( smallFirstFree[bytes / ALIGN] );
	if( smallBlock ) {
		size_t * link =( size_t * )( smallBlock + SMALL_HEADER_SIZE );
		smallBlock[ 1 ] = SMALL_ALLOC;					// allocation entifier
		smallFirstFree[bytes / ALIGN] =( void * )( *link );
		return( void * )( link );
	}

	size_t bytesLeft =( long )( pageSize ) - smallCurPageOffset;
	// if we need to allocate a new page
	if( bytes >= bytesLeft ) {

		smallCurPage->next	= smallFirstUsedPage;
		smallFirstUsedPage	= smallCurPage;
		smallCurPage		= AllocatePage( pageSize );
		if( !smallCurPage ) {
			return NULL;
		}
		// make sure the first allocation is aligned
		smallCurPageOffset	= SMALL_ALIGN( 0 );
	}

	smallBlock			=( ( byte * )smallCurPage->data ) + smallCurPageOffset;
	smallBlock[ 0 ]		=( byte )( bytes / ALIGN );		// write # of bytes/ALIGN
	smallBlock[ 1 ]		= SMALL_ALLOC;					// allocation entifier
	smallCurPageOffset  += bytes + SMALL_HEADER_SIZE;	// increase the offset on the current page
	return( smallBlock + SMALL_HEADER_SIZE );			// skip the first two bytes
}

/*
================
Heap::SmallFree

  frees a block of memory allocated by SmallAllocate( ) call
  data = pointer to block of memory
================
*/
void Heap::SmallFree( void * ptr ) {
	( ( byte * )( ptr ) )[ -1 ] = INVALID_ALLOC;

	byte * d =( ( byte * )ptr ) - SMALL_HEADER_SIZE;
	dword * dt =( dword * )ptr;
	// index into the table with free small memory blocks
	dword ix = * d;

	// check if the index is correct
	if( ix >( 256 / ALIGN ) ) {
		Common::Com_Error( ERR_DISCONNECT, "SmallFree: invalid memory block" );
	}

	* dt =( dword )smallFirstFree[ ix ];	// write next index
	smallFirstFree[ ix ] =( void * )d;		// link
}

//===============================================================
//
//	medium heap code
//
//	Medium-heap allocated pages not returned to OS until heap destructor
//	called( re-used instead on subsequent medium-size malloc requests ).
//
//===============================================================

/*
================
Heap::MediumAllocateFromPage

  performs allocation using the medium heap manager from a given page
  p				= page
  sizeNeeded	= # of bytes needed
  returns pointer to allocated memory
================
*/
void * Heap::MediumAllocateFromPage( Heap::page_s * p, size_t sizeNeeded ) {

	mediumHeapEntry_s	* best, * nw = NULL;
	byte				* ret;

	best =( mediumHeapEntry_s * )( p->firstFree );			// first block is largest

	assert( best );
	assert( best->size == p->largestFree );
	assert( best->size >= sizeNeeded );

	// if we can allocate another block from this page after allocating sizeNeeded bytes
	if( best->size >=( dword )( sizeNeeded + MEDIUM_SMALLEST_SIZE ) ) {
		nw =( mediumHeapEntry_s * )( ( byte * )best + best->size - sizeNeeded );
		nw->page		= p;
		nw->prev		= best;
		nw->next		= best->next;
		nw->prevFree	= NULL;
		nw->nextFree	= NULL;
		nw->size		= sizeNeeded;
		nw->freeBlock	= 0;			// used block
		if( best->next ) {
			best->next->prev = nw;
		}
		best->next	= nw;
		best->size	-= sizeNeeded;
		
		p->largestFree = best->size;
	}
	else {
		if( best->prevFree ) {
			best->prevFree->nextFree = best->nextFree;
		}
		else {
			p->firstFree =( void * )best->nextFree;
		}
		if( best->nextFree ) {
			best->nextFree->prevFree = best->prevFree;
		}

		best->prevFree  = NULL;
		best->nextFree  = NULL;
		best->freeBlock = 0;			// used block
		nw = best;

		p->largestFree = 0;
	}

	ret		=( byte * )( nw ) + ALIGN_SIZE( MEDIUM_HEADER_SIZE );
	ret[ -1 ] = MEDIUM_ALLOC;		// allocation entifier

	return( void * )( ret );
}

/*
================
Heap::MediumAllocate

  allocate memory( 256-32768 bytes ) from medium heap manager
  bytes	= number of bytes to allocate
  returns pointer to allocated memory
================
*/
void * Heap::MediumAllocate( size_t bytes ) {
	Heap::page_s		* p;
	void				* data;

	size_t sizeNeeded = ALIGN_SIZE( bytes ) + ALIGN_SIZE( MEDIUM_HEADER_SIZE );

	// find first page with enough space
	for( p = mediumFirstFreePage; p; p = p->next ) {
		if( p->largestFree >= sizeNeeded ) {
			break;
		}
	}

	if( !p ) {								// need to allocate new page?
		p = AllocatePage( pageSize );
		if( !p ) {
			return NULL;					// malloc failure!
		}
		p->prev		= NULL;
		p->next		= mediumFirstFreePage;
		if( p->next ) {
			p->next->prev = p;
		}
		else {
			mediumLastFreePage	= p;
		}

		mediumFirstFreePage		= p;
		
		p->largestFree	= pageSize;
		p->firstFree	=( void * )p->data;

		mediumHeapEntry_s * e;
		e				=( mediumHeapEntry_s * )( p->firstFree );
		e->page			= p;
		// make sure( ( byte * )e + e->size ) is aligned
		e->size			= pageSize & ~( ALIGN - 1 );
		e->prev			= NULL;
		e->next			= NULL;
		e->prevFree		= NULL;
		e->nextFree		= NULL;
		e->freeBlock	= 1;
	}

	data = MediumAllocateFromPage( p, sizeNeeded );		// allocate data from page

    // if the page can no longer serve memory, move it away from free list
	//( so that it won't slow down the later alloc queries )
	// this modification speeds up the pageWalk from O( N ) to O( sqrt( N ) )
	// a call to free may swap this page back to the free list

	if( p->largestFree < MEDIUM_SMALLEST_SIZE ) {
		if( p == mediumLastFreePage ) {
			mediumLastFreePage = p->prev;
		}

		if( p == mediumFirstFreePage ) {
			mediumFirstFreePage = p->next;
		}

		if( p->prev ) {
			p->prev->next = p->next;
		}
		if( p->next ) {
			p->next->prev = p->prev;
		}

		// link to "completely used" list
		p->prev = NULL;
		p->next = mediumFirstUsedPage;
		if( p->next ) {
			p->next->prev = p;
		}
		mediumFirstUsedPage = p;
		return data;
	} 

	// re-order linked list( so that next malloc query starts from current
	// matching block ) -- this speeds up both the page walks and block walks

	if( p != mediumFirstFreePage ) {
		assert( mediumLastFreePage );
		assert( mediumFirstFreePage );
		assert( p->prev );

		mediumLastFreePage->next	= mediumFirstFreePage;
		mediumFirstFreePage->prev	= mediumLastFreePage;
		mediumLastFreePage			= p->prev;
		p->prev->next				= NULL;
		p->prev						= NULL;
		mediumFirstFreePage			= p;
	}

	return data;
}

/*
================
Heap::MediumFree

  frees a block allocated by the medium heap manager
  ptr	= pointer to data block
================
*/
void Heap::MediumFree( void * ptr ) {
	( ( byte * )( ptr ) )[ -1 ] = INVALID_ALLOC;

	mediumHeapEntry_s	* e =( mediumHeapEntry_s * )( ( byte * )ptr - ALIGN_SIZE( MEDIUM_HEADER_SIZE ) );
	Heap::page_s		* p = e->page;
	bool				isInFreeList;

	isInFreeList = p->largestFree >= MEDIUM_SMALLEST_SIZE;

	assert( e->size );
	assert( e->freeBlock == 0 );

	mediumHeapEntry_s * prev = e->prev;

	// if the previous block is free we can merge
	if( prev && prev->freeBlock ) {
		prev->size += e->size;
		prev->next = e->next;
		if( e->next ) {
			e->next->prev = prev;
		}
		e = prev;
	}
	else {
		e->prevFree		= NULL;				// link to beginning of free list
		e->nextFree		=( mediumHeapEntry_s * )p->firstFree;
		if( e->nextFree ) {
			assert( !( e->nextFree->prevFree ) );
			e->nextFree->prevFree = e;
		}

		p->firstFree	= e;
		p->largestFree	= e->size;
		e->freeBlock	= 1;				// mark block as free
	}
			
	mediumHeapEntry_s * next = e->next;

	// if the next block is free we can merge
	if( next && next->freeBlock ) {
		e->size += next->size;
		e->next = next->next;
		
		if( next->next ) {
			next->next->prev = e;
		}
		
		if( next->prevFree ) {
			next->prevFree->nextFree = next->nextFree;
		}
		else {
			assert( next == p->firstFree );
			p->firstFree = next->nextFree;
		}

		if( next->nextFree ) {
			next->nextFree->prevFree = next->prevFree;
		}
	}

	if( p->firstFree ) {
		p->largestFree =( ( mediumHeapEntry_s * )( p->firstFree ) )->size;
	}
	else {
		p->largestFree = 0;
	}

	// did e become the largest block of the page ?

	if( e->size > p->largestFree ) {
		assert( e != p->firstFree );
		p->largestFree = e->size;

		if( e->prevFree ) {
			e->prevFree->nextFree = e->nextFree;
		}
		if( e->nextFree ) {
			e->nextFree->prevFree = e->prevFree;
		}
		
		e->nextFree =( mediumHeapEntry_s * )p->firstFree;
		e->prevFree = NULL;
		if( e->nextFree ) {
			e->nextFree->prevFree = e;
		}
		p->firstFree = e;
	}

	// if page wasn't in free list( because it was near-full ), move it back there
	if( !isInFreeList ) {

		// remove from "completely used" list
		if( p->prev ) {
			p->prev->next = p->next;
		}
		if( p->next ) {
			p->next->prev = p->prev;
		}
		if( p == mediumFirstUsedPage ) {
			mediumFirstUsedPage = p->next;
		}

		p->next = NULL;
		p->prev = mediumLastFreePage;

		if( mediumLastFreePage ) {
			mediumLastFreePage->next = p;
		}
		mediumLastFreePage = p;
		if( !mediumFirstFreePage ) {
			mediumFirstFreePage = p;
		}
	} 
}

//===============================================================
//
//	large heap code
//
//===============================================================

/*
================
Heap::LargeAllocate

  allocates a block of memory from the operating system
  bytes	= number of bytes to allocate
  returns pointer to allocated memory
================
*/
void * Heap::LargeAllocate( size_t bytes ) {
	Heap::page_s * p = AllocatePage( bytes + ALIGN_SIZE( LARGE_HEADER_SIZE ) );

	assert( p );

	if( !p ) {
		return NULL;
	}

	byte * 	d	=( byte* )( p->data ) + ALIGN_SIZE( LARGE_HEADER_SIZE );
	dword * 	dw	=( dword* )( d - ALIGN_SIZE( LARGE_HEADER_SIZE ) );
	dw[ 0 ]		=( dword )p;				// write pointer back to page table
	d[ -1 ]		= LARGE_ALLOC;			// allocation entifier

	// link to 'large used page list'
	p->prev = NULL;
	p->next = largeFirstUsedPage;
	if( p->next ) {
		p->next->prev = p;
	}
	largeFirstUsedPage = p;

	return( void * )( d );
}

/*
================
Heap::LargeFree

  frees a block of memory allocated by the 'large memory allocator'
  p	= pointer to allocated memory
================
*/
void Heap::LargeFree( void * ptr ) {
	Heap::page_s* 	pg;

	( ( byte * )( ptr ) )[ -1 ] = INVALID_ALLOC;

	// get page pointer
	pg =( Heap::page_s * )( *( ( dword * )( ( ( byte * )ptr ) - ALIGN_SIZE( LARGE_HEADER_SIZE ) ) ) );

	// unlink from doubly linked list
	if( pg->prev ) {
		pg->prev->next = pg->next;
	}
	if( pg->next ) {
		pg->next->prev = pg->prev;
	}
	if( pg == largeFirstUsedPage ) {
		largeFirstUsedPage = pg->next;
	}
	pg->next = pg->prev = NULL;

	FreePage( pg );
}

//===============================================================
//
//	memory allocation all in one place
//
//===============================================================

#undef new

static Heap * 			mem_heap = NULL;
static memoryStats_t	mem_total_allocs = { 0, 0x0FFFFFFF, -1, 0 };
static memoryStats_t	mem_frame_allocs;
static memoryStats_t	mem_frame_frees;

/*
==================
Mem_ClearFrameStats
==================
*/
void Mem_ClearFrameStats( ) {
	mem_frame_allocs.num = mem_frame_frees.num = 0;
	mem_frame_allocs.minSize = mem_frame_frees.minSize = 0x0FFFFFFF;
	mem_frame_allocs.maxSize = mem_frame_frees.maxSize = -1;
	mem_frame_allocs.totalSize = mem_frame_frees.totalSize = 0;
}

/*
==================
Mem_GetFrameStats
==================
*/
void Mem_GetFrameStats( memoryStats_t &allocs, memoryStats_t &frees ) {
	allocs = mem_frame_allocs;
	frees = mem_frame_frees;
}

/*
==================
Mem_GetStats
==================
*/
void Mem_GetStats( memoryStats_t &stats ) {
	stats = mem_total_allocs;
}

/*
==================
Mem_UpdateStats
==================
*/
void Mem_UpdateStats( memoryStats_t & stats, size_t size ) {
	stats.num++;
	if( size < stats.minSize ) {
		stats.minSize = size;
	}
	if( size > stats.maxSize ) {
		stats.maxSize = size;
	}
	stats.totalSize += size;
}

/*
==================
Mem_UpdateAllocStats
==================
*/
void Mem_UpdateAllocStats( size_t size ) {
	Mem_UpdateStats( mem_frame_allocs, size );
	Mem_UpdateStats( mem_total_allocs, size );
}

/*
==================
Mem_UpdateFreeStats
==================
*/
void Mem_UpdateFreeStats( size_t size ) {
	Mem_UpdateStats( mem_frame_frees, size );
	mem_total_allocs.num--;
	mem_total_allocs.totalSize -= size;
}


#ifndef ID_DEBUG_MEMORY

/*
==================
Mem_Alloc
==================
*/
void * Mem_Alloc( const size_t size ) {
	if( !size ) {
		return NULL;
	}
	if( !mem_heap ) {
#ifdef CRASH_ON_STATIC_ALLOCATION
		* ( ( int* )0x0 ) = 1;
#endif
		return malloc( size );
	}
	void * mem = mem_heap->Allocate( size );
	Mem_UpdateAllocStats( mem_heap->Msize( mem ) );
	return mem;
}

/*
==================
Mem_Free
==================
*/
void Mem_Free( void * ptr ) {
	if( !ptr ) {
		return;
	}
	if( !mem_heap ) {
#ifdef CRASH_ON_STATIC_ALLOCATION
		* ( ( int* )0x0 ) = 1;
#endif
		free( ptr );
		return;
	}
	Mem_UpdateFreeStats( mem_heap->Msize( ptr ) );
 	mem_heap->Free( ptr );
}

/*
==================
Mem_Alloc16
==================
*/
void * Mem_Alloc16( const size_t size ) {
	if( !size ) {
		return NULL;
	}
	if( !mem_heap ) {
#ifdef CRASH_ON_STATIC_ALLOCATION
		* ( ( int* )0x0 ) = 1;
#endif
		return malloc( size );
	}
	void * mem = mem_heap->Allocate16( size );
	// make sure the memory is 16 byte aligned
	assert( ( ( ( int )mem ) & 15 ) == 0 );
	return mem;
}

/*
==================
Mem_Free16
==================
*/
void Mem_Free16( void * ptr ) {
	if( !ptr ) {
		return;
	}
	if( !mem_heap ) {
#ifdef CRASH_ON_STATIC_ALLOCATION
		* ( ( int* )0x0 ) = 1;
#endif
		free( ptr );
		return;
	}
	// make sure the memory is 16 byte aligned
	assert( ( ( ( int )ptr ) & 15 ) == 0 );
 	mem_heap->Free16( ptr );
}

/*
==================
Mem_ClearedAlloc
==================
*/
void * Mem_ClearedAlloc( const size_t size ) {
	void * mem = Mem_Alloc( size );
	Common::Com_Memset( mem, 0, size );
	return mem;
}

/*
==================
Mem_ClearedAlloc
==================
*/
void Mem_AllocDefragBlock( ) {
	mem_heap->AllocDefragBlock( );
}

/*
==================
Mem_CopyString
==================
*/
char * Mem_CopyString( const char * in ) {
	char	* out;
	
	out =( char * )Mem_Alloc( strlen( in ) + 1 );
	strcpy( out, in );
	return out;
}

/*
==================
Mem_Dump_f
==================
*/
void Mem_Dump_f( const CmdArgs &args ) {
}

/*
==================
Mem_DumpCompressed_f
==================
*/
void Mem_DumpCompressed_f( const CmdArgs &args ) {
}

/*
==================
Mem_Init
==================
*/
void Mem_Init( ) {
	mem_heap = new Heap;
	Mem_ClearFrameStats( );
}

/*
==================
Mem_Shutdown
==================
*/
void Mem_Shutdown( ) {
	Heap * m = mem_heap;
	mem_heap = NULL;
	delete m;
}

/*
==================
Mem_EnableLeakTest
==================
*/
void Mem_EnableLeakTest( const char * name ) {
}


#else /* !ID_DEBUG_MEMORY */

#undef		Mem_Alloc
#undef		Mem_ClearedAlloc
#undef		Com_ClearedReAlloc
#undef		Mem_Free
#undef		Mem_CopyString
#undef		Mem_Alloc16
#undef		Mem_Free16

#define MAX_CALLSTACK_DEPTH		6

// size of this struct must be a multiple of 16 bytes
typedef struct debugMemory_s {
	const char * 			fileName;
	int						lineNumber;
	int						frameNumber;
	int						size;
	address_t				callStack[ MAX_CALLSTACK_DEPTH ];
	struct debugMemory_s * 	prev;
	struct debugMemory_s * 	next;
} debugMemory_t;

static debugMemory_t * 	mem_debugMemory = NULL;
static char				mem_leakName[ 256 ] = "";

/*
==================
Mem_CleanupFileName
==================
*/
const char * Mem_CleanupFileName( const char * fileName ) {
	int i1, i2;
	Str newFileName;
	static char			newFileNames[ 4 ][ MAX_STRING_CHARS ];
	static int			index;

	newFileName = fileName;
	newFileName.BackSlashesToSlashes( );
	i1 = newFileName.Find( "neo", false );
	if( i1 >= 0 ) {
		i1 = newFileName.Find( "/", false, i1 );
		newFileName = newFileName.Right( newFileName.Length( ) -( i1 + 1 ) );
	}
	while( 1 ) {
		i1 = newFileName.Find( "/../" );
		if( i1 <= 0 ) {
			break;
		}
		i2 = i1 - 1;
		while( i2 > 1 && newFileName[ i2-1 ] != '/' ) {
			i2--;
		}
		newFileName = newFileName.Left( i2 - 1 ) + newFileName.Right( newFileName.Length( ) -( i1 + 4 ) );
	}
	index =( index + 1 ) & 3;
	strncpy( newFileNames[ index ], newFileName.c_str( ), sizeof( newFileNames[ index ] ) );
	return newFileNames[ index ];
}

/*
==================
Mem_Dump
==================
*/
void Mem_Dump( const char * fileName ) {
	int i, numBlocks, totalSize;
	char dump[ 32 ], * ptr;
	debugMemory_t * b;
	Str module, funcName;
	FILE * f;

	f = fopen( fileName, "wb" );
	if( !f ) {
		return;
	}

	totalSize = 0;
	for( numBlocks = 0, b = mem_debugMemory; b; b = b->next, numBlocks++ ) {
		ptr =( ( char * ) b ) + sizeof( debugMemory_t );
		totalSize += b->size;
		for( i = 0; i <( sizeof( dump )-1 ) && i < b->size; i++ ) {
			if( ptr[ i ] >= 32 && ptr[ i ] < 127 ) {
				dump[ i ] = ptr[ i ];
			} else {
				dump[ i ] = '_';
			}
		}
		dump[ i ] = '\0';
		if( ( b->size >> 10 ) != 0 ) {
			fprintf( f, "size: %6d KB: %s, line: %d [ %s ], call stack: %s\r\n", ( b->size >> 10 ), Mem_CleanupFileName( b->fileName ), b->lineNumber, dump, Lib::sys->GetCallStackStr( b->callStack, MAX_CALLSTACK_DEPTH ) );
		}
		else {
			fprintf( f, "size: %7d B: %s, line: %d [ %s ], call stack: %s\r\n", b->size, Mem_CleanupFileName( b->fileName ), b->lineNumber, dump, Lib::sys->GetCallStackStr( b->callStack, MAX_CALLSTACK_DEPTH ) );
		}
	}

	Lib::sys->ShutdownSymbols( );

	fprintf( f, "%8d total memory blocks allocated\r\n", numBlocks );
	fprintf( f, "%8d KB memory allocated\r\n", ( totalSize >> 10 ) );

	fclose( f );
}

/*
==================
Mem_Dump_f
==================
*/
void Mem_Dump_f( const CmdArgs &args ) {
	const char * fileName;

	if( args.Argc( ) >= 2 ) {
		fileName = args.Argv( 1 );
	}
	else {
		fileName = "memorydump.txt";
	}
	Mem_Dump( fileName );
}

/*
==================
Mem_DumpCompressed
==================
*/
typedef struct allocInfo_s {
	const char * 			fileName;
	int						lineNumber;
	int						size;
	int						numAllocs;
	address_t				callStack[ MAX_CALLSTACK_DEPTH ];
	struct allocInfo_s * 	next;
} allocInfo_t;

typedef enum {
	MEMSORT_SIZE, MEMSORT_LOCATION, MEMSORT_NUMALLOCS, MEMSORT_CALLSTACK
} memorySortType_t;

void Mem_DumpCompressed( const char * fileName, memorySortType_t memSort, int sortCallStack, int numFrames ) {
	int numBlocks, totalSize, r, j;
	debugMemory_t * b;
	allocInfo_t * a, * nexta, * allocInfo = NULL, * sortedAllocInfo = NULL, * prevSorted, * nextSorted;
	Str module, funcName;
	FILE * f;

	// build list with memory allocations
	totalSize = 0;
	numBlocks = 0;
	for( b = mem_debugMemory; b; b = b->next ) {

		if( numFrames && b->frameNumber < Lib::frameNumber - numFrames ) {
			continue;
		}

		numBlocks++;
		totalSize += b->size;

		// search for an allocation from the same source location
		for( a = allocInfo; a; a = a->next ) {
			if( a->lineNumber != b->lineNumber ) {
				continue;
			}
			for( j = 0; j < MAX_CALLSTACK_DEPTH; j++ ) {
				if( a->callStack[ j ] != b->callStack[ j ] ) {
					break;
				}
			}
			if( j < MAX_CALLSTACK_DEPTH ) {
				continue;
			}
			if( Str::Cmp( a->fileName, b->fileName ) != 0 ) {
				continue;
			}
			a->numAllocs++;
			a->size += b->size;
			break;
		}

		// if this is an allocation from a new source location
		if( !a ) {
			a =( allocInfo_t * ) ::malloc( sizeof( allocInfo_t ) );
			a->fileName = b->fileName;
			a->lineNumber = b->lineNumber;
			a->size = b->size;
			a->numAllocs = 1;
			for( j = 0; j < MAX_CALLSTACK_DEPTH; j++ ) {
				a->callStack[ j ] = b->callStack[ j ];
			}
			a->next = allocInfo;
			allocInfo = a;
		}
	}

	// sort list
	for( a = allocInfo; a; a = nexta ) {
		nexta = a->next;

		prevSorted = NULL;
		switch( memSort ) {
			// sort on size
			case MEMSORT_SIZE: {
				for( nextSorted = sortedAllocInfo; nextSorted; nextSorted = nextSorted->next ) {
					if( a->size > nextSorted->size ) {
						break;
					}
					prevSorted = nextSorted;
				}
				break;
			}
			// sort on file name and line number
			case MEMSORT_LOCATION: {
				for( nextSorted = sortedAllocInfo; nextSorted; nextSorted = nextSorted->next ) {
					r = Str::Cmp( Mem_CleanupFileName( a->fileName ), Mem_CleanupFileName( nextSorted->fileName ) );
					if( r < 0 ||( r == 0 && a->lineNumber < nextSorted->lineNumber ) ) {
						break;
					}
					prevSorted = nextSorted;
				}
				break;
			}
			// sort on the number of allocations
			case MEMSORT_NUMALLOCS: {
				for( nextSorted = sortedAllocInfo; nextSorted; nextSorted = nextSorted->next ) {
					if( a->numAllocs > nextSorted->numAllocs ) {
						break;
					}
					prevSorted = nextSorted;
				}
				break;
			}
			// sort on call stack
			case MEMSORT_CALLSTACK: {
				for( nextSorted = sortedAllocInfo; nextSorted; nextSorted = nextSorted->next ) {
					if( a->callStack[ sortCallStack ] < nextSorted->callStack[ sortCallStack ] ) {
						break;
					}
					prevSorted = nextSorted;
				}
				break;
			}
		}
		if( !prevSorted ) {
			a->next = sortedAllocInfo;
			sortedAllocInfo = a;
		}
		else {
			prevSorted->next = a;
			a->next = nextSorted;
		}
	}

	f = fopen( fileName, "wb" );
	if( !f ) {
		return;
	}

	// write list to file
	for( a = sortedAllocInfo; a; a = nexta ) {
		nexta = a->next;
		fprintf( f, "size: %6d KB, allocs: %5d: %s, line: %d, call stack: %s\r\n", ( a->size >> 10 ), a->numAllocs, Mem_CleanupFileName( a->fileName ), a->lineNumber, Lib::sys->GetCallStackStr( a->callStack, MAX_CALLSTACK_DEPTH ) );
		::free( a );
	}

	Lib::sys->ShutdownSymbols( );

	fprintf( f, "%8d total memory blocks allocated\r\n", numBlocks );
	fprintf( f, "%8d KB memory allocated\r\n", ( totalSize >> 10 ) );

	fclose( f );
}

/*
==================
Mem_DumpCompressed_f
==================
*/
void Mem_DumpCompressed_f( const CmdArgs &args ) {
	int argNum;
	const char * arg, * fileName;
	memorySortType_t memSort = MEMSORT_LOCATION;
	int sortCallStack = 0, numFrames = 0;

	// get cmd-line options
	argNum = 1;
	arg = args.Argv( argNum );
	while( arg[ 0 ] == '-' ) {
		arg = args.Argv( ++argNum );
		if( Str::Icmp( arg, "s" ) == 0 ) {
			memSort = MEMSORT_SIZE;
		} else if( Str::Icmp( arg, "l" ) == 0 ) {
			memSort = MEMSORT_LOCATION;
		} else if( Str::Icmp( arg, "a" ) == 0 ) {
			memSort = MEMSORT_NUMALLOCS;
		} else if( Str::Icmp( arg, "cs1" ) == 0 ) {
			memSort = MEMSORT_CALLSTACK;
			sortCallStack = 2;
		} else if( Str::Icmp( arg, "cs2" ) == 0 ) {
			memSort = MEMSORT_CALLSTACK;
			sortCallStack = 1;
		} else if( Str::Icmp( arg, "cs3" ) == 0 ) {
			memSort = MEMSORT_CALLSTACK;
			sortCallStack = 0;
		} else if( arg[ 0 ] == 'f' ) {
			numFrames = atoi( arg + 1 );
		} else {
			Lib::common->Printf( "memoryDumpCompressed [ options ] [ filename ]\n"
						"options:\n"
						"  -s     sort on size\n"
						"  -l     sort on location\n"
						"  -a     sort on the number of allocations\n"
						"  -cs1   sort on first function on call stack\n"
						"  -cs2   sort on second function on call stack\n"
						"  -cs3   sort on third function on call stack\n"
						"  -f<X>  only report allocations the last X frames\n"
						"By default the memory allocations are sorted on location.\n"
						"By default a 'memorydump.txt' is written if no file name is specified.\n" );
			return;
		}
		arg = args.Argv( ++argNum );
	}
	if( argNum >= args.Argc( ) ) {
		fileName = "memorydump.txt";
	} else {
		fileName = arg;
	}
	Mem_DumpCompressed( fileName, memSort, sortCallStack, numFrames );
}

/*
==================
Mem_AllocDebugMemory
==================
*/
void * Mem_AllocDebugMemory( const int size, const char * fileName, const int lineNumber, const bool align16 ) {
	void * p;
	debugMemory_t * m;

	if( !size ) {
		return NULL;
	}

	if( !mem_heap ) {
#ifdef CRASH_ON_STATIC_ALLOCATION
		* ( ( int* )0x0 ) = 1;
#endif
		// NOTE: set a breakpoint here to find memory allocations before mem_heap is initialized
		return malloc( size );
	}

	if( align16 ) {
		p = mem_heap->Allocate16( size + sizeof( debugMemory_t ) );
	}
	else {
		p = mem_heap->Allocate( size + sizeof( debugMemory_t ) );
	}

	Mem_UpdateAllocStats( size );

	m =( debugMemory_t * ) p;
	m->fileName = fileName;
	m->lineNumber = lineNumber;
	m->frameNumber = Lib::frameNumber;
	m->size = size;
	m->next = mem_debugMemory;
	m->prev = NULL;
	if( mem_debugMemory ) {
		mem_debugMemory->prev = m;
	}
	mem_debugMemory = m;
	Lib::sys->GetCallStack( m->callStack, MAX_CALLSTACK_DEPTH );

	return( ( ( byte * ) p ) + sizeof( debugMemory_t ) );
}

/*
==================
Mem_FreeDebugMemory
==================
*/
void Mem_FreeDebugMemory( void * p, const char * fileName, const int lineNumber, const bool align16 ) {
	debugMemory_t * m;

	if( !p ) {
		return;
	}

	if( !mem_heap ) {
#ifdef CRASH_ON_STATIC_ALLOCATION
		* ( ( int* )0x0 ) = 1;
#endif
		// NOTE: set a breakpoint here to find memory being freed before mem_heap is initialized
		free( p );
		return;
	}

	m =( debugMemory_t * )( ( ( byte * ) p ) - sizeof( debugMemory_t ) );

	if( m->size < 0 ) {
		Lib::common->FatalError( "memory freed twice, first from %s, now from %s", Lib::sys->GetCallStackStr( m->callStack, MAX_CALLSTACK_DEPTH ), Lib::sys->GetCallStackCurStr( MAX_CALLSTACK_DEPTH ) );
	}

	Mem_UpdateFreeStats( m->size );

	if( m->next ) {
		m->next->prev = m->prev;
	}
	if( m->prev ) {
		m->prev->next = m->next;
	}
	else {
		mem_debugMemory = m->next;
	}

	m->fileName = fileName;
	m->lineNumber = lineNumber;
	m->frameNumber = Lib::frameNumber;
	m->size = -m->size;
	Lib::sys->GetCallStack( m->callStack, MAX_CALLSTACK_DEPTH );

	if( align16 ) {
 		mem_heap->Free16( m );
	}
	else {
 		mem_heap->Free( m );
	}
}

/*
==================
Mem_Alloc
==================
*/
void * Mem_Alloc( const int size, const char * fileName, const int lineNumber ) {
	if( !size ) {
		return NULL;
	}
	return Mem_AllocDebugMemory( size, fileName, lineNumber, false );
}

/*
==================
Mem_Free
==================
*/
void Mem_Free( void * ptr, const char * fileName, const int lineNumber ) {
	if( !ptr ) {
		return;
	}
	Mem_FreeDebugMemory( ptr, fileName, lineNumber, false );
}

/*
==================
Mem_Alloc16
==================
*/
void * Mem_Alloc16( const int size, const char * fileName, const int lineNumber ) {
	if( !size ) {
		return NULL;
	}
	void * mem = Mem_AllocDebugMemory( size, fileName, lineNumber, true );
	// make sure the memory is 16 byte aligned
	assert( ( ( ( int )mem ) & 15 ) == 0 );
	return mem;
}

/*
==================
Mem_Free16
==================
*/
void Mem_Free16( void * ptr, const char * fileName, const int lineNumber ) {
	if( !ptr ) {
		return;
	}
	// make sure the memory is 16 byte aligned
	assert( ( ( ( int )ptr ) & 15 ) == 0 );
	Mem_FreeDebugMemory( ptr, fileName, lineNumber, true );
}

/*
==================
Mem_ClearedAlloc
==================
*/
void * Mem_ClearedAlloc( const int size, const char * fileName, const int lineNumber ) {
	void * mem = Mem_Alloc( size, fileName, lineNumber );
	SIMDProcessor->Common::Com_Memset( mem, 0, size );
	return mem;
}

/*
==================
Mem_CopyString
==================
*/
char * Mem_CopyString( const char * in, const char * fileName, const int lineNumber ) {
	char	* out;
	
	out =( char * )Mem_Alloc( strlen( in ) + 1, fileName, lineNumber );
	strcpy( out, in );
	return out;
}

/*
==================
Mem_Init
==================
*/
void Mem_Init( ) {
	mem_heap = new Heap;
}

/*
==================
Mem_Shutdown
==================
*/
void Mem_Shutdown( ) {

	if( mem_leakName[ 0 ] != '\0' ) {
		Mem_DumpCompressed( va( "%s_leak_size.txt", mem_leakName ), MEMSORT_SIZE, 0, 0 );
		Mem_DumpCompressed( va( "%s_leak_location.txt", mem_leakName ), MEMSORT_LOCATION, 0, 0 );
		Mem_DumpCompressed( va( "%s_leak_cs1.txt", mem_leakName ), MEMSORT_CALLSTACK, 2, 0 );
	}

	Heap * m = mem_heap;
	mem_heap = NULL;
	delete m;
}

/*
==================
Mem_EnableLeakTest
==================
*/
void Mem_EnableLeakTest( const char * name ) {
	Str::Copynz( mem_leakName, name, sizeof( mem_leakName ) );
}

#endif /* !ID_DEBUG_MEMORY */
