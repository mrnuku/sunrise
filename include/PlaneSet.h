class PlaneReference : public Plane {

	friend class			PlaneSet;

private:

	int						referenceCount;
	int						paralellSetIndex;
	bool					isNegative;

public:

							PlaneReference( const Plane & plane, bool initial_ref );

	void					operator ++( int ) { referenceCount++; }
	operator				int( ) const { return referenceCount; }
	int						ReferenceCount( ) const { return referenceCount; }
	int						ParalellSetIndex( ) const { return paralellSetIndex; }
	bool					Negative( ) const { return isNegative; }

	static int				PlaneDistCmp( PlaneReference * const * a, PlaneReference * const * b );
	static int				ParalellSetIndexCmp( PlaneReference * const * a, PlaneReference * const * b );
};

INLINE PlaneReference::PlaneReference( const Plane & plane, bool initial_ref ) {

	this->Plane::operator=( plane );
	isNegative = initial_ref;
	referenceCount = initial_ref ? 1 : 0;
	paralellSetIndex = 0;
}

class PlaneSet : public List< PlaneReference * > {

private:

	float					planeSetNormalEpsilon;
	float					planeSetDistEpsilon;
	HashIndex				hash;
	List< PlaneSet >		paralells;

	void					SetAllParalellSetIndex( int index );
	static int				ParalellNumCmp( const PlaneSet * a, const PlaneSet * b );

public:

							PlaneSet( const float normal_epsilon = ON_EPSILON, const float dist_epsilon = 0.125f );

	void					Clear( void ) { List< PlaneReference * >::Clear( ); hash.Free( ); }
	void					DeleteContents( ) { List< PlaneReference * >::DeleteContents( true ); hash.Free( ); }

	PlaneReference *		FindPlane( const Plane & plane );
	PlaneReference *		FindPlaneNearest( const Plane & plane );

	void					RegenerateHash( );
	void					DeleteNonReferenced( );

	void					BuildParalellPlaneSets( );
	PlaneSet &				GetParalellSet( PlaneReference * plane );
	int						NumParalellSet( ) const { return paralells.Num( ); }
	PlaneSet &				GetParalellSet( const int index );
	const PlaneSet &		GetParalellSet( const int index ) const { return paralells[ index ]; }

	int						FindPlaneWithParalellIndex( int index ) const;
	int						SumParalellIndexCount( int index ) const;
	int						SumReferenceCount( ) const;

	PlaneReference *		FindFacingPlane( const PlaneSet & other ) const;

	PlaneSet &				operator +=( const PlaneSet & other );

	void					PrintPlanes( );
	void					PrintStatistics( );
};

INLINE PlaneSet::PlaneSet( const float normal_epsilon, const float dist_epsilon ) {

	this->planeSetNormalEpsilon = normal_epsilon;
	this->planeSetDistEpsilon = dist_epsilon;
	assert( this->planeSetDistEpsilon <= 0.125f );
}

INLINE void PlaneSet::RegenerateHash( ) {

	this->hash.Clear( );

	for( int i = 0; i < this->List< PlaneReference * >::Num( ); i++ ) {

		int hashKey = ( int )( fabs( ( *this )[ i ]->Dist( ) ) * 0.125f );
		this->hash.Add( hashKey, i );
	}
}

INLINE PlaneReference * PlaneSet::FindPlane( const Plane & plane ) {

	int hashKey = ( int )( fabs( plane.Dist( ) ) * 0.125f );

	for( int j = 0, border = 0; j < 3; j++ ) {

		switch( j ) {

			case 1: border = 1; break;
			case 2: border = -1; break;
		}

		for( int i = hash.First( hashKey + border ); i >= 0; i = hash.Next( i ) ) {

			if ( ( *this )[ i ]->Compare( plane, this->planeSetNormalEpsilon, this->planeSetDistEpsilon ) ) {

				( *this )[ i ]->operator++( 0 ); // PlaneReference counter
				return ( *this )[ i ];
			}
		}
	}

	if( plane.Type( ) >= PLANETYPE_NEGX && plane.Type( ) < PLANETYPE_TRUEAXIAL ) {

		Append( new PlaneReference( -plane, false ) );
		hash.Add( hashKey, Num( ) - 1 );
		Append( new PlaneReference( plane, true ) );
		hash.Add( hashKey, Num( ) - 1 );

		return ( *this )[ Num( ) - 1 ];

	} else {

		Append( new PlaneReference( plane, true ) );
		hash.Add( hashKey, Num( ) - 1 );
		Append( new PlaneReference( -plane, false ) );
		hash.Add( hashKey, Num( ) - 1 );

		return ( *this )[ Num( ) - 2 ];
	}
}

INLINE void PlaneSet::DeleteNonReferenced( ) {

	for( int i = this->List< PlaneReference * >::Num( ) - 1; i >= 0 ; i-- ) {

		PlaneReference * ref = ( *this )[ i ];

		if( !( int )*ref ) {

			this->List< PlaneReference * >::RemoveIndex( i );
			delete ref;
		}
	}

	this->RegenerateHash( );
}

INLINE PlaneSet & PlaneSet::GetParalellSet( PlaneReference * plane ) {

	for( int i = 0; i < paralells.Num( ); i++ ) {

		if( paralells[ i ].Find( plane ) ) {

			return paralells[ i ];
		}
	}

	paralells.Append( PlaneSet( this->planeSetNormalEpsilon, this->planeSetDistEpsilon ) );
	paralells[ paralells.Num( ) - 1 ].Append( plane );

	return paralells[  paralells.Num( ) - 1 ];
}

INLINE PlaneSet & PlaneSet::GetParalellSet( const int index ) {

	return paralells[ index ];
}

INLINE void PlaneSet::SetAllParalellSetIndex( int index ) {

	int planeCount = this->List< PlaneReference * >::Num( );
	 
	for( int i = 0; i < planeCount; i++ ) {

		( *this )[ i ]->paralellSetIndex = index;
	}
}

INLINE void PlaneSet::BuildParalellPlaneSets( ) {

	int planeCount = this->List< PlaneReference * >::Num( );
	Vec3 start, dir;

	for( int i = 0; i < planeCount; i++ ) {

		if( ( *this )[ i ]->paralellSetIndex ) continue;

		for( int j = 0; j < planeCount; j++ ) {

			if( i == j ) continue;

			if( !( *this )[ i ]->PlaneIntersection( *( *this )[ j ], start, dir ) ) {

				GetParalellSet( ( *this )[ i ] ).Append( ( *this )[ j ] );
				GetParalellSet( ( *this )[ i ] ).SetAllParalellSetIndex( 1 );
			}
		}
	}

	paralells.Sort( ParalellNumCmp );

	for( int i = 0; i < paralells.Num( ); i++ ) {

		paralells[ i ].Sort( PlaneReference::PlaneDistCmp );
		paralells[ i ].SetAllParalellSetIndex( i );
	}
}

INLINE int PlaneSet::FindPlaneWithParalellIndex( int index ) const {

	int planeCount = this->List< PlaneReference * >::Num( );

	for( int i = 0; i < planeCount; i++ ) {

		if( ( *this )[ i ]->paralellSetIndex == index ) return i;
	}

	return -1;
}

INLINE int PlaneSet::SumParalellIndexCount( int index ) const {

	int planeCount = this->List< PlaneReference * >::Num( );
	int sum = 0;

	for( int i = 0; i < planeCount; i++ ) {

		if( ( *this )[ i ]->paralellSetIndex == index ) sum++;
	}

	return sum;
}

INLINE int PlaneSet::SumReferenceCount( ) const {

	int planeCount = this->List< PlaneReference * >::Num( );
	int sum = 0;

	for( int i = 0; i < planeCount; i++ ) {

		sum += ( *this )[ i ]->referenceCount;
	}

	return sum;
}

INLINE PlaneReference * PlaneSet::FindFacingPlane( const PlaneSet & other ) const {

	for( int i = 0; i < Num( ); i++ ) {

		for( int j = 0; j < other.Num( ); j++ ) {

			if( *( *this )[ i ] == -*other[ j ] ) {

				return ( *this )[ i ];
			}
		}
	}

	return NULL;
}

INLINE PlaneReference * PlaneSet::FindPlaneNearest( const Plane & plane ) {

	Vec3 start, dir;
	int psi = -1;

	for( int i = 0; i < paralells.Num( ); i++ ) {

		if( !plane.PlaneIntersection( *paralells[ i ][ 0 ], start, dir ) ) {

			psi = i;
			break;
		}
	}

	if( psi == -1 ) return NULL;

	PlaneReference * nearest = paralells[ psi ][ 0 ];
	float lastNearestDistDeltaAbs = abs( plane.Dist( ) - nearest->Dist( ) );

	for( int i = 1; i < paralells[ psi ].Num( ); i++ ) {

		float distDeltaAbs = abs( plane.Dist( ) - paralells[ psi ][ i ]->Dist( ) );

		if( distDeltaAbs < lastNearestDistDeltaAbs ) {

			nearest = paralells[ psi ][ i ];
			lastNearestDistDeltaAbs = distDeltaAbs;
		}
	}

	return nearest;
}

INLINE PlaneSet & PlaneSet::operator +=( const PlaneSet & other ) {

	for( int i = 0; i < other.Num( ); i++ ) {

		this->FindPlane( *other[ i ] );
	}

	return *this;
}

INLINE void PlaneSet::PrintPlanes( ) {

	Common::Com_Printf( "PlaneSet planes: %i, sum: %i {\n", Num( ), SumReferenceCount( ) );

	for( int i = 0; i < Num( ); i++ ) {

		Common::Com_Printf( "    Plane( %s ) neg: %i, ref: %i, psi: %i\n", ( *this )[ i ]->ToString( ), ( *this )[ i ]->isNegative, ( *this )[ i ]->referenceCount, ( *this )[ i ]->paralellSetIndex );
	}

	Common::Com_Printf( "}\n" );
}

INLINE void PlaneSet::PrintStatistics( ) {

	int planeCount = this->List< PlaneReference * >::Num( );
	int sumRefCount = 0;
	int nonRefCount = 0;
	int savedPlaneCount = 0;

	for( int i = 0; i < planeCount; i++ ) {

		int refCount = ( int )*( *this )[ i ];

		if( !refCount ) nonRefCount++;
		if( refCount > 1 ) savedPlaneCount += refCount;
		sumRefCount += refCount;
	}

	Common::Com_Printf( "Plane statistics: total: %i, referenced: %i, non-referenced: %i, saved: %i, average: %.1f, normal epsilon: %.3f, dist epsilon: %.3f\n",
		planeCount, sumRefCount, nonRefCount, savedPlaneCount, ( float )sumRefCount / ( float )planeCount, this->planeSetNormalEpsilon, this->planeSetDistEpsilon );
}
