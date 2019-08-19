// Copyright ( C) 2004 Id Software, Inc.
//

#ifndef __STRPOOL_H__
#define __STRPOOL_H__

/*
===============================================================================

	StrPool

===============================================================================
*/

class StrPool;

class PoolStr : public Str {
	friend class StrPool;

public:
						PoolStr( ) { numUsers = 0; }
						~PoolStr( ) { assert( numUsers == 0 ); }

						// returns total size of allocated memory
	size_t				Allocated( ) const { return Str::Allocated( ); }
						// returns total size of allocated memory including size of string pool type
	size_t				Size( ) const { return sizeof( *this ) + Allocated( ); }
						// returns a pointer to the pool this string was allocated from
	const StrPool *	GetPool( ) const { return pool; }

private:
	StrPool *			pool;
	mutable int			numUsers;
};

class StrPool {
public:
						StrPool( ) { caseSensitive = true; }

	void				SetCaseSensitive( bool caseSensitive );

	int					Num( ) const { return pool.Num( ); }
	size_t				Allocated( ) const;
	size_t				Size( ) const;

	const PoolStr *	operator[]( int index ) const { return pool[index]; }

	const PoolStr *	AllocString( const char *string );
	void				FreeString( const PoolStr *poolStr );
	const PoolStr *	CopyString( const PoolStr *poolStr );
	void				Clear( );

private:
	bool				caseSensitive;
	List<PoolStr *>	pool;
	HashIndex			poolHash;
};

/*
================
StrPool::SetCaseSensitive
================
*/
INLINE void StrPool::SetCaseSensitive( bool caseSensitive ) {
	this->caseSensitive = caseSensitive;
}

/*
================
StrPool::AllocString
================
*/
INLINE const PoolStr *StrPool::AllocString( const char *string ) {
	int i, hash;
	PoolStr *poolStr;

	hash = poolHash.GenerateKey( string, caseSensitive );
	if ( caseSensitive ) {
		for ( i = poolHash.First( hash ); i != -1; i = poolHash.Next( i ) ) {
			if ( pool[i]->Cmp( string ) == 0 ) {
				pool[i]->numUsers++;
				return pool[i];
			}
		}
	} else {
		for ( i = poolHash.First( hash ); i != -1; i = poolHash.Next( i ) ) {
			if ( pool[i]->Icmp( string ) == 0 ) {
				pool[i]->numUsers++;
				return pool[i];
			}
		}
	}

	poolStr = new PoolStr;
	*static_cast<Str *>( poolStr) = string;
	poolStr->pool = this;
	poolStr->numUsers = 1;
	poolHash.Add( hash, pool.Append( poolStr ) );
	return poolStr;
}

/*
================
StrPool::FreeString
================
*/
INLINE void StrPool::FreeString( const PoolStr *poolStr ) {
	int i, hash;

	assert( poolStr->numUsers >= 1 );
	assert( poolStr->pool == this );

	poolStr->numUsers--;
	if ( poolStr->numUsers <= 0 ) {
		hash = poolHash.GenerateKey( poolStr->c_str( ), caseSensitive );
		if ( caseSensitive ) { 
			for ( i = poolHash.First( hash ); i != -1; i = poolHash.Next( i ) ) {
				if ( pool[i]->Cmp( poolStr->c_str( ) ) == 0 ) {
					break;
				}
			}
		} else {
			for ( i = poolHash.First( hash ); i != -1; i = poolHash.Next( i ) ) {
				if ( pool[i]->Icmp( poolStr->c_str( ) ) == 0 ) {
					break;
				}
			}
		}
		assert( i != -1 );
		assert( pool[i] == poolStr );
		delete pool[i];
		pool.RemoveIndex( i );
		poolHash.RemoveIndex( hash, i );
	}
}

/*
================
StrPool::CopyString
================
*/
INLINE const PoolStr *StrPool::CopyString( const PoolStr *poolStr ) {

	assert( poolStr->numUsers >= 1 );

	if ( poolStr->pool == this ) {
		// the string is from this pool so just increase the user count
		poolStr->numUsers++;
		return poolStr;
	} else {
		// the string is from another pool so it needs to be re-allocated from this pool.
		return AllocString( poolStr->c_str( ) );
	}
}

/*
================
StrPool::Clear
================
*/
INLINE void StrPool::Clear( ) {
	int i;

	for ( i = 0; i < pool.Num( ); i++ ) {
		pool[i]->numUsers = 0;
	}
	pool.DeleteContents( true );
	poolHash.Free( );
}

/*
================
StrPool::Allocated
================
*/
INLINE size_t StrPool::Allocated( ) const {
	int i;
	size_t size;

	size = pool.Allocated( ) + poolHash.Allocated( );
	for ( i = 0; i < pool.Num( ); i++ ) {
		size += pool[i]->Allocated( );
	}
	return size;
}

/*
================
StrPool::Size
================
*/
INLINE size_t StrPool::Size( ) const {
	int i;
	size_t size;

	size = pool.Size( ) + poolHash.Size( );
	for ( i = 0; i < pool.Num( ); i++ ) {
		size += pool[i]->Size( );
	}
	return size;
}

#endif /* !__STRPOOL_H__ */
