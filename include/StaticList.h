#ifndef __STATICLIST_H__
#define __STATICLIST_H__

/// * * * * * * * * * * * * * * *
///
/// StaticList< obj, num >
///
/// A non-growing, Common::Com_Memset-able list using no memory allocation.
///
/// * * * * * * * * * * * * * * *

template< class type, int size > class StaticList {

private:

	int								d_num;
	type 							d_list[ size ];

public:

									StaticList( );
									StaticList( const StaticList< type, size > & other );
									~StaticList< type, size >( );

	/// marks the list as empty.  does not deallocate or intialize data.
	void							Clear( );

	/// returns number of elements in list
	int								Num( ) const;

	/// returns the maximum number of elements in the list
	int								Max( ) const;

	/// set number of elements in list
	void							SetNum( int newnum );

	/// returns total size of allocated memory
	size_t							Allocated( ) const;

	/// returns total size of allocated memory including size of list type
	size_t							Size( ) const;

	/// returns size of the used elements in the list
	size_t							MemoryUsed( ) const;

	/// Copies the contents and size attributes of a dynamic list.
	StaticList< type, size > &		operator =( const List< type > & other );

	/// Access operator. Index must be within range or an assert will be issued in debug builds. Release builds do no range checking.
	const type &					operator[ ]( int index ) const;

	/// Access operator. Index must be within range or an assert will be issued in debug builds. Release builds do no range checking.
	type &							operator[ ]( int index );

	/// returns a pointer to the list
	type *							Ptr( );

	/// returns a pointer to the list
	const type *					Ptr( ) const;

	/// returns reference to a new data element at the end of the list.  returns NULL when full.
	type *							Alloc( );

	/// append element
	int								Append( const type & obj );

	/// append list
	int								Append( const StaticList< type, size > &other );

	/// add unique element
	int								AddUnique( const type & obj );

	/// insert the element at the given index
	int								Insert( const type & obj, int index );

	/// find the index for the given element
	int								FindIndex( const type & obj ) const;

	/// find pointer to the given element
	type *							Find( type const & obj ) const;

	/// find the index for the first NULL pointer in the list
	int								FindNull( ) const;

	/// returns the index for the pointer to an element in the list
	int								IndexOf( const type *obj ) const;

	/// remove the element at the given index
	bool							RemoveIndex( int index );

	/// remove the element
	bool							Remove( const type & obj );

	/// swap the contents of the lists
	void							Swap( StaticList< type, size > &other );

	/// delete the contents of the list
	void							DeleteContents( bool clear );
};

template< class type, int size >
INLINE StaticList< type, size >::StaticList( ) {

	d_num = 0;
}

template< class type, int size >
INLINE StaticList< type, size >::StaticList( const StaticList< type, size > & other ) {

	*this = other;
}

template< class type, int size >
INLINE StaticList< type, size >::~StaticList( ) {
}

template< class type, int size >
INLINE void StaticList< type, size >::Clear( ) {

	d_num	= 0;
}

template< class type, int size >
INLINE void StaticList< type, size >::DeleteContents( bool clear ) {

	for( int i = 0; i < size; i++ ) {

		delete d_list[ i ];
		d_list[ i ] = NULL;
	}

	if( clear ) Clear( );
	else Common::Com_Memset( d_list, 0, sizeof( d_list ) );
}

template< class type, int size >
INLINE int StaticList< type, size >::Num( ) const {

	return d_num;
}

template< class type, int size >
INLINE int StaticList< type, size >::Max( ) const {

	return size;
}

template< class type, int size >
INLINE size_t StaticList< type, size >::Allocated( ) const {

	return size * sizeof( type );
}

template< class type, int size >
INLINE size_t StaticList< type, size >::Size( ) const {

	return sizeof( StaticList< type, size > ) + Allocated( );
}

template< class type, int size >
INLINE size_t StaticList< type, size >::MemoryUsed( ) const {

	return d_num * sizeof( d_list[ 0 ] );
}

template< class type, int size >
INLINE void StaticList< type, size >::SetNum( int newnum ) {

	assert( newnum >= 0 );
	assert( newnum <= size );
	d_num = newnum;
}

template< class type, int size >
INLINE StaticList< type, size > & StaticList< type, size >::operator =( const List< type > & other ) {

	assert( size >= other.Num( ) );

	d_num = other.Num( );
	for( int i = 0; i < d_num; i++ ) {

		d_list[ i ] = other[ i ];
	}

	return *this;
}

template< class type, int size >
INLINE const type &StaticList< type, size >::operator[ ]( int index ) const {

	assert( index >= 0 );
	assert( index < size );

	return d_list[ index ];
}

template< class type, int size >
INLINE type &StaticList< type, size >::operator[ ]( int index ) {

	assert( index >= 0 );
	assert( index < size );

	return d_list[ index ];
}

template< class type, int size >
INLINE type *StaticList< type, size >::Ptr( ) {

	return &d_list[ 0 ];
}

template< class type, int size >
INLINE const type *StaticList< type, size >::Ptr( ) const {

	return &d_list[ 0 ];
}

template< class type, int size >
INLINE type *StaticList< type, size >::Alloc( ) {

	if ( d_num >= size ) return NULL;

	return &d_list[ d_num++ ];
}

template< class type, int size >
INLINE int StaticList< type, size >::Append( type const & obj ) {

	assert( d_num < size );
	if ( d_num < size ) {

		d_list[ d_num ] = obj;
		d_num++;
		return d_num - 1;
	}

	return -1;
}

template< class type, int size >
INLINE int StaticList< type, size >::Insert( type const & obj, int index ) {

	assert( d_num < size );
	if ( d_num >= size ) {

		return -1;
	}

	assert( index >= 0 );
	if ( index < 0 ) index = 0;
	else if ( index > d_num ) index = d_num;

	for( int i = d_num; i > index; --i ) {

		d_list[ i ] = d_list[ i - 1 ];
	}

	d_num++;
	d_list[ index ] = obj;
	return index;
}

template< class type, int size >
INLINE int StaticList< type, size >::Append( const StaticList< type, size > &other ) {

	int n = other.Num( );

	if ( d_num + n > size ) n = size - d_num;

	for( int i = 0; i < n; i++ ) {

		d_list[i + d_num] = other.d_list[i];
	}

	d_num += n;

	return Num( );
}

template< class type, int size >
INLINE int StaticList< type, size >::AddUnique( type const & obj ) {

	int index = FindIndex( obj );

	if ( index < 0 ) index = Append( obj );

	return index;
}

template< class type, int size >
INLINE int StaticList< type, size >::FindIndex( type const & obj ) const {

	for( int i = 0; i < d_num; i++ ) {

		if ( d_list[ i ] == obj ) return i;
	}

	return -1; // Not found
}

template< class type, int size >
INLINE type *StaticList< type, size >::Find( type const & obj ) const {

	int i = FindIndex( obj );

	if ( i >= 0 ) return &d_list[ i ];

	return NULL;
}

template< class type, int size >
INLINE int StaticList< type, size >::FindNull( ) const {

	for( int i = 0; i < d_num; i++ ) {

		if ( d_list[ i ] == NULL ) return i;
	}

	return -1; // Not found
}

template< class type, int size >
INLINE int StaticList< type, size >::IndexOf( type const *objptr ) const {

	int index = objptr - d_list;

	assert( index >= 0 );
	assert( index < d_num );

	return index;
}

template< class type, int size >
INLINE bool StaticList< type, size >::RemoveIndex( int index ) {

	assert( index >= 0 );
	assert( index < d_num );

	if( ( index < 0 ) || ( index >= d_num ) ) return false;

	d_num--;
	for( int i = index; i < d_num; i++ ) {

		d_list[ i ] = d_list[ i + 1 ];
	}

	return true;
}

template< class type, int size >
INLINE bool StaticList< type, size >::Remove( type const & obj ) {

	int index = FindIndex( obj );

	if ( index >= 0 ) return RemoveIndex( index );
	
	return false;
}

template< class type, int size >
INLINE void StaticList< type, size >::Swap( StaticList< type, size > &other ) {

	StaticList< type, size > temp = *this;
	*this = other;
	other = temp;
}

#endif /* !__STATICLIST_H__ */
