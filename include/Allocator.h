#ifndef ALLOCATOR_H
#define ALLOCATOR_H

// // //// // //// // //// //
/// AllocatorBase
//
/// Base class for block allocators
// //// // //// // //// //

template< class type > class AllocatorBase {
protected:
	bool									d_lockMemory;				// lock memory so it cannot get swapped out

	int										d_numBaseBlocks;			// number of base blocks
	int										d_baseBlockMemory;			// total memory in base blocks
	int										d_numUsedBlocks;			// number of used blocks
	int										d_usedBlockMemory;			// total memory in used blocks
	int										d_numFreeBlocks;			// number of free blocks
	int										d_freeBlockMemory;			// total memory in free blocks

	int										d_numAllocs;
	int										d_numResizes;
	int										d_numFrees;

	Lock									d_lock;						// lock for thread safe memory allocation

	void									ClearBase( );

public:

	virtual void							Init( ) = 0;
	virtual void							Shutdown( ) = 0;
	virtual void							SetFixedBlocks( int numBlocks ) = 0;
	virtual void							SetLockMemory( bool lock ) = 0;
	virtual void							FreeEmptyBaseBlocks( ) = 0;

	virtual type *							Alloc( const int num ) = 0;
	virtual type *							Resize( type * ptr, const int num ) = 0;
	virtual void							Free( type * ptr ) = 0;
	virtual const char *					CheckMemory( const type * ptr ) const = 0;

	type *									Construct( const int num );
	type *									Construct( const int num, const type & def );

	int										GetNumBaseBlocks( ) const		{ return d_numBaseBlocks; }
	int										GetBaseBlockMemory( ) const		{ return d_baseBlockMemory; }
	int										GetNumUsedBlocks( ) const		{ return d_numUsedBlocks; }
	int										GetUsedBlockMemory( ) const		{ return d_usedBlockMemory; }
	int										GetNumFreeBlocks( ) const		{ return d_numFreeBlocks; }
	int										GetFreeBlockMemory( ) const		{ return d_freeBlockMemory; }
	int										GetAllocCount( ) const			{ return d_numUsedBlocks; }

	virtual int								GetNumEmptyBaseBlocks( ) const = 0;
};

#define BASECLASS AllocatorBase<type>

// // //// // //// // //// //
/// DynamicAlloc
//
/// Dynamic allocator, simple wrapper for normal allocations which can be interchanged with DynamicBlockAlloc.
//
/// No constructor is called for the 'type'.
/// Allocated blocks are always 16 byte aligned.
// //// // //// // //// //

template< class type, int baseBlockSize, int minBlockSize > class DynamicAlloc : public AllocatorBase< type > {
private:
	//List< type * >					d_areaList;
public:
									DynamicAlloc( );
									~DynamicAlloc( );

	virtual void					Init( );
	virtual void					Shutdown( );
	virtual void					SetFixedBlocks( int numBlocks ) {}
	virtual void					SetLockMemory( bool lock ) {}
	virtual void					FreeEmptyBaseBlocks( ) {}

	virtual type *					Alloc( const int num );
	virtual type *					Resize( type * ptr, const int num );
	virtual void					Free( type * ptr );
	virtual const char *			CheckMemory( const type * ptr ) const;

	virtual int						GetNumEmptyBaseBlocks( ) const { return 0; }
};

//#define DYNAMIC_BLOCK_ALLOC_CHECK

// // //// // //// // //// //
/// AllocatorBase
//
/// Fast dynamic block allocator.
//
/// No constructor is called for the 'type'.
/// Allocated blocks are always 16 byte aligned.
// //// // //// // //// //

template< class type > class DynamicBlock {
public:
#ifdef DYNAMIC_BLOCK_ALLOC_CHECK
	int										d_id[ 3 ];
	void *									d_allocator;
#endif
	int										d_size;					// size in bytes of the block
	DynamicBlock< type > *					d_prev;					// previous memory block
	DynamicBlock< type > *					d_next;					// next memory block
	BTreeNode< DynamicBlock< type >, int > *d_node;					// node in the B-Tree with free blocks

	type *									GetMemory( ) const					{ return ( type * )( ( ( byte * ) this ) + sizeof( DynamicBlock< type > ) ); }
	int										GetSize( ) const					{ return abs( d_size ); }
	void									SetSize( int s, bool isBaseBlock )	{ d_size = isBaseBlock ? -s : s; }
	bool									IsBaseBlock( ) const				{ return ( d_size < 0 ); }
};

template< class type, int baseBlockSize, int minBlockSize > class DynamicBlockAlloc : public AllocatorBase< type > {
private:

	DynamicBlock< type > *					d_firstBlock;				// first block in list in order of increasing address
	DynamicBlock< type > *					d_lastBlock;				// last block in list in order of increasing address
	BTree< DynamicBlock< type >, int, 4 >	d_freeTree;					// B-Tree with free memory blocks
	bool									d_allowAllocs;				// allow base block allocations

#ifdef DYNAMIC_BLOCK_ALLOC_CHECK
	int										d_blockId[ 3 ];
#endif

	void									Clear( );
	DynamicBlock< type > *					AllocInternal( const int num );
	DynamicBlock< type > *					ResizeInternal( DynamicBlock< type > * block, const int num );
	void									FreeInternal( DynamicBlock< type > * block );
	void									LinkFreeInternal( DynamicBlock< type > * block );
	void									UnlinkFreeInternal( DynamicBlock< type > * block );
	void									CheckMemory( ) const;

public:

											DynamicBlockAlloc( );
											~DynamicBlockAlloc( );

	virtual void							Init( );
	virtual void							Shutdown( );
	virtual void							SetFixedBlocks( int numBlocks );
	virtual void							SetLockMemory( bool lock );
	virtual void							FreeEmptyBaseBlocks( );

	virtual type *							Alloc( const int num );
	virtual type *							Resize( type * ptr, const int num );
	virtual void							Free( type * ptr );
	virtual const char *					CheckMemory( const type * ptr ) const;

	virtual int								GetNumEmptyBaseBlocks( ) const;
};

// // //// // //// // //// //
// AllocatorBase
// //// // //// // //// //

template< class type >
INLINE type * AllocatorBase< type >::Construct( const int num ) {
	type * block = Alloc( num );
	for( int i = 0; i < num; i++ )
		new ( block + i )( type );
	return block;
}

template< class type >
INLINE type * AllocatorBase< type >::Construct( const int num, const type & def ) {
	type * block = Alloc( num );
	for( int i = 0; i < num; i++ )
		new ( block + i )( def );
	return block;
}

template< class type >
INLINE void AllocatorBase< type >::ClearBase( ) {
	d_lockMemory = false;
	d_numBaseBlocks = 0;
	d_baseBlockMemory = 0;
	d_numUsedBlocks = 0;
	d_usedBlockMemory = 0;
	d_numFreeBlocks = 0;
	d_freeBlockMemory = 0;
	d_numAllocs = 0;
	d_numResizes = 0;
	d_numFrees = 0;
}

// // //// // //// // //// //
// DynamicAlloc
// //// // //// // //// //

template< class type, int baseBlockSize, int minBlockSize  >
INLINE DynamicAlloc< type, baseBlockSize, minBlockSize >::DynamicAlloc( ) {
	BASECLASS::ClearBase( );
}

template< class type, int baseBlockSize, int minBlockSize >
INLINE DynamicAlloc< type, baseBlockSize, minBlockSize >::~DynamicAlloc( ) {
	Shutdown( );
}

template< class type, int baseBlockSize, int minBlockSize >
void DynamicAlloc< type, baseBlockSize, minBlockSize >::Init( ) {
}

template< class type, int baseBlockSize, int minBlockSize >
void DynamicAlloc< type, baseBlockSize, minBlockSize >::Shutdown( ) {
	for( int i = 0; i < d_areaList.Num( ); i++ )
		Mem_Free16( d_areaList[ i ] );
	d_areaList.Clear( );
	BASECLASS::ClearBase( );
}

template< class type, int baseBlockSize, int minBlockSize >
type * DynamicAlloc< type, baseBlockSize, minBlockSize >::Alloc( const int num ) {
	ScopedLock< true > scopedLock( BASECLASS::d_lock );
	BASECLASS::d_numAllocs++;
	if( num <= 0 )
		return NULL;
	BASECLASS::d_numUsedBlocks++;
	BASECLASS::d_usedBlockMemory += num * sizeof( type );
	type * area = ( type * )Mem_Alloc16( NumSizeOf( type, num ) );
	d_areaList.Append( area );
	return area;
}

template< class type, int baseBlockSize, int minBlockSize >
type * DynamicAlloc< type, baseBlockSize, minBlockSize >::Resize( type * ptr, const int num ) {
	ScopedLock< true > scopedLock( BASECLASS::d_lock );
	BASECLASS::d_numResizes++;
	if( ptr == NULL )
		return Alloc( num );
	if( num <= 0 ) {
		Free( ptr );
		return NULL;
	}
	Free( ptr );
	return Alloc( num );
}

template< class type, int baseBlockSize, int minBlockSize >
void DynamicAlloc< type, baseBlockSize, minBlockSize >::Free( type * ptr ) {

	ScopedLock< true > scopedLock( BASECLASS::d_lock );
	BASECLASS::d_numFrees++;
	if( ptr == NULL )
		return;
	int areaIndex = d_areaList.FindIndex( ptr );
	assert( areaIndex != -1 );
	d_areaList.RemoveIndex( areaIndex );
	Mem_Free16( ptr );
}

template< class type, int baseBlockSize, int minBlockSize >
const char * DynamicAlloc< type, baseBlockSize, minBlockSize >::CheckMemory( const type * ptr ) const {
	return NULL;
}

// // //// // //// // //// //
// DynamicBlockAlloc
// //// // //// // //// //

template< class type, int baseBlockSize, int minBlockSize >
INLINE DynamicBlockAlloc< type, baseBlockSize, minBlockSize >::DynamicBlockAlloc( ) {
	Clear( );
}

template< class type, int baseBlockSize, int minBlockSize  >
INLINE DynamicBlockAlloc< type, baseBlockSize, minBlockSize >::~DynamicBlockAlloc( ) {
	Shutdown( );
}

template< class type, int baseBlockSize, int minBlockSize >
void DynamicBlockAlloc< type, baseBlockSize, minBlockSize >::Init( ) {
	d_freeTree.Init( );
}

template< class type, int baseBlockSize, int minBlockSize >
void DynamicBlockAlloc< type, baseBlockSize, minBlockSize >::Shutdown( ) {
	DynamicBlock< type > * block;
	for( block = d_firstBlock; block != NULL; block = block->d_next )
		if( block->d_node == NULL )
			FreeInternal( block );

	for( block = d_firstBlock; block != NULL; block = d_firstBlock ) {
		d_firstBlock = block->d_next;
		assert( block->IsBaseBlock( ) );
		//if ( BASECLASS::d_lockMemory ) {
		//	idLib::sys->UnlockMemory( block, block->GetSize( ) + ( int )sizeof( DynamicBlock< type > ) );
		//}
		Mem_Free16( block );
	}
	d_freeTree.Shutdown( );
	Clear( );
}

template< class type, int baseBlockSize, int minBlockSize >
void DynamicBlockAlloc< type, baseBlockSize, minBlockSize >::SetFixedBlocks( int numBlocks ) {
	DynamicBlock< type > * block;
	for( int i = BASECLASS::d_numBaseBlocks; i < numBlocks; i++ ) {
		block = ( DynamicBlock< type > * )Mem_Alloc16( baseBlockSize );
		//if ( BASECLASS::d_lockMemory ) {
		//	idLib::sys->BASECLASS::d_lockMemory( block, baseBlockSize );
		//}
#ifdef DYNAMIC_BLOCK_ALLOC_CHECK
		Common::Com_Memcpy( block->d_id, blockId, sizeof( block->d_id ) );
		block->d_allocator = ( void * )this;
#endif
		block->SetSize( baseBlockSize - ( int )sizeof( DynamicBlock< type > ), true );
		block->d_next = NULL;
		block->d_prev = d_lastBlock;
		if( d_lastBlock )
			d_lastBlock->d_next = block;
		else
			d_firstBlock = block;
		d_lastBlock = block;
		block->d_node = NULL;
		FreeInternal( block );
		BASECLASS::d_numBaseBlocks++;
		BASECLASS::d_baseBlockMemory += baseBlockSize;
	}
	d_allowAllocs = false;
}

template< class type, int baseBlockSize, int minBlockSize >
void DynamicBlockAlloc< type, baseBlockSize, minBlockSize >::SetLockMemory( bool lock ) {
	BASECLASS::d_lockMemory = lock;
}

template< class type, int baseBlockSize, int minBlockSize >
void DynamicBlockAlloc< type, baseBlockSize, minBlockSize >::FreeEmptyBaseBlocks( ) {
	DynamicBlock< type > * block, * next;
	for( block = d_firstBlock; block != NULL; block = next ) {
		next = block->d_next;
		if( block->IsBaseBlock( ) && block->d_node != NULL && ( next == NULL || next->IsBaseBlock( ) ) ) {
			UnlinkFreeInternal( block );
			if( block->d_prev )
				block->d_prev->d_next = block->d_next;
			else
				d_firstBlock = block->d_next;
			if( block->d_next )
				block->d_next->d_prev = block->d_prev;
			else
				d_lastBlock = block->d_prev;
			//if ( BASECLASS::d_lockMemory ) {
			//	idLib::sys->UnlockMemory( block, block->GetSize( ) + ( int )sizeof( DynamicBlock< type > ) );
			//}
			BASECLASS::d_numBaseBlocks--;
			BASECLASS::d_baseBlockMemory -= block->GetSize( ) + ( int )sizeof( DynamicBlock< type > );
			Mem_Free16( block );
		}
	}

#ifdef DYNAMIC_BLOCK_ALLOC_CHECK
	CheckMemory( );
#endif
}

template< class type, int baseBlockSize, int minBlockSize >
int DynamicBlockAlloc< type, baseBlockSize, minBlockSize >::GetNumEmptyBaseBlocks( ) const {
	int numEmptyBaseBlocks;
	DynamicBlock< type > * block;
	numEmptyBaseBlocks = 0;
	for( block = d_firstBlock; block != NULL; block = block->d_next )
		if( block->IsBaseBlock( ) && block->d_node != NULL && ( block->d_next == NULL || block->d_next->IsBaseBlock( ) ) )
			numEmptyBaseBlocks++;
	return numEmptyBaseBlocks;
}

template< class type, int baseBlockSize, int minBlockSize >
type * DynamicBlockAlloc< type, baseBlockSize, minBlockSize >::Alloc( const int num ) {
	ScopedLock< true > scopedLock( BASECLASS::d_lock );
	DynamicBlock< type > * block;
	BASECLASS::d_numAllocs++;
	if( num <= 0 )
		return NULL;
	block = AllocInternal( num );
	if( block == NULL )
		return NULL;
	block = ResizeInternal( block, num );
	if( block == NULL )
		return NULL;
#ifdef DYNAMIC_BLOCK_ALLOC_CHECK
	CheckMemory( );
#endif
	BASECLASS::d_numUsedBlocks++;
	BASECLASS::d_usedBlockMemory += block->GetSize( );
	return block->GetMemory( );
}

template< class type, int baseBlockSize, int minBlockSize >
type * DynamicBlockAlloc< type, baseBlockSize, minBlockSize >::Resize( type * ptr, const int num ) {
	ScopedLock< true > scopedLock( BASECLASS::d_lock );
	BASECLASS::d_numResizes++;
	if( ptr == NULL )
		return Alloc( num );
	if( num <= 0 ) {
		Free( ptr );
		return NULL;
	}
	DynamicBlock< type > * block = ( DynamicBlock< type > * ) ( ( ( byte * ) ptr ) - ( int )sizeof( DynamicBlock< type > ) );
	BASECLASS::d_usedBlockMemory -= block->GetSize( );
	block = ResizeInternal( block, num );
	if( block == NULL )
		return NULL;
#ifdef DYNAMIC_BLOCK_ALLOC_CHECK
	CheckMemory( );
#endif
	BASECLASS::d_usedBlockMemory += block->GetSize( );
	return block->GetMemory( );
}

template< class type, int baseBlockSize, int minBlockSize >
void DynamicBlockAlloc< type, baseBlockSize, minBlockSize >::Free( type * ptr ) {
	ScopedLock< true > scopedLock( BASECLASS::d_lock );
	BASECLASS::d_numFrees++;
	if( ptr == NULL )
		return;
	DynamicBlock< type > * block = ( DynamicBlock< type > * ) ( ( ( byte * ) ptr ) - ( int )sizeof( DynamicBlock< type > ) );

	BASECLASS::d_numUsedBlocks--;
	BASECLASS::d_usedBlockMemory -= block->GetSize( );
	FreeInternal( block );
#ifdef DYNAMIC_BLOCK_ALLOC_CHECK
	CheckMemory( );
#endif
}

template< class type, int baseBlockSize, int minBlockSize >
const char * DynamicBlockAlloc< type, baseBlockSize, minBlockSize >::CheckMemory( const type * ptr ) const {
	DynamicBlock< type > * block;
	if( ptr == NULL )
		return NULL;
	block = ( DynamicBlock< type > * ) ( ( ( byte * ) ptr ) - ( int )sizeof( DynamicBlock< type > ) );
	if( block->d_node != NULL )
		return "memory has been freed";
#ifdef DYNAMIC_BLOCK_ALLOC_CHECK
	if( block->id[ 0 ] != 0x11111111 || block->id[ 1 ] != 0x22222222 || block->id[ 2 ] != 0x33333333 )
		return "memory has invalid id";
	if( block->allocator != (void*)this )
		return "memory was allocated with different allocator";
#endif
	/* base blocks can be larger than baseBlockSize which can cause this code to fail
	DynamicBlock< type > * base;
	for( base = d_firstBlock; base != NULL; base = base->d_next )
		if( base->IsBaseBlock( ) )
			if( (( int )block) >= (( int )base) && (( int )block) < (( int )base) + baseBlockSize )
				break;
	if( base == NULL )
		return "no base block found for memory";
	*/
	return NULL;
}

template< class type, int baseBlockSize, int minBlockSize >
INLINE void DynamicBlockAlloc< type, baseBlockSize, minBlockSize >::Clear( ) {
	d_firstBlock = d_lastBlock = NULL;
	d_allowAllocs = true;
	BASECLASS::ClearBase( );
#ifdef DYNAMIC_BLOCK_ALLOC_CHECK
	blockId[ 0 ] = 0x11111111;
	blockId[ 1 ] = 0x22222222;
	blockId[ 2 ] = 0x33333333;
#endif
}

template< class type, int baseBlockSize, int minBlockSize >
INLINE DynamicBlock< type > * DynamicBlockAlloc< type, baseBlockSize, minBlockSize >::AllocInternal( const int num ) {
	DynamicBlock< type > * block;
	int alignedBytes = ( num * sizeof( type ) + 15 ) & ~15;
	block = d_freeTree.FindSmallestLargerEqual( alignedBytes );
	if( block != NULL )
		UnlinkFreeInternal( block );
	else if( d_allowAllocs ) {
		int allocSize = Max( baseBlockSize, alignedBytes + ( int )sizeof( DynamicBlock< type > ) );
		block = ( DynamicBlock< type > * ) Mem_Alloc16( allocSize );
		//if ( BASECLASS::d_lockMemory ) {
		//	idLib::sys->BASECLASS::d_lockMemory( block, baseBlockSize );
		//}
#ifdef DYNAMIC_BLOCK_ALLOC_CHECK
		Common::Com_Memcpy( block->id, blockId, sizeof( block->id ) );
		block->allocator = ( void * )this;
#endif
		block->SetSize( allocSize - ( int )sizeof( DynamicBlock< type > ), true );
		block->d_next = NULL;
		block->d_prev = d_lastBlock;
		if( d_lastBlock )
			d_lastBlock->d_next = block;
		else
			d_firstBlock = block;
		d_lastBlock = block;
		block->d_node = NULL;
		BASECLASS::d_numBaseBlocks++;
		BASECLASS::d_baseBlockMemory += allocSize;
	}
	return block;
}

template< class type, int baseBlockSize, int minBlockSize >
INLINE DynamicBlock< type > * DynamicBlockAlloc< type, baseBlockSize, minBlockSize >::ResizeInternal( DynamicBlock< type > * block, const int num ) {
	int alignedBytes = ( num * sizeof( type ) + 15 ) & ~15;
#ifdef DYNAMIC_BLOCK_ALLOC_CHECK
	assert( block->id[ 0 ] == 0x11111111 && block->id[ 1 ] == 0x22222222 && block->id[ 2 ] == 0x33333333 && block->allocator == ( void * )this );
#endif
	// if the new size is larger
	if( alignedBytes > block->GetSize( ) ) {
		DynamicBlock< type > * nextBlock = block->d_next;
		// try to annexate the next block if it's free
		if( nextBlock && !nextBlock->IsBaseBlock( ) && nextBlock->d_node != NULL && block->GetSize( ) + ( int )sizeof( DynamicBlock< type > ) + nextBlock->GetSize( ) >= alignedBytes ) {
			UnlinkFreeInternal( nextBlock );
			block->SetSize( block->GetSize( ) + ( int )sizeof( DynamicBlock< type > ) + nextBlock->GetSize( ), block->IsBaseBlock( ) );
			block->d_next = nextBlock->d_next;
			if( nextBlock->d_next )
				nextBlock->d_next->d_prev = block;
			else
				d_lastBlock = block;
		} else {
			// allocate a new block and copy
			DynamicBlock< type > * oldBlock = block;
			block = AllocInternal( num );
			if( block == NULL )
				return NULL;
			Common::Com_Memcpy( block->GetMemory( ), oldBlock->GetMemory( ), oldBlock->GetSize( ) );
			FreeInternal( oldBlock );
		}
	}
	// if the unused space at the end of this block is large enough to hold a block with at least one element
	if( block->GetSize( ) - alignedBytes - ( int )sizeof( DynamicBlock< type > ) < Max( minBlockSize, ( int )sizeof( type ) ) )
		return block;
	DynamicBlock< type > * newBlock;
	newBlock = ( DynamicBlock< type > * ) ( ( ( byte * ) block ) + ( int )sizeof( DynamicBlock< type > ) + alignedBytes );
#ifdef DYNAMIC_BLOCK_ALLOC_CHECK
	Common::Com_Memcpy( newBlock->id, blockId, sizeof( newBlock->id ) );
	newBlock->allocator = (void*)this;
#endif
	newBlock->SetSize( block->GetSize( ) - alignedBytes - ( int )sizeof( DynamicBlock< type > ), false );
	newBlock->d_next = block->d_next;
	newBlock->d_prev = block;
	if( newBlock->d_next )
		newBlock->d_next->d_prev = newBlock;
	else
		d_lastBlock = newBlock;
	newBlock->d_node = NULL;
	block->d_next = newBlock;
	block->SetSize( alignedBytes, block->IsBaseBlock( ) );
	FreeInternal( newBlock );
	return block;
}

template< class type, int baseBlockSize, int minBlockSize >
INLINE void DynamicBlockAlloc< type, baseBlockSize, minBlockSize >::FreeInternal( DynamicBlock< type > * block ) {
	assert( block->d_node == NULL );
#ifdef DYNAMIC_BLOCK_ALLOC_CHECK
	assert( block->id[ 0 ] == 0x11111111 && block->id[ 1 ] == 0x22222222 && block->id[ 2 ] == 0x33333333 && block->allocator == (void*)this );
#endif
	// try to merge with a next free block
	DynamicBlock< type > * nextBlock = block->d_next;
	if( nextBlock && !nextBlock->IsBaseBlock( ) && nextBlock->d_node != NULL ) {
		UnlinkFreeInternal( nextBlock );
		block->SetSize( block->GetSize( ) + ( int )sizeof( DynamicBlock< type > ) + nextBlock->GetSize( ), block->IsBaseBlock( ) );
		block->d_next = nextBlock->d_next;
		if( nextBlock->d_next )
			nextBlock->d_next->d_prev = block;
		else
			d_lastBlock = block;
	}

	// try to merge with a previous free block
	DynamicBlock< type > * prevBlock = block->d_prev;
	if( prevBlock && !block->IsBaseBlock( ) && prevBlock->d_node != NULL ) {
		UnlinkFreeInternal( prevBlock );
		prevBlock->SetSize( prevBlock->GetSize( ) + ( int )sizeof( DynamicBlock< type > ) + block->GetSize( ), prevBlock->IsBaseBlock( ) );
		prevBlock->d_next = block->d_next;
		if( block->d_next )
			block->d_next->d_prev = prevBlock;
		else
			d_lastBlock = prevBlock;
		LinkFreeInternal( prevBlock );
	} else
		LinkFreeInternal( block );
}

template< class type, int baseBlockSize, int minBlockSize >
INLINE void DynamicBlockAlloc< type, baseBlockSize, minBlockSize >::LinkFreeInternal( DynamicBlock< type > * block ) {
	block->d_node = d_freeTree.Add( block, block->GetSize( ) );
	BASECLASS::d_numFreeBlocks++;
	BASECLASS::d_freeBlockMemory += block->GetSize( );
}

template< class type, int baseBlockSize, int minBlockSize >
INLINE void DynamicBlockAlloc< type, baseBlockSize, minBlockSize >::UnlinkFreeInternal( DynamicBlock< type > * block ) {
	d_freeTree.Remove( block->d_node );
	block->d_node = NULL;
	BASECLASS::d_numFreeBlocks--;
	BASECLASS::d_freeBlockMemory -= block->GetSize( );
}

template< class type, int baseBlockSize, int minBlockSize >
INLINE void DynamicBlockAlloc< type, baseBlockSize, minBlockSize >::CheckMemory( ) const {
	for( DynamicBlock< type > * block = d_firstBlock; block != NULL; block = block->d_next ) {
		if( block->d_prev == NULL ) // make sure the block is properly linked
			assert( d_firstBlock == block );
		else
			assert( block->d_prev->d_next == block );
		if( block->d_next == NULL )
			assert( d_lastBlock == block );
		else
			assert( block->d_next->d_prev == block );
	}
}

#undef BASECLASS

#endif
