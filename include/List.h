#ifndef LIST_H
#define LIST_H

template< class type >
INLINE int ListSortCompare( const type * a, const type * b ) {
	return *a - *b;
}

// // //// // //// // //// //
/// List
//
/// Does not allocate memory until the first item is added.
// //// // //// // //// //
template< class type > class List {
protected:
	/// number of elements
	int						d_num;
	/// allocated d_size
	int						d_size;
	/// next block allocation d_size
	int						d_granularity;
	/// d_list data
	type *					d_list;

public:
	typedef int				cmp_t( const type *, const type * );
	typedef type			new_t( );
							List( int newgranularity = 16 );
							List( const List< type > & other );
							~List< type >( );
	/// Frees up the memory allocated by the d_list. Assumes that type automatically handles freeing up memory.
	void					Clear( );
	/// Returns the number of elements currently contained in the d_list.
	/// Note that this is NOT an indication of the memory allocated.
	int						Num( ) const;
	/// Returns the number of elements currently allocated for.
	int						NumAllocated( ) const;
	/// Sets the base d_size of the array and resizes the array to match.
	void					SetGranularity( int newgranularity = 16 );
	/// get the current d_granularity
	int						GetGranularity( ) const;
	/// return total memory allocated for the d_list in bytes, but doesn't take into account additional memory allocated by type
	size_t					Allocated( ) const;
	/// returns total d_size of allocated memory including d_size of d_list type
	size_t					Size( ) const;
	/// returns d_size of the used elements in the d_list
	size_t					MemoryUsed( ) const;
	/// Copies the contents and d_size attributes of another d_list.
	List< type > &			operator =( const List< type > & other );
	/// Access operator. Index must be within range or an assert will be issued in debug builds. Release builds do no range checking.
	const type &			operator[ ]( int index ) const;
	/// Access operator. Index must be within range or an assert will be issued in debug builds. Release builds do no range checking.
	type &					operator[ ]( int index );
	/// Resizes the array to exactly the number of elements it contains or frees up memory if empty.
	void					Condense( );
	/// Allocates memory for the amount of elements requested while keeping the contents intact.
	/// Contents are copied using their = operator so that data is correnctly instantiated.
	void					Resize( int newsize );
	/// Allocates memory for the amount of elements requested while keeping the contents intact.
	/// Contents are copied using their = operator so that data is correnctly instantiated.
	void					Resize( int newsize, int newgranularity );
	/// Resize to the exact d_size specified irregardless of d_granularity
	void					SetNum( int newnum, bool resize = true );
	/// Makes sure the d_list has at least the given number of elements.
	void					AssureSize( int newSize );
	/// Makes sure the d_list has at least the given number of elements and initialize any elements not yet initialized.
	void					AssureSize( int newSize, const type & initValue );
	/// Makes sure the d_list has at least the given number of elements and allocates any elements using the allocator.
	/// NOTE: This function can only be called on lists containing pointers. Calling it on non-pointer lists will cause a compiler error.
	void					AssureSizeAlloc( int newSize, new_t * allocator );
	/// Returns a pointer to the begining of the array.  Useful for iterating through the d_list in loops.
	/// Note: may return NULL if the d_list is empty.
	type *					Ptr( );
	/// Returns a pointer to the begining of the array.  Useful for iterating through the d_list in loops.
	/// Note: may return NULL if the d_list is empty.
	const type *			Ptr( ) const;
	/// Returns a reference to a new data element at the end of the d_list.
	type &					Alloc( );
	/// Increases the d_size of the d_list by one element and copies the supplied data into it. Returns the index of the new element.
	int						Append( const type & obj );
	/// Adds the other d_list to this one. Returns the d_size of the new combined d_list
	int						Append( const List< type > & other );
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
	void					SortSubSection( int startIndex, int endIndex, cmp_t *compare = ( cmp_t * )&ListSortCompare<type> );
	/// Swaps the contents of two lists
	void					Swap( List<type> &other );
	/// Calls the destructor of all elements in the d_list.  Conditionally frees up memory used by the d_list.
	/// Note that this only works on lists containing pointers to objects and will cause a compiler error if called with non-pointers.
	/// Since the d_list was not responsible for allocating the object, it has no information on whether the object still exists or not,
	/// so care must be taken to ensure that the pointers are still valid when this function is called.
	/// Function will set all pointers in the d_list to NULL.
	void					DeleteContents( bool clear );
};

// // //// // //// // //// //
// List
// //// // //// // //// //

template< class type >
INLINE List< type >::List( int newgranularity ) {
	if( newgranularity <= 0 )
		SetGranularity( );
	d_list		= NULL;
	d_granularity	= newgranularity;
	Clear( );
}

template< class type >
INLINE List< type >::List( const List< type > & other ) {
	d_list = NULL;
	*this = other;
}

template< class type >
INLINE List< type >::~List( ) {
	Clear( );
}

template< class type >
INLINE void List< type >::Clear( ) {
	if ( d_list )
		delete[ ] d_list;
	d_list	= NULL;
	d_num		= 0;
	d_size	= 0;
}

template< class type >
INLINE void List< type >::DeleteContents( bool clear ) {
	for( int i = 0; i < d_num; i++ ) {
		delete d_list[ i ];
		d_list[ i ] = NULL;
	}
	if( clear )
		Clear( );
	else
		Common::Com_Memset( d_list, 0, d_size * sizeof( type ) );
}

template< class type >
INLINE size_t List< type >::Allocated( ) const {
	return d_size * sizeof( type );
}

template< class type >
INLINE size_t List< type >::Size( ) const {
	return sizeof( List< type > ) + Allocated( );
}

template< class type >
INLINE size_t List< type >::MemoryUsed( ) const {
	return d_num * sizeof( *d_list );
}

template< class type >
INLINE int List< type >::Num( ) const {
	return d_num;
}

template< class type >
INLINE int List< type >::NumAllocated( ) const {
	return d_size;
}

template< class type >
INLINE void List< type >::SetNum( int newnum, bool resize ) {
	assert( newnum >= 0 );
	if ( resize || newnum > d_size )
		Resize( newnum );
	d_num = newnum;
}

template< class type >
INLINE void List< type >::SetGranularity( int newgranularity ) {
	int newsize;
	assert( newgranularity > 0 );
	d_granularity = newgranularity;
	if( d_list ) {
		// resize it to the closest level of d_granularity
		newsize = d_num + d_granularity - 1;
		newsize -= newsize % d_granularity;
		if( newsize != d_size )
			Resize( newsize );
	}
}

template< class type >
INLINE int List< type >::GetGranularity( ) const {
	return d_granularity;
}

template< class type >
INLINE void List< type >::Condense( ) {
	if( d_list ) {
		if( d_num )
			Resize( d_num );
		else
			Clear( );
	}
}

template< class type >
INLINE void List< type >::Resize( int newsize ) {
	assert( newsize >= 0 );
	if( newsize <= 0 ) { // free up the d_list if no data is being reserved
		Clear( );
		return;
	}
	if( newsize == d_size )
		return; // not changing the d_size, so just exit

	type * temp	= d_list;
	d_size	= newsize;
	if( d_size < d_num )
		d_num = d_size;
	d_list = new type[ d_size ]; // copy the old d_list into our new one
	for( int i = 0; i < d_num; i++ )
		d_list[ i ] = temp[ i ];
	if( temp )
		delete[ ] temp; // delete the old d_list if it exists
}

template< class type >
INLINE void List< type >::Resize( int newsize, int newgranularity ) {
	assert( newsize >= 0 );
	assert( newgranularity > 0 );
	d_granularity = newgranularity;
	if( newsize <= 0 ) { // free up the d_list if no data is being reserved
		Clear( );
		return;
	}
	type * temp	= d_list;
	d_size	= newsize;
	if( d_size < d_num )
		d_num = d_size;
	d_list = new type[ d_size ]; // copy the old d_list into our new one
	for( int i = 0; i < d_num; i++ )
		d_list[ i ] = temp[ i ];
	if ( temp )
		delete[ ] temp; // delete the old d_list if it exists
}

template< class type >
INLINE void List< type >::AssureSize( int newSize ) {
	int newNum = newSize;
	if( newSize > d_size ) {
		if( !d_granularity )
			SetGranularity( );
		newSize += d_granularity - 1;
		newSize -= newSize % d_granularity;
		Resize( newSize );
	}
	d_num = newNum;
}

template< class type >
INLINE void List< type >::AssureSize( int newSize, const type & initValue ) {
	int newNum = newSize;
	if ( newSize > d_size ) {
		if( !d_granularity )
			SetGranularity( );
		newSize += d_granularity - 1;
		newSize -= newSize % d_granularity;
		d_num = d_size;
		Resize( newSize );
		for ( int i = d_num; i < newSize; i++ )
			d_list[ i ] = initValue;
	}
	d_num = newNum;
}

template< class type >
INLINE void List< type >::AssureSizeAlloc( int newSize, new_t *allocator ) {
	int newNum = newSize;
	if ( newSize > d_size ) {
		if( !d_granularity )
			SetGranularity( );
		newSize += d_granularity - 1;
		newSize -= newSize % d_granularity;
		d_num = d_size;
		Resize( newSize );
		for( int i = d_num; i < newSize; i++ )
			d_list[ i ] = ( *allocator)( );
	}
	d_num = newNum;
}

template< class type >
INLINE List< type > & List< type >::operator =( const List< type > & other ) {
	bool sameSize = true;
	if( d_size != other.d_size ) {		
		Clear( );
		d_size		= other.d_size;
		sameSize	= false;
	}
	d_num			= other.d_num;
	d_granularity	= other.d_granularity;
	if ( d_size ) {
		if( !sameSize )
			d_list = new type[ d_size ];
		for( int i = 0; i < d_num; i++ )
			d_list[ i ] = other.d_list[ i ];
	}
	return *this;
}

template< class type >
INLINE const type & List< type >::operator[ ]( int index ) const {
	assert( index >= 0 );
	assert( index < d_num );
	return d_list[ index ];
}

template< class type >
INLINE type & List< type >::operator[ ]( int index ) {
	assert( index >= 0 );
	assert( index < d_num );
	return d_list[ index ];
}

template< class type >
INLINE type * List< type >::Ptr( ) {
	return d_list;
}

template< class type >
const INLINE type * List< type >::Ptr( ) const {
	return d_list;
}

template< class type >
INLINE type & List< type >::Alloc( ) {
	if( !d_list )
		Resize( d_granularity );
	if( d_num == d_size )
		Resize( d_size + d_granularity );
	return d_list[ d_num++ ];
}

template< class type >
INLINE int List< type >::Append( type const & obj ) {
	if( !d_list )
		Resize( d_granularity );
	if( d_num == d_size ) {
		if( !d_granularity )
			SetGranularity( );
		int newsize = d_size + d_granularity;
		Resize( newsize - newsize % d_granularity );
	}
	d_list[ d_num ] = obj;
	d_num++;
	return d_num - 1;
}

template< class type >
INLINE int List< type >::Insert( type const & obj, int index ) {
	if( !d_list )
		Resize( d_granularity );
	if( d_num == d_size ) {
		if( !d_granularity )
			SetGranularity( );
		int newsize = d_size + d_granularity;
		Resize( newsize - newsize % d_granularity );
	}
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
INLINE int List< type >::Append( const List< type > & other ) {
	if( !d_list ) {
		if( !d_granularity )
			SetGranularity( );
		Resize( d_granularity );
	}
	int n = other.Num( );
	for( int i = 0; i < n; i++ )
		Append( other[ i ] );
	return Num( );
}

template< class type >
INLINE int List< type >::AddUnique( type const & obj ) {
	int index;
	index = FindIndex( obj );
	if( index < 0 )
		index = Append( obj );
	return index;
}

template< class type >
INLINE int List< type >::FindIndex( type const & obj ) const {
	for( int i = 0; i < d_num; i++ ) {
		if( d_list[ i ] == obj )
			return i;
	}
	// Not found
	return -1;
}

template< class type >
INLINE type *List< type >::Find( type const & obj ) const {
	int i  = FindIndex( obj );
	if ( i >= 0 )
		return &d_list[ i ];
	return NULL;
}

template< class type >
INLINE type & List< type >::FindSmallest( ) {
	type * ptr = d_list;
	for( int i = 1; i < d_num; i++ ) {
		if( d_list[ i ] < ( *ptr ) )
			ptr = &d_list[ i ];
	}
	return *ptr;
}

template< class type >
INLINE int List< type >::FindNull( ) const {
	for( int i = 0; i < d_num; i++ ) {
		if( d_list[ i ] == NULL )
			return i;
	}
	// Not found
	return -1;
}

template< class type >
INLINE int List< type >::IndexOf( type const * objptr ) const {
	int index = objptr - d_list;
	assert( index >= 0 );
	assert( index < d_num );
	return index;
}

template< class type >
INLINE bool List< type >::DeleteIndex( int index ) {
	assert( d_list != NULL );
	assert( index >= 0 );
	assert( index < d_num );
	if( ( index < 0 ) || ( index >= d_num ) )
		return false;
	d_num--;
	delete d_list[ index ];
	for( int i = index; i < d_num; i++ )
		d_list[ i ] = d_list[ i + 1 ];
	return true;
}

template< class type >
INLINE bool List< type >::RemoveIndex( int index ) {
	assert( d_list != NULL );
	assert( index >= 0 );
	assert( index < d_num );
	if( ( index < 0 ) || ( index >= d_num ) )
		return false;
	d_num--;
	for( int i = index; i < d_num; i++ )
		d_list[ i ] = d_list[ i + 1 ];
	return true;
}

template< class type >
INLINE bool List< type >::RemoveIndexFast( int index ) {
	assert( d_list != NULL );
	assert( index >= 0 );
	assert( index < d_num );
	if( ( index < 0 ) || ( index >= d_num ) )
		return false;
	d_num--;
	// nothing to do
	if( index == d_num )
		return true;
	d_list[ index ] = d_list[ d_num ];	
	return true;
}

template< class type >
INLINE bool List< type >::Remove( type const & obj ) {
	int index = FindIndex( obj );
	if( index >= 0 )
		return RemoveIndex( index );	
	return false;
}

template< class type >
INLINE bool List< type >::RemoveFast( type const & obj ) {
	int index = FindIndex( obj );
	if( index >= 0 )
		return RemoveIndexFast( index );	
	return false;
}

template< class type >
INLINE void List< type >::Sort( cmp_t * compare ) {
	if ( !d_list )
		return;
	typedef int cmp_c( const void *, const void * );
	cmp_c * vCompare = ( cmp_c * )compare;
	qsort( ( void * )d_list, ( size_t )d_num, sizeof( type ), vCompare );
}

template< class type >
INLINE void List< type >::SortSubSection( int startIndex, int endIndex, cmp_t * compare ) {
	if ( !d_list )
		return;
	if ( startIndex < 0 )
		startIndex = 0;
	if ( endIndex >= d_num )
		endIndex = d_num - 1;
	if ( startIndex >= endIndex )
		return;
	typedef int cmp_c( const void * , const void * );
	cmp_c * vCompare = ( cmp_c * )compare;
	qsort( ( void * )( &d_list[startIndex] ), ( size_t )( endIndex - startIndex + 1 ), sizeof( type ), vCompare );
}

template< class type >
INLINE void List< type >::Swap( List< type > & other ) {
	::Swap( d_num, other.d_num );
	::Swap( d_size, other.d_size );
	::Swap( d_granularity, other.d_granularity );
	::Swap( d_list, other.d_list );
}

#endif
