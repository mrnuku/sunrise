// Copyright (C) 2004 Id Software, Inc.
//

#ifndef __HEAP_H__
#define __HEAP_H__

#ifndef USE_LIBC_MALLOC
	#define USE_LIBC_MALLOC		0
#endif

#ifndef CRASH_ON_STATIC_ALLOCATION
//	#define CRASH_ON_STATIC_ALLOCATION
#endif

//===============================================================
//
//	Heap
//
//===============================================================

#define SMALL_HEADER_SIZE		( ( size_t )( sizeof( byte ) + sizeof( byte ) ) )
#define MEDIUM_HEADER_SIZE		( ( size_t )( sizeof( mediumHeapEntry_s ) + sizeof( byte ) ) )
#define LARGE_HEADER_SIZE		( ( size_t )( sizeof( dword * ) + sizeof( byte ) ) )

#define ALIGN_SIZE( bytes )		( ( ( bytes ) + ALIGN - 1 ) & ~( ALIGN - 1 ) )
#define SMALL_ALIGN( bytes )	( ALIGN_SIZE( ( bytes ) + SMALL_HEADER_SIZE ) - SMALL_HEADER_SIZE )
#define MEDIUM_SMALLEST_SIZE	( ALIGN_SIZE( 256 ) + ALIGN_SIZE( MEDIUM_HEADER_SIZE ) )


class Heap {

public:
					Heap( );
					~Heap( );				// frees all associated data
	void			Init( );					// initialize
	void *			Allocate( const size_t bytes );	// allocate memory
	void			Free( void * p );				// free memory
	void *			Allocate16( const size_t bytes );// allocate 16 byte aligned memory
	void			Free16( void * p );				// free 16 byte aligned memory
	size_t			Msize( void * p );				// return size of data block
	void			Dump( );

	void 			AllocDefragBlock( );		// hack for huge renderbumps

private:

	enum {
		ALIGN = 8									// memory alignment in bytes
	};

	enum {
		INVALID_ALLOC	= 0xdd,
		SMALL_ALLOC		= 0xaa,						// small allocation
		MEDIUM_ALLOC	= 0xbb,						// medium allocaction
		LARGE_ALLOC		= 0xcc						// large allocaction
	};

	struct page_s {									// allocation page
		void *				data;					// data pointer to allocated memory
		size_t				dataSize;				// number of bytes of memory 'data' points to
		page_s *			next;					// next free page in same page manager
		page_s *			prev;					// used only when allocated
		size_t				largestFree;			// this data used by the medium-size heap manager
		void *				firstFree;				// pointer to first free entry
	};

	struct mediumHeapEntry_s {
		page_s *			page;					// pointer to page
		size_t				size;					// size of block
		mediumHeapEntry_s *	prev;					// previous block
		mediumHeapEntry_s *	next;					// next block
		mediumHeapEntry_s *	prevFree;				// previous free block
		mediumHeapEntry_s *	nextFree;				// next free block
		size_t				freeBlock;				// non-zero if free block
	};

	// variables
	void *			smallFirstFree[256/ALIGN+1];	// small heap allocator lists (for allocs of 1-255 bytes)
	page_s *		smallCurPage;					// current page for small allocations
	size_t			smallCurPageOffset;				// byte offset in current page
	page_s *		smallFirstUsedPage;				// first used page of the small heap manager

	page_s *		mediumFirstFreePage;			// first partially free page
	page_s *		mediumLastFreePage;				// last partially free page
	page_s *		mediumFirstUsedPage;			// completely used page

	page_s *		largeFirstUsedPage;				// first page used by the large heap manager

	page_s *		swapPage;

	dword			pagesAllocated;					// number of pages currently allocated
	size_t			pageSize;						// size of one alloc page in bytes

	dword			pageRequests;					// page requests
	dword			OSAllocs;						// number of allocs made to the OS

	int				c_heapAllocRunningCount;

	void *			defragBlock;					// a single huge block that can be allocated
													// at startup, then freed when needed

	// methods
	page_s *		AllocatePage( size_t bytes );	// allocate page from the OS
	void			FreePage( Heap::page_s * p );	// free an OS allocated page

	void *			SmallAllocate( size_t bytes );	// allocate memory (1-255 bytes) from small heap manager
	void			SmallFree( void * ptr );			// free memory allocated by small heap manager

	void *			MediumAllocateFromPage( Heap::page_s * p, size_t sizeNeeded );
	void *			MediumAllocate( size_t bytes );	// allocate memory (256-32768 bytes) from medium heap manager
	void			MediumFree( void * ptr );		// free memory allocated by medium heap manager

	void *			LargeAllocate( size_t bytes );	// allocate large block from OS directly
	void			LargeFree( void * ptr );			// free memory allocated by large heap manager

	void			ReleaseSwappedPages( );
	void			FreePageReal( Heap::page_s * p );
};


/*
===============================================================================

	Memory Management

	This is a replacement for the compiler heap code (i.e. "C" malloc( ) and
	free( ) calls). On average 2.5-3.0 times faster than MSVC malloc( )/free( ).
	Worst case performance is 1.65 times faster and best case > 70 times.
 
===============================================================================
*/


typedef struct {
	int			num;
	size_t		minSize;
	size_t		maxSize;
	size_t		totalSize;
} memoryStats_t;


void		Mem_Init( );
void		Mem_Shutdown( );
void		Mem_EnableLeakTest( const char * name );
void		Mem_ClearFrameStats( );
void		Mem_GetFrameStats( memoryStats_t &allocs, memoryStats_t &frees );
void		Mem_GetStats( memoryStats_t &stats );
void		Mem_Dump_f( const class idCmdArgs &args );
void		Mem_DumpCompressed_f( const class idCmdArgs &args );
void		Mem_AllocDefragBlock( );


#ifndef ID_DEBUG_MEMORY

void *		Mem_Alloc( const size_t size );
void *		Mem_ClearedAlloc( const size_t size );
void		Mem_Free( void  * ptr );
char *		Mem_CopyString( const char * in );
void *		Mem_Alloc16( const size_t size );
void		Mem_Free16( void * ptr );

#ifdef ID_REDIRECT_NEWDELETE

__inline void * operator new( size_t s ) {
	return Mem_Alloc( s );
}
__inline void operator delete( void * p ) {
	Mem_Free( p );
}
__inline void * operator new[]( size_t s ) {
	return Mem_Alloc( s );
}
__inline void operator delete[]( void * p ) {
	Mem_Free( p );
}

#endif

#else /* ID_DEBUG_MEMORY */

void *		Mem_Alloc( const int size, const char * fileName, const int lineNumber );
void *		Mem_ClearedAlloc( const int size, const char * fileName, const int lineNumber );
void		Mem_Free( void * ptr, const char * fileName, const int lineNumber );
char *		Mem_CopyString( const char * in, const char * fileName, const int lineNumber );
void *		Mem_Alloc16( const int size, const char * fileName, const int lineNumber );
void		Mem_Free16( void * ptr, const char * fileName, const int lineNumber );

#ifdef ID_REDIRECT_NEWDELETE

__inline void * operator new( size_t s, int t1, int t2, char * fileName, int lineNumber ) {
	return Mem_Alloc( s, fileName, lineNumber );
}
__inline void operator delete( void * p, int t1, int t2, char * fileName, int lineNumber ) {
	Mem_Free( p, fileName, lineNumber );
}
__inline void * operator new[]( size_t s, int t1, int t2, char * fileName, int lineNumber ) {
	return Mem_Alloc( s, fileName, lineNumber );
}
__inline void operator delete[]( void * p, int t1, int t2, char * fileName, int lineNumber ) {
	Mem_Free( p, fileName, lineNumber );
}
__inline void * operator new( size_t s ) {
	return Mem_Alloc( s, "", 0 );
}
__inline void operator delete( void * p ) {
	Mem_Free( p, "", 0 );
}
__inline void * operator new[]( size_t s ) {
	return Mem_Alloc( s, "", 0 );
}
__inline void operator delete[]( void * p ) {
	Mem_Free( p, "", 0 );
}

#define ID_DEBUG_NEW						new( 0, 0, __FILE__, __LINE__ )
#undef new
#define new									ID_DEBUG_NEW

#endif

#define		Mem_Alloc( size )				Mem_Alloc( size, __FILE__, __LINE__ )
#define		Mem_ClearedAlloc( size )		Mem_ClearedAlloc( size, __FILE__, __LINE__ )
#define		Mem_Free( ptr )					Mem_Free( ptr, __FILE__, __LINE__ )
#define		Mem_CopyString( s )				Mem_CopyString( s, __FILE__, __LINE__ )
#define		Mem_Alloc16( size )				Mem_Alloc16( size, __FILE__, __LINE__ )
#define		Mem_Free16( ptr )				Mem_Free16( ptr, __FILE__, __LINE__ )

#endif /* ID_DEBUG_MEMORY */

#endif /* !__HEAP_H__ */
