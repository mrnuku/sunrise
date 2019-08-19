// Copyright (C) 2007 Id Software, Inc.
//


#ifndef __VECTORWELD_H__
#define __VECTORWELD_H__

/*
===============================================================================

	Vector Weld

	Welds Vectors

===============================================================================
*/

template< class type, int dimension >
class VectorWeld {

public:

										VectorWeld( );
										VectorWeld( const type & mins, const type & maxs, const int boxHashSize, const int initialSize );

	void								Init( const type & mins, const type & maxs, const int boxHashSize, const int initialSize );
	static int							HashSizeForBounds( const type & mins, const type & maxs, const float epsilon );
	void								SetGranularity( int newGranularity );
	void								Clear( );

	void								AddVector( type * v, const float epsilon );
	int									GetNumUniqueVectors( ) const;
	void								Weld( const float integralEpsilon );

	void								Resize( const type & mins, const type & maxs, const int boxHashSize );
	const type *						FindNearest( const type & vec ) const;

private:

	struct WeldVector;

	struct WeldGroup {

		/// first vector in weld group
		WeldVector *					first;
	};

	struct WeldVector {

		/// pointer to vector
		type *							v;

		/// weld group this vector is in
		WeldGroup *						group;

		/// next vector in the weld group
		WeldVector *					next;
	};

	List< WeldVector * >				vectors;
	List< WeldGroup * >					groups;
	HashIndex							hash;
	type								mins;
	type								maxs;
	int									boxHashSize;
	float								boxInvSize[ dimension ];
	float								boxHalfSize[ dimension ];

	BlockAlloc< WeldGroup, 1024 >		weldGroupAllocator;
	BlockAlloc< WeldVector, 1024 >		weldVectorAllocator;

	void								MergeGroups( WeldGroup * group1, WeldGroup * group2 );
};

template< class type, int dimension >
VectorWeld< type, dimension >::VectorWeld( ) {

	hash.Clear( IPow( boxHashSize, dimension ), 128 );
	boxHashSize = 16;
	Common::Com_Memset( boxInvSize, 0, dimension * sizeof( boxInvSize[ 0 ] ) );
	Common::Com_Memset( boxHalfSize, 0, dimension * sizeof( boxHalfSize[ 0 ] ) );
}

template< class type, int dimension >
VectorWeld< type, dimension >::VectorWeld( const type & mins, const type & maxs, const int boxHashSize, const int initialSize ) {

	Init( mins, maxs, boxHashSize, initialSize );
}

template< class type, int dimension >
void VectorWeld< type, dimension >::Init( const type & mins, const type & maxs, const int boxHashSize, const int initialSize ) {

	vectors.AssureSize( initialSize );
	vectors.SetNum( 0, false );
	vectors.SetGranularity( 1024 );

	groups.AssureSize( initialSize );
	groups.SetNum( 0, false );
	groups.SetGranularity( 1024 );

	hash.Clear( IPow( boxHashSize, dimension ), initialSize );
	hash.SetGranularity( 1024 );

	this->mins = mins;
	this->maxs = maxs;
	this->boxHashSize = boxHashSize;

	for( int i = 0; i < dimension; i++ ) {

		float boxSize = ( maxs[ i ] - mins[ i ] ) / ( float )boxHashSize;
		boxInvSize[ i ] = 1.0f / boxSize;
		boxHalfSize[ i ] = boxSize * 0.5f;
	}
}

template< class type, int dimension >
int VectorWeld< type, dimension >::HashSizeForBounds( const type & mins, const type & maxs, const float epsilon ) {

	float min = INFINITY;
	int i;

	for( i = 0; i < 3; i++ ) {

		if( maxs[ i ] - mins[ i ] < min ) {

			min = maxs[ i ] - mins[ i ];
		}
	}

	min /= epsilon * 4;

	for( i = 0; i < 7; i++ ) {

		if( ( 1 << i ) > min ) {

			break;
		}
	}

	return 1 << i;
}

template< class type, int dimension >
void VectorWeld< type, dimension >::SetGranularity( int newGranularity ) {

	vectors.SetGranularity( newGranularity );
	groups.SetGranularity( newGranularity );
	hash.SetGranularity( newGranularity );
}

template< class type, int dimension >
void VectorWeld< type, dimension >::Clear( ) {

	vectors.Clear( );
	groups.Clear( );
	hash.Clear( );
}

template< class type, int dimension >
void VectorWeld< type, dimension >::AddVector( type * v, const float epsilon ) {

	int i, j, k, hashKey, partialHashKey[ dimension ];

	WeldVector * vector = weldVectorAllocator.Alloc( );
	vector->group = weldGroupAllocator.Alloc( );
	vector->group->first = vector;
	vector->v = v;
	vector->next = NULL;

	vectors.Alloc( ) = vector;
	groups.Alloc( ) = vector->group;

	// create partial hash key for this vector
	for( i = 0; i < dimension; i++ ) {

		assert( epsilon <= boxHalfSize[ i ] );
		partialHashKey[ i ] = ( int )( ( ( *vector->v )[ i ] - mins[ i ] - boxHalfSize[ i ] ) * boxInvSize[ i ] );
	}

	// find nearby vectors and merge their groups into the group of this vector
	for( i = 0; i < ( 1 << dimension ); i++ ) {

		hashKey = 0;
		for( j = 0; j < dimension; j++ ) {

			hashKey *= boxHashSize;
			hashKey += partialHashKey[ j ] + ( ( i >> j ) & 1 );
		}

		for( j = hash.First( hashKey ); j != -1; j = hash.Next( j ) ) {

			// if already in the same group
			if( vectors[ j ]->group == vector->group ) {

				continue;
			}

			// merge groups if the vertices are close enough to each other
			const type & v1 = *vector->v;
			const type & v2 = *vectors[ j ]->v;

			for( k = 0; k < dimension; k++ ) {

				if( fabs( v1[ k ] - v2[ k ] ) > epsilon ) {

					break;
				}
			}

			if ( k >= dimension ) {

				MergeGroups( vector->group, vectors[ j ]->group );
			}
		}
	}

	// add the vector to the hash
	hashKey = 0;
	for( i = 0; i < dimension; i++ ) {

		hashKey *= boxHashSize;
		hashKey += ( int )( ( ( *vector->v )[ i ] - mins[ i ] ) * boxInvSize[ i ] );
	}

	hash.Add( hashKey, vectors.Num( ) - 1 );
}

template< class type, int dimension >
int VectorWeld< type, dimension >::GetNumUniqueVectors( ) const {

	return groups.Num( );
}

template< class type, int dimension >
void VectorWeld< type, dimension >::Weld( const float integralEpsilon ) {

	int i, j;
	WeldVector *vector;
	type mins, maxs, center;

	for( i = 0; i < groups.Num( ); i++ ) {

		if( groups[ i ]->first == NULL ) {

			continue;
		}

		for( j = 0; j < dimension; j++ ) {

			mins[ j ] = INFINITY;
			maxs[ j ] = -INFINITY;
		}

		for( vector = groups[ i ]->first; vector != NULL; vector = vector->next ) {

			const type & v = *vector->v;

			for( j = 0; j < dimension; j++ ) {

				if( v[ j ] < mins[ j ] ) {

					mins[ j ] = v[ j ];
				}

				if( v[ j ] > maxs[ j ] ) {

					maxs[ j ] = v[ j ];
				}
			}
		}

		center = ( mins + maxs ) * 0.5f;

		for( j = 0; j < dimension; j++ ) {

			if( fabs( center[ j ] - Rint( center[ j ] ) ) < integralEpsilon ) {

				center[ j ] = Rint( center[ j ] );
			}
		}

		for( vector = groups[ i ]->first; vector != NULL; vector = vector->next ) {

			*vector->v = center;
		}
	}
}

template< class type, int dimension >
void VectorWeld< type, dimension >::MergeGroups( WeldGroup * group1, WeldGroup * group2 ) {

	WeldVector *v, *next;

	// move all vertices from group2 to group1
	for( v = group2->first; v != NULL; v = next ) {

		next = v->next;
		v->group = group1;
		v->next = group1->first;
		group1->first = v;
	}

	// remove group2
	group2->first = NULL;
	groups.RemoveFast( group2 );
	weldGroupAllocator.Free( group2 );
}

template< class type, int dimension >
void VectorWeld< type, dimension >::Resize( const type & mins, const type & maxs, const int boxHashSize ) {

	this->mins = mins;
	this->maxs = maxs;
	this->boxHashSize = boxHashSize;

	for( int i = 0; i < dimension; i++ ) {

		float boxSize = ( maxs[ i ] - mins[ i ] ) / ( float )boxHashSize;
		boxInvSize[ i ] = 1.0f / boxSize;
		boxHalfSize[ i ] = boxSize * 0.5f;
	}

	hash.Clear( IPow( boxHashSize, dimension ), 1024 );
	weldGroupAllocator.Shutdown( );
	groups.SetNum( 0, false );

	for( int i = 0; i < vectors.Num( ); i++ ) {

		vectors[ i ]->next = NULL;

		int hashKey = 0;
		for( int j = 0; j < dimension; j++ ) {

			hashKey *= boxHashSize;
			hashKey += ( int )( ( ( *vectors[ i ]->v )[ j ] - mins[ j ] ) * boxInvSize[ j ] );
		}

		int vertIndex = hash.First( hashKey );

		if( vertIndex != -1 ) {

			WeldVector * fromHash;
			for( int k = 0; vertIndex != -1; k++, vertIndex = hash.Next( k ) ) {

				fromHash = vectors[ vertIndex ];
			}

			vectors[ i ]->group = fromHash->group;
			fromHash->next = vectors[ i ];

		} else {

			vectors[ i ]->group = weldGroupAllocator.Alloc( );
			vectors[ i ]->group->first = vectors[ i ];
			groups.Append( vectors[ i ]->group );
		}

		hash.Add( hashKey, i );
	}
}

template< class type, int dimension >
const type * VectorWeld< type, dimension >::FindNearest( const type & vec ) const {

	float lenSqr, absSqr = INFINITY;
	type * nearest = NULL;

	int hashKey = 0;
	for( int i = 0; i < dimension; i++ ) {

		hashKey *= boxHashSize;
		hashKey += ( int )( ( vec[ i ] - mins[ i ] ) * boxInvSize[ i ] );
	}

	int vertIndex = hash.First( hashKey );

	if( vertIndex != -1 ) {

		WeldVector * fromHash = vectors[ vertIndex ]->group->first;

		while( fromHash ) {

			lenSqr = fabs( ( vec - *( fromHash->v ) ).LengthSqr( ) );
			if( lenSqr < absSqr ) {

				absSqr = lenSqr;
				nearest = fromHash->v;
			}

			fromHash = fromHash->next;
		}

	} else {

		for( int i = 0; i < vectors.Num( ); i++ ) {

			lenSqr = fabs( ( vec - *( vectors[ i ]->v ) ).LengthSqr( ) );
			if( lenSqr < absSqr ) {

				absSqr = lenSqr;
				nearest = vectors[ i ]->v;
			}
		}

		Common::Com_Printf( "No vector found in hashlist, nearest in: %.2f\n", sqrtf( absSqr ) );
	}

	return nearest;
}

#endif /* !__VECTORWELD_H__ */
