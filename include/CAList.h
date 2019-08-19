#ifndef CALIST_H
#define CALIST_H

// // //// // //// // //// //
/// CAListBase
//
/// List with a custom allocator
// //// // //// // //// //

template< class type > class CAListBase {
protected:
	/// number of elements
	int						d_num;
	/// allocated d_size
	int						d_size;
	/// next block allocation d_size
	int						d_granularity;
	/// d_list data
	type *					d_list;
	int						GranulateSize( int size );

public:
	typedef int				cmp_t( const type *, const type * );
	typedef int				cmp_c( const void *, const void * );
	/// Frees up the memory allocated by the d_list. Assumes that type automatically handles freeing up memory.
	virtual void			Clear( ) = 0;
	/// Allocates memory for the amount of elements requested while keeping the contents intact.
	/// Contents are copied using their = operator so that data is correnctly instantiated.
	virtual void			Resize( int newsize ) = 0;
	/// Returns the number of elements currently contained in the d_list.
	/// Note that this is NOT an indication of the memory allocated.
	int						Num( ) const;
	/// Returns the number of elements currently allocated for.
	int						NumAllocated( ) const;
	/// Sets the base d_size of the array and resizes the array to match.
	void					SetGranularity( int newgranularity );
	/// get the current d_granularity
	int						GetGranularity( ) const;
	/// return total memory allocated for the d_list in bytes, but doesn't take into account additional memory allocated by type
	size_t					Allocated( ) const;
	/// returns total d_size of allocated memory including d_size of d_list type
	size_t					Size( ) const;
	/// returns d_size of the used elements in the d_list
	size_t					MemoryUsed( ) const;
	/// Copies the contents and d_size attributes of another d_list.
	CAListBase< type > &	operator =( const CAListBase< type > & other );
	/// Access operator. Index must be within range or an assert will be issued in debug builds. Release builds do no range checking.
	const type &			operator[ ]( int index ) const;
	/// Access operator. Index must be within range or an assert will be issued in debug builds. Release builds do no range checking.
	type &					operator[ ]( int index );
	/// Resizes the array to exactly the number of elements it contains or frees up memory if empty.
	void					Condense( );
	/// Resize to the exact d_size specified irregardless of d_granularity
	void					SetNum( int newnum, bool resize = true );
	/// Makes sure the d_list has at least the given number of elements.
	void					AssureSize( int newSize );
	/// Makes sure the d_list has at least the given number of elements and initialize any elements not yet initialized.
	void					AssureSize( int newSize, const type & initValue );
	/// Returns a pointer to the begining of the array.  Useful for iterating through the d_list in loops.
	/// Note: may return NULL if the d_list is empty.
	type *					Ptr( );
	/// Returns a pointer to the begining of the array.  Useful for iterating through the d_list in loops.
	/// Note: may return NULL if the d_list is empty.
	const type *			Ptr( ) const;
	/// Returns a reference to a new data element at the end of the d_list.
	type &					Alloc( );
	/// Decreases d_num by 1
	void					RemoveLast( );
	/// Increases the d_size of the d_list by one element and copies the supplied data into it. Returns the index of the new element.
	int						Append( const type & obj );
	/// Adds the other d_list to this one. Returns the d_size of the new combined d_list
	int						Append( const CAListBase< type > & other );
	/// Appends an array
	int						Append( const type * array_ptr, int array_size );
	/// Adds the data to the d_list if it doesn't already exist. Returns the index of the data in the d_list.
	int						AddUnique( const type & obj );
	/// Increases the d_size of the d_list by at leat one element if necessary and inserts the supplied data into it. Returns the index of the new element.
	int						Insert( const type & obj, int index = 0 );
	/// Searches for the specified data in the d_list and returns it's index. Returns -1 if the data is not found.
	int						FindIndex( type const & obj ) const;
	/// Searches for the specified data in the d_list and returns it's address. Returns NULL if the data is not found.
	type *					Find( type const & obj ) const;
	type &					FindSmallest( );
	/// Searches for a NULL pointer in the d_list.  Returns -1 if NULL is not found.
	/// NOTE: This function can only be called on lists containing pointers. Calling it on non-pointer lists will cause a compiler error.
	int						FindNull( ) const;
	/// Takes a pointer to an element in the d_list and returns the index of the element.
	/// NOTE: This is NOT a guarantee that the object is really in the d_list. 
	/// Function will assert in debug builds if pointer is outside the bounds of the d_list, but remains silent in release builds.
	int						IndexOf( const type * obj ) const;
	/// Removes the element at the specified index, then calls the delete operator, only good for ptrs...
	bool					DeleteIndex( int index );
	/// Removes the element at the specified index and moves all data following the element down to fill in the gap.
	/// The number of elements in the d_list is reduced by one.  Returns false if the index is outside the bounds of the d_list.
	/// Note that the element is not destroyed, so any memory used by it may not be freed until the destruction of the d_list.
	bool					RemoveIndex( int index );
	/// 
	bool					RemoveIndexMemcpy( int index );
	/// Removes the element at the specified index and moves the last element into it's spot, rather than moving the whole array down by one.
	/// Of course, this doesn't maintain the order of elements!
	/// The number of elements in the d_list is reduced by one.  Returns false if the index is outside the bounds of the d_list.
	/// Note that the element is not destroyed, so any memory used by it may not be freed until the destruction of the d_list.
	bool					RemoveIndexFast( int index );
	/// Removes the element if it is found within the d_list and moves all data following the element down to fill in the gap.
	/// The number of elements in the d_list is reduced by one.  Returns false if the data is not found in the d_list.
	/// Note that the element is not destroyed, so any memory used by it may not be freed until the destruction of the d_list.
	bool					Remove( const type & obj );
	/// Removes the element if it is found within the d_list and moves the last element into the gap.
	/// The number of elements in the d_list is reduced by one.  Returns false if the data is not found in the d_list.
	/// Note that the element is not destroyed, so any memory used by it may not be freed until the destruction of the d_list.
	bool					RemoveFast( const type & obj );
	/// Performs a qsort on the d_list using the supplied comparison function.  Note that the data is merely moved around the d_list, so any pointers to data within the d_list may no longer be valid.
	void					Sort( cmp_t * compare = ( cmp_t * )&ListSortCompare< type > );
	/// Sorts a subsection of the d_list.
	void					SortSubSection( int startIndex, int endIndex, cmp_t * compare = ( cmp_t * )&ListSortCompare< type > );
	/// Swaps the contents of two lists
	void					Swap( CAListBase< type > & other );
	/// Calls the destructor of all elements in the d_list.  Conditionally frees up memory used by the d_list.
	/// Note that this only works on lists containing pointers to objects and will cause a compiler error if called with non-pointers.
	/// Since the d_list was not responsible for allocating the object, it has no information on whether the object still exists or not,
	/// so care must be taken to ensure that the pointers are still valid when this function is called.
	/// Function will set all pointers in the d_list to NULL.
	void					DeleteContents( bool clear );
	void					SaveFile( const Str & file_name ) const;
	void					LoadFile( const Str & file_name );
};

#define CALISTSIZEALIGN 16

// // //// // //// // //// //
/// CAList
//
/// An implementetion of CAListBase with internal allocator
// //// // //// // //// //

template< class type, int static_size = 1 > class CAList : public CAListBase< type > {
protected:
	type					d_staticList[ static_size ];

public:
							CAList( int newgranularity = 0 );
							CAList( const CAListBase< type > & other );
							~CAList( );

	virtual void			Clear( );
	virtual void			Resize( int newsize );
};

// // //// // //// // //// //
/// DList
//
/// using pure dynamic allocator
// //// // //// // //// //

template< class type > class DList : public CAListBase< type > {
public:
							DList( int newgranularity = 0 );
							DList( const CAListBase< type > & other );
							~DList( );

	virtual void			Clear( );
	virtual void			Resize( int newsize );
};

// // //// // //// // //// //
// CAListBase
// //// // //// // //// //

template< class type >
INLINE int CAListBase< type >::GranulateSize( int size ) {
	if( d_granularity <= 0 )
		SetGranularity( (1024*4) / sizeof( type ) );
	int newsize = size + d_granularity - 1;
	return newsize - ( newsize % d_granularity );
}

template< class type >
INLINE void CAListBase< type >::DeleteContents( bool clear ) {
	for( int i = 0; i < d_num; i++ ) {
		delete d_list[ i ];
		d_list[ i ] = NULL;
	}
	if( clear )
		Clear( );
	else Common::Com_Memset( d_list, 0, d_size * sizeof( type ) );
}

template< class type >
INLINE size_t CAListBase< type >::Allocated( ) const {
	return NumSizeOf( type, d_size );
}

template< class type >
INLINE size_t CAListBase< type >::Size( ) const {
	return sizeof( CAListBase< type > ) + NumSizeOf( type, d_size );
}

template< class type >
INLINE size_t CAListBase< type >::MemoryUsed( ) const {
	return NumSizeOf( type, d_num );
}

template< class type >
INLINE int CAListBase< type >::Num( ) const {
	return d_num;
}

template< class type >
INLINE int CAListBase< type >::NumAllocated( ) const {
	return d_size;
}

template< class type >
INLINE void CAListBase< type >::SetNum( int newnum, bool resize ) {
	assert( newnum >= 0 );
	if( resize || newnum > d_size )
		Resize( newnum );
	d_num = newnum;
}

template< class type >
INLINE void CAListBase< type >::SetGranularity( int newgranularity ) {
	assert( newgranularity > 0 );
	d_granularity = newgranularity;
	if( d_list ) { // resize it to the closest level of d_granularity
		int newsize = GranulateSize( d_num );
		if( newsize != d_size )
			Resize( newsize );
	}
}

template< class type >
INLINE int CAListBase< type >::GetGranularity( ) const {
	return d_granularity;
}

template< class type >
INLINE void CAListBase< type >::Condense( ) {
	if( d_list ) {
		if( d_num )
			Resize( d_num );
		else Clear( );
	}
}

template< class type >
INLINE void CAListBase< type >::AssureSize( int newSize ) {
	if( newSize > d_size )
		Resize( GranulateSize( newSize ) );
	d_num = newSize;
}

template< class type >
INLINE void CAListBase< type >::AssureSize( int newSize, const type & initValue ) {
	if( newSize > d_size )
		Resize( GranulateSize( newSize ) );
	for( int i = d_num; i < newSize; i++ )
		d_list[ i ] = initValue;
	d_num = newSize;
}

template< class type >
INLINE CAListBase< type > & CAListBase< type >::operator =( const CAListBase< type > & other ) {
	Resize( other.d_size );
	d_num				= other.d_num;
	d_granularity		= other.d_granularity;
	for( int i = 0; i < d_num; i++ )
		d_list[ i ] = other.d_list[ i ];
	return *this;
}

template< class type >
INLINE const type & CAListBase< type >::operator[ ]( int index ) const {
	assert( index >= 0 && index < d_num );
	return d_list[ index ];
}

template< class type >
INLINE type & CAListBase< type >::operator[ ]( int index ) {
	assert( index >= 0 && index < d_num );
	return d_list[ index ];
}

template< class type >
INLINE type * CAListBase< type >::Ptr( ) {
	return d_list;
}

template< class type >
INLINE const type * CAListBase< type >::Ptr( ) const {
	return d_list;
}

template< class type >
INLINE type & CAListBase< type >::Alloc( ) {
	if( !d_list || ( d_num == d_size && !d_granularity ) )
		Resize( GranulateSize( 1 ) );
	if( d_num == d_size )
		Resize( d_size + d_granularity );
	return d_list[ d_num++ ];
}

template< class type >
INLINE void CAListBase< type >::RemoveLast( ) {
	assert( d_num );
	d_num--;
}

template< class type >
INLINE int CAListBase< type >::Append( type const & obj ) {
	if( !d_list || ( d_num == d_size && !d_granularity ) )
		Resize( GranulateSize( 1 ) );
	if( d_num == d_size )
		Resize( GranulateSize( d_size + d_granularity ) );
	d_list[ d_num ] = obj;
	d_num++;
	return d_num - 1;
}

template< class type >
INLINE int CAListBase< type >::Insert( type const & obj, int index ) {
	if( !d_list || ( d_num == d_size && !d_granularity ) )
		Resize( GranulateSize( 1 ) );
	if( d_num == d_size )
		Resize( GranulateSize( d_size + d_granularity ) );
	if( index < 0 )
		index = 0;
	else if( index > d_num )
		index = d_num;
	for( int i = d_num; i > index; --i )
		d_list[ i ] = d_list[ i - 1 ];
	d_num++;
	d_list[ index ] = obj;
	return index;
}

template< class type >
INLINE int CAListBase< type >::Append( const type * array_ptr, int array_size ) {
	Resize( GranulateSize( d_num + array_size ) );
	for( int i = 0; i < array_size; i++ )
		Append( array_ptr[ i ] );
	return Num( );
}

template< class type >
INLINE int CAListBase< type >::Append( const CAListBase< type > & other ) {
	int numOther = other.Num( );
	Resize( GranulateSize( d_num + numOther ) );
	for( int i = 0; i < numOther; i++ )
		Append( other[ i ] );
	return Num( );
}

template< class type >
INLINE int CAListBase< type >::AddUnique( type const & obj ) {
	int index = FindIndex( obj );
	if( index < 0 )
		index = Append( obj );
	return index;
}

template< class type >
INLINE int CAListBase< type >::FindIndex( type const & obj ) const {
	for( int i = 0; i < d_num; i++ ) {
		if( d_list[ i ] == obj )
			return i;
	}
	return -1;
}

template< class type >
INLINE type *CAListBase< type >::Find( type const & obj ) const {
	int i  = FindIndex( obj );
	if( i >= 0 )
		return &d_list[ i ];
	return NULL;
}

template< class type >
INLINE type & CAListBase< type >::FindSmallest( ) {
	type * ptr = d_list;
	for( int i = 1; i < d_num; i++ ) {
		if( d_list[ i ] < ( *ptr ) )
			ptr = &d_list[ i ];
	}
	return *ptr;
}

template< class type >
INLINE int CAListBase< type >::FindNull( ) const {
	for( int i = 0; i < d_num; i++ ) {
		if( d_list[ i ] == NULL )
			return i;
	}
	return -1;
}

template< class type >
INLINE int CAListBase< type >::IndexOf( type const * objptr ) const {
	int index = objptr - d_list;
	assert( index >= 0 && index < d_num );
	return index;
}

template< class type >
INLINE bool CAListBase< type >::DeleteIndex( int index ) {
	assert( index >= 0 && index < d_num );
	if( ( index < 0 ) || ( index >= d_num ) )
		return false;
	d_num--;
	delete d_list[ index ];
	for( int i = index; i < d_num; i++ )
		d_list[ i ] = d_list[ i + 1 ];
	return true;
}

template< class type >
INLINE bool CAListBase< type >::RemoveIndex( int index ) {
	assert( index >= 0 && index < d_num );
	if( ( index < 0 ) || ( index >= d_num ) )
		return false;
	d_num--;
	for( int i = index; i < d_num; i++ )
		d_list[ i ] = d_list[ i + 1 ];
	return true;
}

template< class type >
INLINE bool CAListBase< type >::RemoveIndexMemcpy( int index ) {
	assert( index >= 0 && index < d_num );
	if( ( index < 0 ) || ( index >= d_num ) )
		return false;
	d_num--;
	__movsb( ( byte * )&d_list[ index ], ( byte * )&d_list[ index + 1 ], sizeof( type ) * ( d_num - index ) );
	return true;
}

template< class type >
INLINE bool CAListBase< type >::RemoveIndexFast( int index ) {
	assert( index >= 0 && index < d_num );
	if( ( index < 0 ) || ( index >= d_num ) )
		return false;
	d_num--;
	if( index == d_num )
		return true;
	d_list[ index ] = d_list[ d_num ];		
	return true;
}

template< class type >
INLINE bool CAListBase< type >::Remove( type const & obj ) {
	int index = FindIndex( obj );
	if( index >= 0 )
		return RemoveIndex( index );	
	return false;
}

template< class type >
INLINE bool CAListBase< type >::RemoveFast( type const & obj ) {
	int index = FindIndex( obj );
	if( index >= 0 )
		return RemoveIndexFast( index );	
	return false;
}

template< class type >
INLINE void CAListBase< type >::Sort( cmp_t * compare ) {
	if( !d_list )
		return;
	cmp_c * vCompare = ( cmp_c * )compare;
	qsort( ( void * )d_list, ( size_t )d_num, sizeof( type ), vCompare );
}

template< class type >
INLINE void CAListBase< type >::SortSubSection( int startIndex, int endIndex, cmp_t * compare ) {
	if( !d_list )
		return;
	if( startIndex < 0 )
		startIndex = 0;
	if( endIndex >= d_num )
		endIndex = d_num - 1;
	if( startIndex >= endIndex )
		return;
	cmp_c * vCompare = ( cmp_c * )compare;
	qsort( ( void * )( &d_list[startIndex] ), ( size_t )( endIndex - startIndex + 1 ), sizeof( type ), vCompare );
}

template< class type >
INLINE void CAListBase< type >::Swap( CAListBase< type > & other ) {
	::Swap( d_num, other.d_num );
	::Swap( d_size, other.d_size );
	::Swap( d_granularity, other.d_granularity );
	::Swap( d_list, other.d_list );
}

// // //// // //// // //// //
// DList
// //// // //// // //// //

template< class type >
DList< type >::DList( int newgranularity ) {
	CAListBase< type >::d_granularity = newgranularity;
	CAListBase< type >::d_list	= NULL;
	Clear( );
	if( !newgranularity )
		GranulateSize( 1 );
}

template< class type >
DList< type >::DList( const CAListBase< type > & other ) {
	CAListBase< type >::d_granularity = other.GetGranularity( );
	CAListBase< type >::d_list	= NULL;
	Clear( );
	Append( other );
}

template< class type >
DList< type >::~DList( ) {
	Clear( );
}

template< class type >
void DList< type >::Clear( ) {
	if( CAListBase< type >::d_list ) {
		for( int i = 0; i < CAListBase< type >::d_size; i++ )
			( CAListBase< type >::d_list + i )->~type( );
		Mem_Free( CAListBase< type >::d_list );
	}
	CAListBase< type >::d_list	= NULL;
	CAListBase< type >::d_num	= 0;
	CAListBase< type >::d_size	= 0;
}

template< class type >
void DList< type >::Resize( int newsize ) {
	assert( newsize >= 0 );
	if( !newsize ) {  // free up the d_list if no data is being reserved
		Clear( );
		return;
	}
	if( newsize == CAListBase< type >::d_size )
		return; // not changing the d_size, so just exit
	type * temp					= CAListBase< type >::d_list;
	int oldSize					= CAListBase< type >::d_size;
	CAListBase< type >::d_size	= newsize;
	if( CAListBase< type >::d_size < CAListBase< type >::d_num )
		CAListBase< type >::d_num = CAListBase< type >::d_size;
	int allocSize = NumSizeOf( type, CAListBase< type >::d_size ) + CALISTSIZEALIGN - 1;
	CAListBase< type >::d_list = ( type * )Mem_Alloc( allocSize - ( allocSize % CALISTSIZEALIGN ) ); // copy the old d_list into our new one
	for( int i = 0; i < CAListBase< type >::d_size; i++ )
		new ( CAListBase< type >::d_list + i )( type );
	for( int i = 0; i < CAListBase< type >::d_num; i++ )
		CAListBase< type >::d_list[ i ] = temp[ i ];
	for( int i = 0; i < oldSize; i++ )
		( temp + i )->~type( );
	Mem_Free( temp ); // delete the old d_list if it exists
}

// // //// // //// // //// //
// CAList
// //// // //// // //// //

template< class type, int static_size >
CAList< type, static_size >::CAList( int newgranularity ) {
	CAListBase< type >::d_granularity = newgranularity ? newgranularity : ( static_size != 1 ) ? static_size : 0;
	CAListBase< type >::d_list	= d_staticList;
	Clear( );
}

template< class type, int static_size >
CAList< type, static_size >::CAList( const CAListBase< type > & other ) {
	if( static_size >= other.Num( ) )
		CAListBase< type >::d_granularity = static_size;
	else
		CAListBase< type >::d_granularity = other.GetGranularity( );
	CAListBase< type >::d_list	= d_staticList;
	Clear( );
	Append( other );
}

template< class type, int static_size >
CAList< type, static_size >::~CAList( ) {
	Clear( );
}

template< class type, int static_size >
void CAList< type, static_size >::Clear( ) {
	if( CAListBase< type >::d_list != d_staticList ) {
		for( int i = 0; i < CAListBase< type >::d_size; i++ )
			( CAListBase< type >::d_list + i )->~type( );
		Mem_Free( CAListBase< type >::d_list );
	}
	CAListBase< type >::d_list	= d_staticList;
	CAListBase< type >::d_num	= 0;
	CAListBase< type >::d_size	= static_size;
}

template< class type, int static_size >
void CAList< type, static_size >::Resize( int newsize ) {
	void *		Mem_Alloc( const size_t size );
	assert( newsize >= 0 );
	if( !newsize ) {  // free up the d_list if no data is being reserved
		Clear( );
		return;
	}
	if( newsize == CAListBase< type >::d_size || ( ( CAListBase< type >::d_list == d_staticList ) && ( newsize <= static_size ) ) )
		return; // not changing the d_size, so just exit
	type * temp					= CAListBase< type >::d_list;
	int oldSize					= CAListBase< type >::d_size;
	CAListBase< type >::d_size	= newsize;
	if( CAListBase< type >::d_size < CAListBase< type >::d_num )
		CAListBase< type >::d_num = CAListBase< type >::d_size;
	if( newsize > static_size ) {
		int allocSize = NumSizeOf( type, CAListBase< type >::d_size ) + CALISTSIZEALIGN - 1;
		CAListBase< type >::d_list = ( type * )Mem_Alloc( allocSize - ( allocSize % CALISTSIZEALIGN ) ); // copy the old d_list into our new one
	} else {
		CAListBase< type >::d_list = d_staticList;
		CAListBase< type >::d_size = static_size;
	}
	for( int i = 0; i < CAListBase< type >::d_size; i++ )
		new ( CAListBase< type >::d_list + i )( type );
	for( int i = 0; i < CAListBase< type >::d_num; i++ )
		CAListBase< type >::d_list[ i ] = temp[ i ];
	if( temp != d_staticList ) {
		for( int i = 0; i < oldSize; i++ )
			( temp + i )->~type( );
		Mem_Free( temp ); // delete the old d_list if it exists
	}
}

#endif
