#ifndef HASHINDEX_H
#define HASHINDEX_H

#define DEFAULT_HASH_SIZE			1024
#define DEFAULT_HASH_GRANULARITY	1024

class HashIndex {
private:
	int				d_hashSize;
	int *			d_hash;
	int				d_indexSize;
	int *			d_indexChain;
	int				d_granularity;
	int				d_hashMask;
	int				d_lookupMask;

	void			Init( const int initialHashSize, const int initialIndexSize );
	void			Allocate( const int newHashSize, const int newIndexSize );

public:
					HashIndex( );
					HashIndex( const int initialHashSize, const int initialIndexSize );
					~HashIndex( );
					HashIndex( const HashIndex & other );

	/// returns total size of allocated memory
	size_t			Allocated( ) const;

	/// returns total size of allocated memory including size of hash index type
	size_t			Size( ) const;

	HashIndex &		operator =( const HashIndex & other );

	/// add an index to the hash, assumes the index has not yet been added to the hash
	void			Add( const int key, const int index );
	
	/// remove an index from the hash
	void			Remove( const int key, const int index );

	/// get the first index from the hash, returns -1 if empty hash entry
	int				First( const int key ) const;
	
	/// get the next index from the hash, returns -1 if at the end of the hash chain
	int				Next( const int index ) const;
	
	/// insert an entry into the index and add it to the hash, increasing all indexes >= index
	void			InsertIndex( const int key, const int index );
	
	/// remove an entry from the index and remove it from the hash, decreasing all indexes >= index
	void			RemoveIndex( const int key, const int index );
	
	/// clear the hash
	void			Clear( );
	
	/// clear and resize
	void			Clear( const int newHashSize, const int newIndexSize );
	
	/// free allocated memory
	void			Free( );
	
	/// get size of hash table
	int				GetHashSize( ) const;
	
	/// get size of the index
	int				GetIndexSize( ) const;
	
	/// set granularity
	void			SetGranularity( const int newGranularity );
	
	/// force resizing the index, current hash table stays intact
	void			ResizeIndex( const int newIndexSize );
	
	/// returns number in the range [0-100] representing the spread over the hash table
	int				GetSpread( ) const;
	
	/// returns a key for a string
	int				GenerateKey( const char *string, bool caseSensitive = true ) const;
	
	/// returns a key for two integers
	int				GenerateKey( const int n1, const int n2 ) const;
};

INLINE HashIndex::HashIndex( ) {
	Init( DEFAULT_HASH_SIZE, DEFAULT_HASH_SIZE );
}

INLINE HashIndex::HashIndex( const int initialHashSize, const int initialIndexSize ) {
	Init( initialHashSize, initialIndexSize );
}

INLINE HashIndex::~HashIndex( ) {
	Free( );
}

INLINE size_t HashIndex::Allocated( ) const {
	return d_hashSize * sizeof( int ) + d_indexSize * sizeof( int );
}

INLINE size_t HashIndex::Size( ) const {
	return sizeof( *this ) + Allocated( );
}

INLINE HashIndex & HashIndex::operator =( const HashIndex & other ) {
	d_granularity = other.d_granularity;
	d_hashMask = other.d_hashMask;
	d_lookupMask = other.d_lookupMask;

	if( other.d_lookupMask == 0 ) {
		d_hashSize = other.d_hashSize;
		d_indexSize = other.d_indexSize;
		Free( );
	}
	else {
		if( other.d_hashSize != d_hashSize || !d_hash ) {
			if( d_hash )
				delete[ ] d_hash;
			d_hashSize = other.d_hashSize;
			d_hash = new int[ d_hashSize ];
		}
		if( other.d_indexSize != d_indexSize || !d_indexChain ) {
			if( d_indexChain )
				delete[ ] d_indexChain;
			d_indexSize = other.d_indexSize;
			d_indexChain = new int[ d_indexSize ];
		}
		Common::Com_Memcpy( d_hash, other.d_hash, d_hashSize * sizeof( d_hash[ 0 ] ) );
		Common::Com_Memcpy( d_indexChain, other.d_indexChain, d_indexSize * sizeof( d_indexChain[ 0 ] ) );
	}

	return *this;
}

INLINE HashIndex::HashIndex( const HashIndex & other ) { // FIXME optimize plz

	Init( DEFAULT_HASH_SIZE, DEFAULT_HASH_SIZE );
	*this = other;
}

INLINE void HashIndex::Add( const int key, const int index ) {
	assert( index >= 0 );
	if( !d_hash )
		Allocate( d_hashSize, index >= d_indexSize ? index + 1 : d_indexSize );
	else if( index >= d_indexSize )
		ResizeIndex( index + 1 );
	int h = key & d_hashMask;
	d_indexChain[ index ] = d_hash[ h ];
	d_hash[ h ] = index;
}

INLINE void HashIndex::Remove( const int key, const int index ) {
	int k = key & d_hashMask;
	if( !d_hash )
		return;
	if( d_hash[ k ] == index )
		d_hash[ k ] = d_indexChain[ index ];
	else
		for( int i = d_hash[ k ]; i != -1; i = d_indexChain[ i ] )
			if ( d_indexChain[ i ] == index ) {
				d_indexChain[ i ] = d_indexChain[ index ];
				break;
			}
	d_indexChain[ index ] = -1;
}

INLINE int HashIndex::First( const int key ) const {
	return d_hash ? d_hash[ key & d_hashMask & d_lookupMask ] : -1;
}

INLINE int HashIndex::Next( const int index ) const {
	assert( index >= 0 && index < d_indexSize );
	return d_indexChain ? d_indexChain[ index & d_lookupMask ] : -1;
}

INLINE void HashIndex::InsertIndex( const int key, const int index ) {
	if( d_hash ) {
		int max = index;
		for( int i = 0; i < d_hashSize; i++ )
			if( d_hash[ i ] >= index ) {
				d_hash[ i ]++;
				if( d_hash[ i ] > max )
					max = d_hash[ i ];
			}
		for( int i = 0; i < d_indexSize; i++ )
			if( d_indexChain[ i ] >= index ) {
				d_indexChain[ i ]++;
				if( d_indexChain[i] > max )
					max = d_indexChain[ i ];
			}
		if( max >= d_indexSize )
			ResizeIndex( max + 1 );
		for( int i = max; i > index; i-- )
			d_indexChain[ i ] = d_indexChain[ i - 1 ];
		d_indexChain[ index ] = -1;
	}
	Add( key, index );
}

INLINE void HashIndex::RemoveIndex( const int key, const int index ) {
	Remove( key, index );
	if( d_hash ) {
		int max = index;
		for( int i = 0; i < d_hashSize; i++ )
			if( d_hash[ i ] >= index ) {
				if ( d_hash[ i ] > max )
					max = d_hash[ i ];
				d_hash[ i ]--;
			}
		for( int i = 0; i < d_indexSize; i++ )
			if( d_indexChain[ i ] >= index ) {
				if( d_indexChain[ i ] > max )
					max = d_indexChain[ i ];
				d_indexChain[ i ]--;
			}
		for( int i = index; i < max; i++ )
			d_indexChain[ i ] = d_indexChain[ i + 1 ];
		d_indexChain[ max ] = -1;
	}
}

INLINE void HashIndex::Clear( ) {
	if( d_hash ) // only clear the hash table because clearing the indexChain is not really needed
		Common::Com_Memset( d_hash, 0xFF, d_hashSize * sizeof( d_hash[ 0 ] ) );
}

INLINE void HashIndex::Clear( const int newHashSize, const int newIndexSize ) {
	Free( );
	d_hashSize = newHashSize;
	d_indexSize = newIndexSize;
}

INLINE int HashIndex::GetHashSize( ) const {
	return d_hashSize;
}

INLINE int HashIndex::GetIndexSize( ) const {
	return d_indexSize;
}

INLINE void HashIndex::SetGranularity( const int newGranularity ) {
	assert( newGranularity > 0 );
	d_granularity = newGranularity;
}

INLINE int HashIndex::GenerateKey( const char * string, bool caseSensitive ) const {
	if( caseSensitive )
		return ( Str::Hash( string ) & d_hashMask );
	else
		return ( Str::IHash( string ) & d_hashMask );
}

INLINE int HashIndex::GenerateKey( const int n1, const int n2 ) const {
	return ( ( n1 + n2 ) & d_hashMask );
}

// // //// // //// // //// //
/// HashMap
// //// // //// // //// //

template< class type, size_t index_block_size = 256, size_t hash_base2_size = 8, class t_allocator = DynamicAlloc<type,index_block_size,index_block_size> >
class HashMap {
private:
	size_t			d_hashSize;
	type *			d_hash;
	size_t			d_indexSize;
	type *			d_indexChain;
	type			d_hashMask;
	type			d_lookupMask;
	type			d_invalidMask[ 1 ];
	t_allocator		d_allocator;

	void			Init( );
	void			Allocate( const size_t newHashPow2Size, const size_t newIndexSize );

public:
					HashMap( );
					HashMap( const HashMap & other );
					~HashMap( );

	HashMap &		operator =( const HashMap & other );

	/// returns total size of allocated memory
	size_t			Allocated( ) const	{ NumSizeOf( type, d_hashSize ) + NumSizeOf( type, d_indexSize ); }

	/// returns total size of allocated memory including size of hash index type
	size_t			Size( ) const		{ return sizeof( *this ) + Allocated( ); }

	/// add an index to the hash, assumes the index has not yet been added to the hash
	void			Add( const type & key, const type & index );
	
	/// remove an index from the hash
	void			Remove( const type & key, const type & index );

	/// get the first index from the hash, returns -1 if empty hash entry
	const type &	First( const type & key ) const	{ return d_hash[ key & d_hashMask & d_lookupMask ]; }
	
	/// get the next index from the hash, returns -1 if at the end of the hash chain
	const type &	Next( const type & index ) const	{ assert( index >= 0 && index < d_indexSize ); return d_indexChain[ index & d_lookupMask ]; }
	
	/// insert an entry into the index and add it to the hash, increasing all indexes >= index
	void			InsertIndex( const type & key, const type & index );
	
	/// remove an entry from the index and remove it from the hash, decreasing all indexes >= index
	void			RemoveIndex( const type & key, const type & index );
	
	/// clear the hash
	void			Clear( )	{ if( d_hash != d_invalidMask ) Common::Com_Memset( d_hash, 0xFF, NumSizeOf( type, d_hashSize ) ); }
	
	/// free allocated memory
	void			Free( );
	
	/// get size of hash table
	size_t			GetHashSize( ) const	{ return d_hashSize; }
	size_t			GetHashPow2Size( ) const{ return InvPow2( d_hashSize ); }
	
	/// get size of the index
	size_t			GetIndexSize( ) const	{ return d_indexSize; }
	
	/// force resizing the index, current hash table stays intact
	void			ResizeIndex( const size_t newIndexSize );

	/// force resizing the index and hash table
	void			ResizeHash( const size_t new_pow2_size );
	
	/// returns number in the range [0-100] representing the spread over the hash table
	size_t			GetSpread( ) const;
	
	/// returns a key for a string
	type			GenerateKey( const char * string, bool caseSensitive = true ) const	{ return ( type )( caseSensitive ? Str::Hash( string ) : Str::IHash( string ) ) & d_hashMask; }
	
	/// returns a key for two integers
	type			GenerateKey( const type & n1, const type & n2 ) const	{ return ( ( n1 + n2 ) & d_hashMask ); }
};

// // //// // //// // //// //
// HashMap
// //// // //// // //// //

template< class type, size_t index_block_size, size_t hash_base2_size, class t_allocator >
HashMap< type, index_block_size, hash_base2_size, t_allocator >::HashMap( ) {
	Init( );
}

template< class type, size_t index_block_size, size_t hash_base2_size, class t_allocator >
HashMap< type, index_block_size, hash_base2_size, t_allocator >::HashMap( const HashMap & other ) { // FIXME optimize plz
	Init( );
	*this = other;
}

template< class type, size_t index_block_size, size_t hash_base2_size, class t_allocator >
HashMap< type, index_block_size, hash_base2_size, t_allocator >::~HashMap( ) {
	Free( );
}

template< class type, size_t index_block_size, size_t hash_base2_size, class t_allocator >
HashMap< type, index_block_size, hash_base2_size, t_allocator > & HashMap< type, index_block_size, hash_base2_size, t_allocator >::operator =( const HashMap & other ) {
	d_hashMask = other.d_hashMask;
	d_lookupMask = other.d_lookupMask;
	if( other.d_lookupMask == 0 ) {
		d_hashSize = other.d_hashSize;
		d_indexSize = other.d_indexSize;
		Free( );
	} else {
		if( other.d_hashSize != d_hashSize || d_hash == d_invalidMask ) {
			if( d_hash != d_invalidMask )
				d_allocator.Free( d_hash );
			d_hashSize = other.d_hashSize;
			d_hash = t_allocator.Alloc( ( int )d_hashSize );
		}
		if( other.d_indexSize != d_indexSize || d_indexChain == d_invalidMask ) {
			if( d_indexChain != d_invalidMask )
				d_allocator.Free( d_indexChain );
			d_indexSize = other.d_indexSize;
			d_indexChain = d_allocator.Alloc( ( int )d_indexSize );
		}
		Common::Com_Memcpy( d_hash, other.d_hash, NumSizeOf( type, d_hashSize ) );
		Common::Com_Memcpy( d_indexChain, other.d_indexChain, NumSizeOf( type, d_indexSize ) );
	}
	return *this;
}

template< class type, size_t index_block_size, size_t hash_base2_size, class t_allocator >
void HashMap< type, index_block_size, hash_base2_size, t_allocator >::Add( const type & key, const type & index ) {
	assert( index >= 0 );
	if( d_hash == d_invalidMask )
		Allocate( hash_base2_size, ( size_t )index + 1 );
	else if( ( size_t )index >= d_indexSize )
		ResizeIndex( ( size_t )index + 1 );
	type h = key & d_hashMask;
	d_indexChain[ index ] = d_hash[ h ];
	d_hash[ h ] = index;
}

template< class type, size_t index_block_size, size_t hash_base2_size, class t_allocator >
void HashMap< type, index_block_size, hash_base2_size, t_allocator >::Remove( const type & key, const type & index ) {
	type k = key & d_hashMask;
	if( d_hash == d_invalidMask )
		return;
	if( d_hash[ k ] == index )
		d_hash[ k ] = d_indexChain[ index ];
	else
		for( type i = d_hash[ k ]; i != -1; i = d_indexChain[ i ] )
			if( d_indexChain[ i ] == index ) {
				d_indexChain[ i ] = d_indexChain[ index ];
				break;
			}
	d_indexChain[ index ] = d_invalidMask[ 0 ];
}

template< class type, size_t index_block_size, size_t hash_base2_size, class t_allocator >
void HashMap< type, index_block_size, hash_base2_size, t_allocator >::InsertIndex( const type & key, const type & index ) {
	if( d_hash != d_invalidMask ) {
		type max = index;
		for( size_t i = 0; i < d_hashSize; i++ )
			if( d_hash[ i ] >= index ) {
				d_hash[ i ]++;
				if( d_hash[ i ] > max )
					max = d_hash[ i ];
			}
		for( size_t i = 0; i < d_indexSize; i++ )
			if( d_indexChain[ i ] >= index ) {
				d_indexChain[ i ]++;
				if( d_indexChain[ i ] > max )
					max = d_indexChain[ i ];
			}
		if( ( size_t )max >= d_indexSize )
			ResizeIndex( max + 1 );
		for( type i = max; i > index; i-- )
			d_indexChain[ i ] = d_indexChain[ i - 1 ];
		d_indexChain[ index ] = d_invalidMask[ 0 ];
	}
	Add( key, index );
}

template< class type, size_t index_block_size, size_t hash_base2_size, class t_allocator >
void HashMap< type, index_block_size, hash_base2_size, t_allocator >::RemoveIndex( const type & key, const type & index ) {
	Remove( key, index );
	if( d_hash != d_invalidMask ) {
		type max = index;
		for( size_t i = 0; i < d_hashSize; i++ )
			if( d_hash[ i ] >= index ) {
				if ( d_hash[ i ] > max )
					max = d_hash[ i ];
				d_hash[ i ]--;
			}
		for( size_t i = 0; i < d_indexSize; i++ )
			if( d_indexChain[ i ] >= index ) {
				if( d_indexChain[ i ] > max )
					max = d_indexChain[ i ];
				d_indexChain[ i ]--;
			}
		for( type i = index; i < max; i++ )
			d_indexChain[ i ] = d_indexChain[ i + 1 ];
		d_indexChain[ max ] = d_invalidMask[ 0 ];
	}
}

template< class type, size_t index_block_size, size_t hash_base2_size, class t_allocator >
void HashMap< type, index_block_size, hash_base2_size, t_allocator >::Init( ) {
	d_invalidMask[ 0 ]	= -1;
	d_hashSize			= 0;
	d_hash				= d_invalidMask;
	d_indexSize			= 0;
	d_indexChain		= d_invalidMask;
	d_hashMask			= 0;
	d_lookupMask		= 0;
}

template< class type, size_t index_block_size, size_t hash_base2_size, class t_allocator >
void HashMap< type, index_block_size, hash_base2_size, t_allocator >::Allocate( const size_t newHashPow2Size, const size_t newIndexSize ) {
	Free( );
	d_hashSize		= Pow2( newHashPow2Size );
	d_hash			= d_allocator.Alloc( ( int )d_hashSize );
	Common::Com_Memset( d_hash, 0xFF, NumSizeOf( type, d_hashSize ) );
	d_indexSize		= Granulate( index_block_size, newIndexSize );
	d_indexChain	= d_allocator.Alloc( ( int )d_indexSize );
	Common::Com_Memset( d_indexChain, 0xFF, NumSizeOf( type, d_indexSize ) );
	d_hashMask		= ( type )d_hashSize - 1;
	d_lookupMask	= d_invalidMask[ 0 ];
}

template< class type, size_t index_block_size, size_t hash_base2_size, class t_allocator >
void HashMap< type, index_block_size, hash_base2_size, t_allocator >::Free( ) {
	if( d_hash != d_invalidMask )
		d_allocator.Free( d_hash );
	if( d_indexChain != d_invalidMask )
		d_allocator.Free( d_indexChain );
	Init( );
}

template< class type, size_t index_block_size, size_t hash_base2_size, class t_allocator >
void HashMap< type, index_block_size, hash_base2_size, t_allocator >::ResizeIndex( const size_t newIndexSize ) {
	size_t newSize = Granulate( index_block_size, newIndexSize );
	type * oldIndexChain = d_indexChain;
	d_indexChain = d_allocator.Alloc( ( int )newSize );
	if( oldIndexChain != d_invalidMask ) {
		Common::Com_Memcpy( d_indexChain, oldIndexChain, NumSizeOf( type, d_indexSize ) );
		Common::Com_Memset( d_indexChain + d_indexSize, 0xFF, NumSizeOf( type, newSize - d_indexSize ) );
		d_allocator.Free( oldIndexChain );
	} else
		Common::Com_Memset( d_indexChain, 0xFF, NumSizeOf( type, newSize ) );
	d_indexSize = newSize;
}

template< class type, size_t index_block_size, size_t hash_base2_size, class t_allocator >
void HashMap< type, index_block_size, hash_base2_size, t_allocator >::ResizeHash( const size_t new_pow2_size ) {
	size_t newSize = Pow2( new_pow2_size );
	type * oldHashTable = d_hash;
	d_hash = d_allocator.Alloc( ( int )newSize );
	if( oldHashTable != d_invalidMask ) {
		Common::Com_Memcpy( d_hash, oldHashTable, NumSizeOf( type, d_hashSize ) );
		Common::Com_Memset( d_hash + d_hashSize, 0xFF, NumSizeOf( type, newSize - d_hashSize ) );
		d_allocator.Free( oldHashTable );
	} else
		Common::Com_Memset( d_hash, 0xFF, NumSizeOf( type, d_hashSize ) );
	d_hashSize = newSize;
	d_hashMask = ( size_t )d_hashSize - 1;
}

template< class type, size_t index_block_size, size_t hash_base2_size, class t_allocator >
size_t HashMap< type, index_block_size, hash_base2_size, t_allocator >::GetSpread( ) const {
	if( d_hash == d_invalidMask )
		return 100;
	size_t totalItems = 0;
	size_t * numHashItems = new size_t[ d_hashSize ];
	for( size_t i = 0; i < d_hashSize; i++ ) {
		numHashItems[ i ] = 0;
		for( type index = d_hash[ i ]; index != -1; index = d_indexChain[ index ] )
			numHashItems[ i ]++;
		totalItems += numHashItems[ i ];
	}
	if( totalItems <= 1 ) { // if no items in hash
		delete[ ] numHashItems;
		return 100;
	}
	size_t average = totalItems / d_hashSize;
	size_t error = 0;
	for( size_t i = 0; i < d_hashSize; i++ ) {
		size_t e = abs( numHashItems[ i ] - average );
		if( e > 1 )
			error += e - 1;
	}
	delete[ ] numHashItems;
	return 100 - ( error * 100 / totalItems );
}

// // //// // //// // //// //
/// KeyMap
// //// // //// // //// //

template< class type, size_t block_size > class KeyMapNode : public LLNode< KeyMapNode< type, block_size > > {
public:
	int									d_blockBase;
	type								d_block[ block_size ];

										KeyMapNode( ) { d_blockBase = 0; Check( this ); };
};

template< class type, size_t block_size = 256, class block_allocator = DynamicBlockAlloc<KeyMapNode<type,block_size>,32,32> >
class KeyMap {
private:
	typedef KeyMapNode<type,block_size>	t_KeyNode;
	block_allocator						d_nodeAllocator;
	t_KeyNode *							d_root;
	int									d_keyMask;
	int									d_blockMask;

	int									d_startIndex;
	int									d_endIndex;

	type *								FindBase( const int base );
	type *								AllocBase( const int base );

public:

										KeyMap( );
										//~KeyMap( );

	void								Clear( );

	/// Access operator. Index must be within range or an assert will be issued in debug builds. Release builds do no range checking.
	const type &						operator[ ]( int index ) const;

	/// Access operator. Index must be within range or an assert will be issued in debug builds. Release builds do no range checking.
	type &								operator[ ]( int index );

	int									Start( ) const	{ return d_startIndex; }
	int									End( ) const	{ return d_endIndex; }
};

template< class type, size_t block_size, class block_allocator >
KeyMap< type, block_size, block_allocator >::KeyMap( ) {
	assert( IsPow2( block_size ) );
	d_keyMask = ( int )block_size - 1;
	d_blockMask = ~d_keyMask;
	d_root = NULL;
	d_startIndex = INT_MAX;
	d_endIndex = INT_MIN;
}

template< class type, size_t block_size, class block_allocator >
void KeyMap< type, block_size, block_allocator >::Clear( ) {
	d_nodeAllocator.Shutdown( );
	d_nodeAllocator.Init( );
	d_keyMask = ( int )block_size - 1;
	d_blockMask = ~d_keyMask;
	d_root = NULL;
	d_startIndex = INT_MAX;
	d_endIndex = INT_MIN;
}

template< class type, size_t block_size, class block_allocator >
INLINE type * KeyMap< type, block_size, block_allocator >::FindBase( const int base ) {
	for( t_KeyNode * node = d_root; node; node = node->Next( ) )
		if( node->d_blockBase == base )
			return node->d_block;
	return AllocBase( base );
}

template< class type, size_t block_size, class block_allocator >
type * KeyMap< type, block_size, block_allocator >::AllocBase( const int base ) {
	t_KeyNode * node = d_nodeAllocator.Construct( 1 );
	node->d_blockBase = base;
	if( d_root )
		node->AddToEnd( d_root );
	else
		d_root = node;
	return node->d_block;
}

template< class type, size_t block_size, class block_allocator >
INLINE const type & KeyMap< type, block_size, block_allocator >::operator[ ]( int index ) const {
	d_startIndex = MinRef( d_startIndex, index );
	d_endIndex = MaxRef( d_endIndex, index );
	return FindBase( index & d_blockMask )[ index & d_keyMask ];
}

template< class type, size_t block_size, class block_allocator >
INLINE type & KeyMap< type, block_size, block_allocator >::operator[ ]( int index ) {
	d_startIndex = MinRef( d_startIndex, index );
	d_endIndex = MaxRef( d_endIndex, index );
	return FindBase( index & d_blockMask )[ index & d_keyMask ];
}

#endif
