#include "precompiled.h"
#pragma hdrstop

void HashIndex::Init( const int initialHashSize, const int initialIndexSize ) {
	assert( IsPow2( initialHashSize ) );

	d_hashSize = initialHashSize;
	d_hash = NULL;
	d_indexSize = initialIndexSize;
	d_indexChain = NULL;
	d_granularity = DEFAULT_HASH_GRANULARITY;
	d_hashMask = d_hashSize - 1;
	d_lookupMask = 0;
}

void HashIndex::Allocate( const int newHashSize, const int newIndexSize ) {
	assert( IsPow2( newHashSize ) );

	Free( );
	d_hashSize = newHashSize;
	d_hash = new int[ d_hashSize ];
	Common::Com_Memset( d_hash, 0xFF, d_hashSize * sizeof( d_hash[ 0 ] ) );
	d_indexSize = newIndexSize;
	d_indexChain = new int[ d_indexSize ];
	Common::Com_Memset( d_indexChain, 0xFF, d_indexSize * sizeof( d_indexChain[ 0 ] ) );
	d_hashMask = d_hashSize - 1;
	d_lookupMask = -1;
}

void HashIndex::Free( ) {
	if( d_hash ) {
		delete[ ] d_hash;
		d_hash = NULL;
	}
	if( d_indexChain ) {
		delete[ ] d_indexChain;
		d_indexChain = NULL;
	}
	d_lookupMask = 0;
}

void HashIndex::ResizeIndex( const int newIndexSize ) {
	if( newIndexSize <= d_indexSize )
		return;
	int newSize, mod = newIndexSize % d_granularity;
	if( !mod )
		newSize = newIndexSize;
	else
		newSize = newIndexSize + d_granularity - mod;
	if( !d_indexChain ) {
		d_indexSize = newSize;
		return;
	}
	int * oldIndexChain = d_indexChain;
	d_indexChain = new int[ newSize ];
	Common::Com_Memcpy( d_indexChain, oldIndexChain, d_indexSize * sizeof( int ) );
	Common::Com_Memset( d_indexChain + d_indexSize, 0xFF, ( newSize - d_indexSize ) * sizeof( int ) );
	delete[ ] oldIndexChain;
	d_indexSize = newSize;
}

int HashIndex::GetSpread( ) const {
	if( !d_hash )
		return 100;
	int totalItems = 0;
	int * numHashItems = new int[ d_hashSize ];
	for( int i = 0; i < d_hashSize; i++ ) {
		numHashItems[ i ] = 0;
		for( int index = d_hash[ i ]; index >= 0; index = d_indexChain[ index ] )
			numHashItems[ i ]++;
		totalItems += numHashItems[ i ];
	}
	if( totalItems <= 1 ) { // if no items in hash
		delete[ ] numHashItems;
		return 100;
	}
	int average = totalItems / d_hashSize;
	int error = 0;
	for( int i = 0; i < d_hashSize; i++ ) {
		int e = abs( numHashItems[ i ] - average );
		if( e > 1 )
			error += e - 1;
	}
	delete[ ] numHashItems;
	return 100 -( error * 100 / totalItems );
}
