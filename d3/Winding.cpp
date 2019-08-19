#include "precompiled.h"
#pragma hdrstop

bool Winding::ReAllocate( int n, bool keep ) {

	Vec5 *oldP = p;

	n = ( n + 3 ) & ~3;	// align up to multiple of four
	p = new Vec5[ n ];

	if( oldP ) {

		if( keep ) {

			memcpy( p, oldP, numPoints * sizeof( p[ 0 ] ) );
		}

		delete[ ] oldP;
	}

	allocedSize = n;

	return true;
}

void Winding::BaseForPlane( const Vec3 & normal, const float dist, const float radius ) {

	Vec3 org, vright, vup;

	EnsureAlloced( 4 );
	numPoints = 4;

	normal.NormalVectors( vup, vright );
	vup *= radius;
	vright *= radius;
	org = normal * dist;

	p[ 0 ].ToVec3( ) = org - vright + vup;
	p[ 0 ].s = p[ 0 ].t = 0.0f;

	p[ 1 ].ToVec3( ) = org + vright + vup;
	p[ 1 ].s = p[ 1 ].t = 0.0f;

	p[ 2 ].ToVec3( ) = org + vright - vup;
	p[ 2 ].s = p[ 2 ].t = 0.0f;

	p[ 3 ].ToVec3( ) = org - vright - vup;
	p[ 3 ].s = p[ 3 ].t = 0.0f;
}

void CalculateSides( const Vec5 * points, int numPoints, const Plane & plane, const float epsilon, double *dists, byte *sides, int counts[ 3 ] ) {

	int i;

	counts[ SIDE_FRONT ] = counts[ SIDE_BACK ] = counts[ SIDE_ON ] = 0;

	for( i = 0; i < numPoints; i++ ) {

		dists[ i ] = plane.Distance( points[ i ].ToVec3( ) );

		if( dists[ i ] > epsilon ) {

			sides[ i ] = SIDE_FRONT;

		} else if( dists[ i ] < -epsilon ) {

			sides[ i ] = SIDE_BACK;

		} else {

			sides[ i ] = SIDE_ON;
		}

		counts[sides[ i ]]++;
	}

	sides[ i ] = sides[ 0 ];
	dists[ i ] = dists[ 0 ];
}

int Winding::Split( const Plane & plane, const float epsilon, Winding & front, Winding & back ) const {

	int				i, j;
	double *		dists;
	byte *			sides;
	int				counts[ 3 ];
	const Vec5 *	p1, *p2;
	Vec5			mid;
	int				maxpts;

	assert( this != NULL );

	front.numPoints = 0;
	back.numPoints = 0;

	dists = ( double * )_alloca( ( numPoints + 4 ) * sizeof( dists[ 0 ] ) );
	sides = ( byte * )_alloca( ( numPoints + 4 ) * sizeof( sides[ 0 ] ) );

	CalculateSides( p, numPoints, plane, epsilon, dists, sides, counts );

	// if coplanar, put on the front side if the normals match
	if( !counts[ SIDE_FRONT ] && !counts[ SIDE_BACK ] ) {

		Plane windingPlane;

		GetPlane( windingPlane );

		if( windingPlane.Normal( ) * plane.Normal( ) > 0.0f ) {

			front = *this;
			return SIDE_FRONT;

		} else {

			back = *this;
			return SIDE_BACK;
		}
	}

	// if nothing at the front of the clipping plane
	if( !counts[ SIDE_FRONT ] ) {

		back = *this;
		return SIDE_BACK;
	}

	// if nothing at the back of the clipping plane
	if( !counts[ SIDE_BACK ] ) {

		front = *this;
		return SIDE_FRONT;
	}

	maxpts = numPoints + 4;	// cannot use counts[ 0 ]+2 because of fp grouping errors

	front.EnsureAlloced( maxpts );
	back.EnsureAlloced( maxpts );

	for( i = 0; i < numPoints; i++ ) {

		p1 = &p[ i ];

		if( sides[ i ] == SIDE_ON ) {

			front.p[front.numPoints] = *p1;
			front.numPoints++;
			back.p[back.numPoints] = *p1;
			back.numPoints++;
			continue;
		}

		if( sides[ i ] == SIDE_FRONT ) {

			front.p[front.numPoints] = *p1;
			front.numPoints++;

		} else if( sides[ i ] == SIDE_BACK ) {

			back.p[back.numPoints] = *p1;
			back.numPoints++;
		}

		if( sides[ i + 1 ] == SIDE_ON || sides[ i + 1 ] == sides[ i ] ) {

			continue;
		}

		// generate a split point
		p2 = &p[ ( i + 1 ) % numPoints ];

		// always calculate the split going from the same side or minor epsilon issues can happen
		if( sides[ i ] == SIDE_FRONT ) {

			double d = dists[ i ] / ( dists[ i ] - dists[ i + 1 ] );

			for( j = 0; j < 3; j++ ) {

				// avoid round off error when possible
				if( plane.Normal( )[ j ] == 1.0f ) {

					mid[ j ] = plane.Dist( );

				} else if( plane.Normal( )[ j ] == -1.0f ) {

					mid[ j ] = -plane.Dist( );

				} else {

					mid[ j ] = ( float )( ( double )( *p1 )[ j ] + d * ( ( double )( *p2 )[ j ] - ( double )( *p1 )[ j ] ) );
				}
			}

			mid.s = ( float )( ( double )p1->s + d * ( ( double )p2->s - ( double )p1->s ) );
			mid.t = ( float )( ( double )p1->t + d * ( ( double )p2->t - ( double )p1->t ) );

		} else {

			double d = dists[ i + 1 ] / ( dists[ i + 1 ] - dists[ i ] );

			for( j = 0; j < 3; j++ ) {	

				// avoid round off error when possible
				if( plane.Normal( )[ j ] == 1.0f ) {

					mid[ j ] = plane.Dist( );

				} else if( plane.Normal( )[ j ] == -1.0f ) {

					mid[ j ] = -plane.Dist( );

				} else {

					mid[ j ] = ( float )( ( double )( *p2 )[ j ] + d * ( ( double )( *p1 )[ j ] - ( double )( *p2 )[ j ] ) );
				}
			}

			mid.s = ( float )( ( double )p2->s + d * ( ( double )p1->s - ( double )p2->s ) );
			mid.t = ( float )( ( double )p2->t + d * ( ( double )p1->t - ( double )p2->t ) );
		}

		front.p[ front.numPoints ] = mid;
		front.numPoints++;
		back.p[ back.numPoints ] = mid;
		back.numPoints++;
	}

	if( front.numPoints > maxpts || back.numPoints > maxpts ) {

		Common::Com_Error( ERR_DROP, "Winding::Split: points exceeded estimate." );
	}

	return SIDE_CROSS;
}

int Winding::Split( const Plane & plane, const float epsilon, Winding ** front, Winding ** back ) const {

	int				i, j;
	double *		dists;
	byte *			sides;
	int				counts[ 3 ];
	const Vec5 *	p1, *p2;
	Vec5			mid;
	Winding *		f, *b;
	int				maxpts;

	assert( this != NULL );

	dists = ( double * )_alloca( ( numPoints + 4 ) * sizeof( dists[ 0 ] ) );
	sides =  (byte * )_alloca( ( numPoints + 4 ) * sizeof( sides[ 0 ] ) );

	CalculateSides( p, numPoints, plane, epsilon, dists, sides, counts );

	*front = *back = NULL;

	// if coplanar, put on the front side if the normals match
	if( !counts[ SIDE_FRONT ] && !counts[ SIDE_BACK ] ) {

		Plane windingPlane;

		GetPlane( windingPlane );

		if( windingPlane.Normal( ) * plane.Normal( ) > 0.0f ) {

			*front = Copy( );
			return SIDE_FRONT;

		} else {

			*back = Copy( );
			return SIDE_BACK;
		}
	}

	// if nothing at the front of the clipping plane
	if( !counts[ SIDE_FRONT ] ) {

		*back = Copy( );
		return SIDE_BACK;
	}

	// if nothing at the back of the clipping plane
	if( !counts[ SIDE_BACK ] ) {

		*front = Copy( );
		return SIDE_FRONT;
	}

	maxpts = numPoints + 4;	// cannot use counts[ 0 ]+2 because of fp grouping errors

	*front = f = new Winding( maxpts );
	*back = b = new Winding( maxpts );
		
	for( i = 0; i < numPoints; i++ ) {

		p1 = &p[ i ];
		
		if( sides[ i ] == SIDE_ON ) {

			f->p[ f->numPoints ] = *p1;
			f->numPoints++;
			b->p[ b->numPoints ] = *p1;
			b->numPoints++;
			continue;
		}
	
		if( sides[ i ] == SIDE_FRONT ) {

			f->p[ f->numPoints ] = *p1;
			f->numPoints++;

		} else if( sides[ i ] == SIDE_BACK ) {

			b->p[ b->numPoints ] = *p1;
			b->numPoints++;
		}

		if( sides[ i + 1 ] == SIDE_ON || sides[ i + 1 ] == sides[ i ] ) {
			continue;
		}
			
		// generate a split point
		p2 = &p[ ( i + 1 ) % numPoints ];
		
		// always calculate the split going from the same side or minor epsilon issues can happen
		if( sides[ i ] == SIDE_FRONT ) {

			double d = dists[ i ] / ( dists[ i ] - dists[ i + 1 ] );

			for( j = 0; j < 3; j++ ) {

				// avoid round off error when possible
				if( plane.Normal( )[ j ] == 1.0f ) {

					mid[ j ] = plane.Dist( );

				} else if( plane.Normal( )[ j ] == -1.0f ) {

					mid[ j ] = -plane.Dist( );

				} else {

					mid[ j ] = ( float )( ( double )( *p1 )[ j ] + d * ( ( double )( *p2 )[ j ] - ( double )( *p1 )[ j ] ) );
				}
			}

			mid.s = ( float )( ( double )p1->s + d * ( ( double )p2->s - ( double )p1->s ) );
			mid.t = ( float )( ( double )p1->t + d * ( ( double )p2->t - ( double )p1->t ) );

		} else {

			double d = dists[ i + 1 ] / ( dists[ i + 1 ] - dists[ i ] );

			for( j = 0; j < 3; j++ ) {	

				// avoid round off error when possible
				if( plane.Normal( )[ j ] == 1.0f ) {

					mid[ j ] = plane.Dist( );

				} else if( plane.Normal( )[ j ] == -1.0f ) {

					mid[ j ] = -plane.Dist( );

				} else {

					mid[ j ] = ( float )( ( double )( *p2 )[ j ] + d * ( ( double )( *p1 )[ j ] - ( double )( *p2 )[ j ] ) );
				}
			}

			mid.s = ( float )( ( double )p2->s + d * ( ( double )p1->s - ( double )p2->s ) );
			mid.t = ( float )( ( double )p2->t + d * ( ( double )p1->t - ( double )p2->t ) );
		}

		f->p[ f->numPoints ] = mid;
		f->numPoints++;
		b->p[ b->numPoints ] = mid;
		b->numPoints++;
	}
	
	if( f->numPoints > maxpts || b->numPoints > maxpts ) {

		Common::Com_Error( ERR_DROP, "Winding::Split: points exceeded estimate." );
	}

	return SIDE_CROSS;
}

int Winding::SplitInPlace( const Plane & plane, const float epsilon, Winding ** back ) {

	int				i, j;
	double *		dists;
	byte *			sides;
	int				counts[ 3 ];
	Vec5 *			newPoints;
	int				newNumPoints;
	const Vec5 *	p1, *p2;
	Vec5			mid;
	Winding *		b;
	int				maxpts;

	assert( this );

	dists = ( double * )_alloca( ( numPoints + 4 ) * sizeof( dists[ 0 ] ) );
	sides = ( byte * )_alloca( ( numPoints + 4 ) * sizeof( sides[ 0 ] ) );

	CalculateSides( p, numPoints, plane, epsilon, dists, sides, counts );

	*back = NULL;

	// if coplanar, put on the front side if the normals match
	if( !counts[ SIDE_FRONT ] && !counts[ SIDE_BACK ] ) {

		Plane windingPlane;

		GetPlane( windingPlane );

		if( windingPlane.Normal( ) * plane.Normal( ) > 0.0f ) {

			return SIDE_FRONT;

		} else {

			return SIDE_BACK;
		}
	}

	// if nothing at the front of the clipping plane
	if( !counts[ SIDE_FRONT ] ) {

		return SIDE_BACK;
	}

	// if nothing at the back of the clipping plane
	if( !counts[ SIDE_BACK ] ) {

		return SIDE_FRONT;
	}

	maxpts = numPoints + 4;	// cannot use counts[ 0 ]+2 because of fp grouping errors

	newPoints = ( Vec5 * )_alloca16( maxpts * sizeof( newPoints[ 0 ] ) );
	newNumPoints = 0;

	*back = b = new Winding( maxpts );

	for( i = 0; i < numPoints; i++ ) {

		p1 = &p[ i ];

		if( sides[ i ] == SIDE_ON ) {

			newPoints[ newNumPoints ] = *p1;
			newNumPoints++;
			b->p[ b->numPoints ] = *p1;
			b->numPoints++;
			continue;
		}

		if( sides[ i ] == SIDE_FRONT ) {

			newPoints[ newNumPoints ] = *p1;
			newNumPoints++;

		} else if( sides[ i ] == SIDE_BACK ) {

			b->p[ b->numPoints ] = *p1;
			b->numPoints++;
		}

		if( sides[ i + 1 ] == SIDE_ON || sides[ i + 1 ] == sides[ i ] ) {

			continue;
		}
			
		// generate a split point
		p2 = &p[ ( i + 1 ) % numPoints ];
		
		// always calculate the split going from the same side or minor epsilon issues can happen
		if( sides[ i ] == SIDE_FRONT ) {

			double d = dists[ i ] / ( dists[ i ] - dists[ i + 1 ] );

			for( j = 0; j < 3; j++ ) {

				// avoid round off error when possible
				if( plane.Normal( )[ j ] == 1.0f ) {

					mid[ j ] = plane.Dist( );

				} else if( plane.Normal( )[ j ] == -1.0f ) {

					mid[ j ] = -plane.Dist( );

				} else {

					mid[ j ] = ( float )( ( double )( *p1 )[ j ] + d * ( ( double )( *p2 )[ j ] - ( double )( *p1 )[ j ] ) );
				}
			}

			mid.s = ( float )( ( double )p1->s + d * ( ( double )p2->s - ( double )p1->s ) );
			mid.t = ( float )( ( double )p1->t + d * ( ( double )p2->t - ( double )p1->t ) );

		} else {

			double d = dists[ i + 1 ] / ( dists[ i + 1 ] - dists[ i ] );

			for( j = 0; j < 3; j++ ) {	

				// avoid round off error when possible
				if( plane.Normal( )[ j ] == 1.0f ) {

					mid[ j ] = plane.Dist( );

				} else if( plane.Normal( )[ j ] == -1.0f ) {

					mid[ j ] = -plane.Dist( );

				} else {

					mid[ j ] = ( float )( ( double )( *p2 )[ j ] + d * ( ( double )( *p1 )[ j ] - ( double )( *p2 )[ j ] ) );
				}
			}

			mid.s = ( float )( ( double )p2->s + d * ( ( double )p1->s - ( double )p2->s ) );
			mid.t = ( float )( ( double )p2->t + d * ( ( double )p1->t - ( double )p2->t ) );
		}

		newPoints[ newNumPoints ] = mid;
		newNumPoints++;
		b->p[ b->numPoints ] = mid;
		b->numPoints++;
	}

	if( !EnsureAlloced( newNumPoints, false ) ) {

		return true;
	}

	numPoints = newNumPoints;
	memcpy( p, newPoints, newNumPoints * sizeof( p[ 0 ] ) );

	if( numPoints > maxpts || b->numPoints > maxpts ) {

		Common::Com_Error( ERR_DROP, "Winding::Split: points exceeded estimate." );
	}

	return SIDE_CROSS;
}

bool Winding::ClipInPlace( const Plane & plane, const float epsilon, const bool keepOn ) {

	int			i, j;
	double *	dists;
	byte *		sides;
	int			counts[ 3 ];
	Vec5 *		newPoints;
	int			newNumPoints;
	Vec5 *		p1, *p2;
	Vec5		mid;
	int			maxpts;

	assert( this );

	dists = (double *) _alloca( ( numPoints + 4 ) * sizeof( dists[ 0 ] ) );
	sides = (byte *) _alloca( ( numPoints + 4 ) * sizeof( sides[ 0 ] ) );

	CalculateSides( p, numPoints, plane, epsilon, dists, sides, counts );

	// if the winding is on the plane and we should keep it
	if( keepOn && !counts[ SIDE_FRONT ] && !counts[ SIDE_BACK ] ) {

		return true;
	}

	// if nothing at the front of the clipping plane
	if( !counts[ SIDE_FRONT ] ) {

		numPoints = 0;
		return false;
	}

	// if nothing at the back of the clipping plane
	if( !counts[ SIDE_BACK ] ) {

		return true;
	}

	maxpts = numPoints + 4;		// cannot use counts[ 0 ]+2 because of fp grouping errors

	newPoints = ( Vec5 * )_alloca16( maxpts * sizeof( newPoints[ 0 ] ) );
	newNumPoints = 0;
		
	for( i = 0; i < numPoints; i++ ) {

		p1 = &p[ i ];

		if( sides[ i ] == SIDE_ON ) {

			newPoints[ newNumPoints ] = *p1;
			newNumPoints++;
			continue;
		}

		if( sides[ i ] == SIDE_FRONT ) {

			newPoints[ newNumPoints ] = *p1;
			newNumPoints++;
		}

		if( sides[ i + 1 ] == SIDE_ON || sides[ i + 1 ] == sides[ i ] ) {

			continue;
		}

		// generate a split point
		p2 = &p[ ( i + 1 ) % numPoints ];

		// always calculate the split going from the same side or minor epsilon issues can happen
		if( sides[ i ] == SIDE_FRONT ) {

			double d = dists[ i ] / ( dists[ i ] - dists[ i + 1 ] );
			for( j = 0; j < 3; j++ ) {

				// avoid round off error when possible
				if( plane.Normal( )[ j ] == 1.0f ) {

					mid[ j ] = plane.Dist( );

				} else if( plane.Normal( )[ j ] == -1.0f ) {

					mid[ j ] = -plane.Dist( );

				} else {

					mid[ j ] = ( float )( ( double )( *p1 )[ j ] + d * ( ( double )( *p2 )[ j ] - ( double )( *p1 )[ j ] ) );
				}
			}

			mid.s = ( float )( ( double )p1->s + d * ( ( double )p2->s - ( double )p1->s ) );
			mid.t = ( float )( ( double )p1->t + d * ( ( double )p2->t - ( double )p1->t ) );

		} else {

			double d = dists[ i + 1 ] / ( dists[ i + 1 ] - dists[ i ] );

			for( j = 0; j < 3; j++ ) {

				// avoid round off error when possible
				if( plane.Normal( )[ j ] == 1.0f ) {

					mid[ j ] = plane.Dist( );

				} else if( plane.Normal( )[ j ] == -1.0f ) {

					mid[ j ] = -plane.Dist( );

				} else {

					mid[ j ] = ( float )( ( float )( ( double )( *p2 )[ j ] + d * ( ( double )( *p1 )[ j ] - ( double )( *p2 )[ j ] ) ) );
				}
			}

			mid.s = ( float )( ( double )p2->s + d * ( ( double )p1->s - ( double )p2->s ) );
			mid.t = ( float )( ( double )p2->t + d * ( ( double )p1->t - ( double )p2->t ) );
		}

		newPoints[ newNumPoints ] = mid;
		newNumPoints++;
	}
	
	if( !EnsureAlloced( newNumPoints, false ) ) {

		return true;
	}

	numPoints = newNumPoints;
	memcpy( p, newPoints, newNumPoints * sizeof( p[ 0 ] ) );

	return true;
}

int * CopyEdgeNums( const int *edgeNums, const int numEdgeNums ) {

	int *newEdgeNums = new int[numEdgeNums];

	for( int i = 0; i < numEdgeNums; i++ ) {

		newEdgeNums[ i ] = edgeNums[ i ];
	}

	return newEdgeNums;
}

int Winding::SplitWithEdgeNums( const int *edgeNums, const Plane & plane, const int edgeNum, const float epsilon, Winding ** front, Winding ** back, int ** frontEdgeNums, int ** backEdgeNums ) const {

	int				i, j;
	double *		dists;
	byte *			sides;
	int				counts[ 3 ];
	const Vec5 *	p1, *p2;
	Vec5			mid;
	Winding *		f, *b;
	int *			fe, *be;
	int				maxpts;

	assert( this );

	dists = ( double * )_alloca( ( numPoints + 4 ) * sizeof( dists[ 0 ] ) );
	sides = ( byte * )_alloca( ( numPoints + 4 ) * sizeof( sides[ 0 ] ) );

	CalculateSides( p, numPoints, plane, epsilon, dists, sides, counts );
	
	*front = *back = NULL;
	*frontEdgeNums = *backEdgeNums = NULL;

	// if coplanar, put on the front side if the normals match
	if( !counts[ SIDE_FRONT ] && !counts[ SIDE_BACK ] ) {

		Plane windingPlane;

		GetPlane( windingPlane );

		if( windingPlane.Normal( ) * plane.Normal( ) > 0.0f ) {

			*frontEdgeNums = CopyEdgeNums( edgeNums, numPoints );
			*front = Copy( );
			return SIDE_FRONT;

		} else {

			*backEdgeNums = CopyEdgeNums( edgeNums, numPoints );
			*back = Copy( );
			return SIDE_BACK;
		}
	}

	// if nothing at the front of the clipping plane
	if( !counts[ SIDE_FRONT ] ) {

		*backEdgeNums = CopyEdgeNums( edgeNums, numPoints );
		*back = Copy( );
		return SIDE_BACK;
	}

	// if nothing at the back of the clipping plane
	if( !counts[ SIDE_BACK ] ) {

		*frontEdgeNums = CopyEdgeNums( edgeNums, numPoints );
		*front = Copy( );
		return SIDE_FRONT;
	}

	maxpts = numPoints + 4;	// cannot use counts[ 0 ]+2 because of fp grouping errors

	*front = f = new Winding( maxpts );
	*back = b = new Winding( maxpts );

	*frontEdgeNums = fe = new int[maxpts];
	*backEdgeNums = be = new int[maxpts];

	for( i = 0; i < numPoints; i++ ) {

		p1 = &p[ i ];
		
		if( sides[ i ] == SIDE_ON ) {

			if( sides[ i + 1 ] == SIDE_FRONT ) {

				fe[ f->numPoints ] = edgeNums[ i ];
				be[ b->numPoints ] = edgeNum;

			} else if( sides[ i + 1 ] == SIDE_BACK ) {

				fe[ f->numPoints ] = edgeNum;
				be[ b->numPoints ] = edgeNums[ i ];

			} else {

				fe[ f->numPoints ] = edgeNums[ i ];
				be[ b->numPoints ] = edgeNums[ i ];
			}

			f->p[ f->numPoints ] = *p1;
			f->numPoints++;
			b->p[ b->numPoints ] = *p1;
			b->numPoints++;
			continue;
		}

		if( sides[ i ] == SIDE_FRONT ) {

			fe[ f->numPoints ] = edgeNums[ i ];
			f->p[ f->numPoints ] = *p1;
			f->numPoints++;

		} else if( sides[ i ] == SIDE_BACK ) {

			be[ b->numPoints ] = edgeNums[ i ];
			b->p[ b->numPoints ] = *p1;
			b->numPoints++;
		}

		if( sides[ i + 1 ] == SIDE_ON || sides[ i + 1 ] == sides[ i ] ) {

			continue;
		}

		// generate a split point
		p2 = &p[ ( i + 1 ) % numPoints ];
		
		// always calculate the split going from the same side or minor epsilon issues can happen
		if( sides[ i ] == SIDE_FRONT ) {

			double d = dists[ i ] / ( dists[ i ] - dists[ i + 1 ] );

			for( j = 0; j < 3; j++ ) {

				// avoid round off error when possible
				if( plane.Normal( )[ j ] == 1.0f ) {

					mid[ j ] = plane.Dist( );

				} else if( plane.Normal( )[ j ] == -1.0f ) {

					mid[ j ] = -plane.Dist( );

				} else {

					mid[ j ] = ( float )( ( double )( *p1 )[ j ] + d * ( ( double )( *p2 )[ j ] - ( double )( *p1 )[ j ] ) );
				}
			}

			mid.s = ( float )( ( double )p1->s + d * ( ( double )p2->s - ( double )p1->s ) );
			mid.t = ( float )( ( double )p1->t + d * ( ( double )p2->t - ( double )p1->t ) );

			fe[ f->numPoints ] = edgeNum;
			be[ b->numPoints ] = edgeNums[ i ];

		} else {

			double d = dists[ i + 1 ] / ( dists[ i + 1 ] - dists[ i ] );

			for( j = 0; j < 3; j++ ) {

				// avoid round off error when possible
				if( plane.Normal( )[ j ] == 1.0f ) {

					mid[ j ] = plane.Dist( );

				} else if( plane.Normal( )[ j ] == -1.0f ) {

					mid[ j ] = -plane.Dist( );

				} else {

					mid[ j ] = ( float )( ( double )( *p2 )[ j ] + d * ( ( double )( *p1 )[ j ] - ( double )( *p2 )[ j ] ) );
				}
			}

			mid.s = ( float )( ( double )p2->s + d * ( ( double )p1->s - ( double )p2->s ) );
			mid.t = ( float )( ( double )p2->t + d * ( ( double )p1->t - ( double )p2->t ) );

			fe[ f->numPoints ] = edgeNums[ i ];
			be[ b->numPoints ] = edgeNum;
		}

		f->p[ f->numPoints ] = mid;
		f->numPoints++;
		b->p[ b->numPoints ] = mid;
		b->numPoints++;
	}
	
	if( f->numPoints > maxpts || b->numPoints > maxpts ) {

		Common::Com_Error( ERR_DROP, "Winding::Split: points exceeded estimate." );
	}

	return SIDE_CROSS;
}

int Winding::SplitInPlaceWithEdgeNums( List< int > &edgeNums, const Plane & plane, const float epsilon, Winding **back, int **backEdgePlanes ) {

	assert( 0 );
	return SIDE_FRONT;
}

int Winding::ClipInPlaceWithEdgeNums( List< int > &edgeNums, const Plane & plane, const int edgeNum, const float epsilon, const bool keepOn ) {

	int			i, j;
	double *	dists;
	byte *		sides;
	int			counts[ 3 ];
	Vec5 *		newPoints;
	int *		newEdgeNums;
	int			newNumPoints;
	Vec5 *		p1, * p2;
	Vec5		mid;
	int			maxpts;

	assert( this );

	dists = ( double * )_alloca( ( numPoints + 4 ) * sizeof( dists[ 0 ] ) );
	sides = ( byte * )_alloca( ( numPoints + 4 ) * sizeof( sides[ 0 ] ) );

	CalculateSides( p, numPoints, plane, epsilon, dists, sides, counts );
	
	// if the winding is on the plane and we should keep it
	if( keepOn && !counts[ SIDE_FRONT ] && !counts[ SIDE_BACK ] ) {

		return SIDE_ON;
	}

	// if nothing at the front of the clipping plane
	if( !counts[ SIDE_FRONT ] ) {

		numPoints = 0;
		return SIDE_BACK;
	}

	// if nothing at the back of the clipping plane
	if( !counts[ SIDE_BACK ] ) {

		return SIDE_FRONT;
	}

	maxpts = numPoints + 4;		// cannot use counts[ 0 ]+2 because of fp grouping errors

	newPoints = ( Vec5 * )_alloca16( maxpts * sizeof( newPoints[ 0 ] ) );
	newEdgeNums = ( int * )_alloca16( maxpts * sizeof( newEdgeNums[ 0 ] ) );
	newNumPoints = 0;

	for( i = 0; i < numPoints; i++ ) {

		p1 = &p[ i ];

		if( sides[ i ] == SIDE_ON ) {

			if( sides[ i + 1 ] == SIDE_FRONT ) {

				newEdgeNums[ newNumPoints ] = edgeNums[ i ];

			} else if( sides[ i + 1 ] == SIDE_BACK ) {

				newEdgeNums[ newNumPoints ] = edgeNum;

			} else {

				newEdgeNums[ newNumPoints ] = edgeNums[ i ];
			}

			newPoints[ newNumPoints ] = *p1;
			newNumPoints++;
			continue;
		}
	
		if( sides[ i ] == SIDE_FRONT ) {

			newEdgeNums[ newNumPoints ] = edgeNums[ i ];
			newPoints[ newNumPoints ] = *p1;
			newNumPoints++;
		}

		if( sides[ i + 1 ] == SIDE_ON || sides[ i + 1 ] == sides[ i ] ) {

			continue;
		}
			
		// generate a split point
		p2 = &p[ ( i + 1 ) % numPoints ];
		
		// always calculate the split going from the same side or minor epsilon issues can happen
		if( sides[ i ] == SIDE_FRONT ) {

			double d = dists[ i ] / ( dists[ i ] - dists[ i + 1 ] );

			for( j = 0; j < 3; j++ ) {

				// avoid round off error when possible
				if( plane.Normal( )[ j ] == 1.0f ) {

					mid[ j ] = plane.Dist( );

				} else if( plane.Normal( )[ j ] == -1.0f ) {

					mid[ j ] = -plane.Dist( );

				} else {

					mid[ j ] = ( float )( ( double )( *p1 )[ j ] + d * ( ( double )( *p2 )[ j ] - ( double )( *p1 )[ j ] ) );
				}
			}

			mid.s = ( float )( ( double )p1->s + d * ( ( double )p2->s - ( double )p1->s ) );
			mid.t = ( float )( ( double )p1->t + d * ( ( double )p2->t - ( double )p1->t ) );

			newEdgeNums[ newNumPoints ] = edgeNum;

		} else {

			double d = dists[ i + 1 ] / ( dists[ i + 1 ] - dists[ i ] );

			for( j = 0; j < 3; j++ ) {	

				// avoid round off error when possible
				if( plane.Normal( )[ j ] == 1.0f ) {

					mid[ j ] = plane.Dist( );

				} else if( plane.Normal( )[ j ] == -1.0f ) {

					mid[ j ] = -plane.Dist( );

				} else {

					mid[ j ] = ( float )( ( double )( *p2 )[ j ] + d * ( ( double )( *p1 )[ j ] - ( double )( *p2 )[ j ] ) );
				}
			}

			mid.s = ( float )( ( double )p2->s + d * ( ( double )p1->s - ( double )p2->s ) );
			mid.t = ( float )( ( double )p2->t + d * ( ( double )p1->t - ( double )p2->t ) );

			newEdgeNums[ newNumPoints ] = edgeNums[ i ];
		}
			
		newPoints[ newNumPoints ] = mid;
		newNumPoints++;
	}

	if( !EnsureAlloced( newNumPoints, false ) ) {

		return true;
	}

	numPoints = newNumPoints;
	memcpy( p, newPoints, newNumPoints * sizeof( p[ 0 ] ) );

	edgeNums.SetNum( newNumPoints, false );
	memcpy( edgeNums.Ptr( ), newEdgeNums, newNumPoints * sizeof( edgeNums[ 0 ] ) );

	return SIDE_CROSS;
}

Winding * Winding::Copy( ) const {

	Winding * w;

	w = new Winding( numPoints );
	w->numPoints = numPoints;
	memcpy( w->p, p, numPoints * sizeof( p[ 0 ] ) );

	return w;
}

Winding * Winding::Reverse( ) const {

	Winding * w;
	int i;

	w = new Winding( numPoints );
	w->numPoints = numPoints;

	for( i = 0; i < numPoints; i++ ) {

		w->p[ numPoints - i - 1 ] = p[ i ];
	}

	return w;
}

void Winding::ReverseSelf( ) {

	Vec5 v;
	int i;

	for( i = 0; i < ( numPoints >> 1 ); i++ ) {

		v = p[ i ];
		p[ i ] = p[ numPoints - i - 1 ];
		p[ numPoints - i - 1 ] = v;
	}
}

bool Winding::Check( bool print ) const {

	int				i, j;
	float			d, edgedist;
	Vec3			dir, edgenormal;
	float			area;
	Plane			plane;

	if( numPoints < 3 ) {

		if( print ) {

			Common::Com_DPrintf( "Winding::Check: only %i points.", numPoints );
		}

		return false;
	}
	
	area = GetArea( );

	if( area < 1.0f ) {

		if( print ) {

			Common::Com_DPrintf( "Winding::Check: tiny area: %f", area );
		}

		return false;
	}

	GetPlane( plane );
	
	for( i = 0; i < numPoints; i++ ) {

		const Vec3 &p1 = p[ i ].ToVec3( );

		// check if the winding is huge
		for( j = 0; j < 3; j++ ) {

			if( p1[ j ] >= MAX_WORLD_COORD || p1[ j ] <= MIN_WORLD_COORD ) {

				if( print ) {

					Common::Com_DPrintf( "Winding::Check: point %d outside world %c-axis: %f", i, 'X'+j, p1[ j ] );
				}
				return false;
			}
		}

		j = i + 1 == numPoints ? 0 : i + 1;
		
		// check if the point is on the face plane
		d = p1 * plane.Normal( ) + plane[ 3 ];
		if( d < -ON_EPSILON || d > ON_EPSILON ) {

			if( print ) {

				Common::Com_DPrintf( "Winding::Check: point %d off plane.", i );
			}
			return false;
		}
	
		// check if the edge isn't degenerate
		const Vec3 &p2 = p[ j ].ToVec3( );
		dir = p2 - p1;
		
		if( dir.Length( ) < ON_EPSILON) {

			if( print ) {

				Common::Com_DPrintf( "Winding::Check: edge %d is degenerate.", i );
			}
			return false;
		}

		// check if the winding is convex
		edgenormal = plane.Normal( ).Cross( dir );
		edgenormal.Normalize( );
		edgedist = p1 * edgenormal;
		edgedist += ON_EPSILON;
		
		// all other points must be on front side
		for( j = 0; j < numPoints; j++ ) {

			if( j == i ) {

				continue;
			}

			d = p[ j ].ToVec3( ) * edgenormal;

			if( d > edgedist ) {

				if( print ) {

					Common::Com_DPrintf( "Winding::Check: non-convex." );
				}

				return false;
			}
		}
	}

	return true;
}

float Winding::GetArea( ) const {

	Vec3 d1, d2, cross;
	float total  = 0.0f;

	for( int i = 2; i < numPoints; i++ ) {

		d1 = p[i-1].ToVec3( ) - p[ 0 ].ToVec3( );
		d2 = p[ i ].ToVec3( ) - p[ 0 ].ToVec3( );
		cross = d1.Cross( d2 );
		total += cross.Length( );
	}

	return total * 0.5f;
}

float Winding::GetRadius( const Vec3 &center ) const {

	float radius = 0.0f, r;
	Vec3 dir;

	for( int i = 0; i < numPoints; i++ ) {

		dir = p[ i ].ToVec3( ) - center;
		r = dir * dir;

		if( r > radius ) {

			radius = r;
		}
	}

	return sqrtf( radius );
}

Vec3 Winding::GetCenter( ) const {

	Vec3 center;

	center.Zero( );


	for( int i = 0; i < numPoints; i++ ) {
		center += p[ i ].ToVec3( );
	}

	center *= ( 1.0f / numPoints );

	return center;
}

Vec3 Winding::GetNormal( ) const {

	Vec3 v1, v2, center, normal;

	if( numPoints < 3 ) {

		normal.Zero( );
		return normal;
	}

	center = GetCenter( );
	v1 = p[ 0 ].ToVec3( ) - center;
	v2 = p[ 1 ].ToVec3( ) - center;
	normal = v2.Cross( v1 );
	normal.Normalize( );

	return normal;
}

void Winding::GetPlane( Vec3 & normal, float &dist ) const {

	Vec3 v1, v2, center;

	if( numPoints < 3 ) {

		normal.Zero( );
		dist = 0.0f;
		return;
	}

	center = GetCenter( );
	v1 = p[ 0 ].ToVec3( ) - center;
	v2 = p[ 1 ].ToVec3( ) - center;
	normal = v2.Cross( v1 );
	normal.Normalize( );
	dist = p[ 0 ].ToVec3( ) * normal;
}

void Winding::GetPlane( Plane & plane ) const {

	Vec3 v1, v2;
	Vec3 center;

	if( numPoints < 3 ) {

		plane.Zero( );
		return;
	}

	center = GetCenter( );
	v1 = p[ 0 ].ToVec3( ) - center;
	v2 = p[ 1 ].ToVec3( ) - center;
	plane.SetNormal( v2.Cross( v1 ) );
	plane.Normalize( );
	plane.FitThroughPoint( p[ 0 ].ToVec3( ) );
}

Plane Winding::GetPlane( ) const {

	Plane toReturn;
	GetPlane( toReturn );
	return toReturn;
}

void Winding::GetBounds( Bounds &bounds ) const {

	if( !numPoints ) {

		bounds.Clear( );
		return;
	}

	bounds[ 0 ] = bounds[ 1 ] = p[ 0 ].ToVec3( );

	for( int i = 1; i < numPoints; i++ ) {

		if( p[ i ].x < bounds[ 0 ].x ) {

			bounds[ 0 ].x = p[ i ].x;

		} else if( p[ i ].x > bounds[ 1 ].x ) {

			bounds[ 1 ].x = p[ i ].x;
		}

		if( p[ i ].y < bounds[ 0 ].y ) {

			bounds[ 0 ].y = p[ i ].y;

		} else if( p[ i ].y > bounds[ 1 ].y ) {

			bounds[ 1 ].y = p[ i ].y;
		}

		if( p[ i ].z < bounds[ 0 ].z ) {

			bounds[ 0 ].z = p[ i ].z;

		} else if( p[ i ].z > bounds[ 1 ].z ) {

			bounds[ 1 ].z = p[ i ].z;
		}
	}
}

Bounds Winding::GetBounds( ) const {

	Bounds toReturn;
	GetBounds( toReturn );
	return toReturn;
}

void Winding::RemoveEqualPoints( const float epsilon ) {

	for( int i = 0; i < numPoints; i++ ) {

		if( ( p[ i ].ToVec3( ) - p[ ( i + numPoints - 1 ) % numPoints ].ToVec3( ) ).LengthSqr( ) >= Square( epsilon ) ) {

			continue;
		}

		numPoints--;

		for( int j = i; j < numPoints; j++ ) {

			p[ j ] = p[ j + 1 ];
		}

		i--;
	}
}

void Winding::RemoveColinearPoints( const Vec3 & normal, const float epsilon ) {

	Vec3 edgeNormal;
	float dist;

	if( numPoints <= 3 ) {

		return;
	}

	for( int i = 0; i < numPoints; i++ ) {

		// create plane through edge orthogonal to winding plane
		edgeNormal = (p[ i ].ToVec3( ) - p[(i+numPoints-1)%numPoints].ToVec3( )).Cross( normal );
		edgeNormal.Normalize( );
		dist = edgeNormal * p[ i ].ToVec3( );

		if( fabs( edgeNormal * p[(i+1)%numPoints].ToVec3( ) - dist ) > epsilon ) {

			continue;
		}

		numPoints--;
		for( int j = i; j < numPoints; j++ ) {

			p[ j ] = p[j+1];
		}

		i--;
	}
}

void Winding::AddToConvexHull( const Winding *winding, const Vec3 & normal, const float epsilon ) {

	int				i, j, k;
	Vec3			dir;
	float			d;
	int				maxPts;
	Vec3 *		hullDirs;
	bool *			hullSide;
	bool			outside;
	int				numNewHullPoints;
	Vec5 *		newHullPoints;

	if( !winding ) {

		return;
	}

	maxPts = this->numPoints + winding->numPoints;

	if( !this->EnsureAlloced( maxPts, true ) ) {

		return;
	}

	newHullPoints = ( Vec5 * )_alloca( maxPts * sizeof( Vec5 ) );
	hullDirs = ( Vec3 * )_alloca( maxPts * sizeof( Vec3 ) );
	hullSide = ( bool * )_alloca( maxPts * sizeof( bool ) );

	for( i = 0; i < winding->numPoints; i++ ) {

		const Vec5 &p1 = winding->p[ i ];

		// calculate hull edge vectors
		for( j = 0; j < this->numPoints; j++ ) {

			dir = this->p[ (j + 1) % this->numPoints ].ToVec3( ) - this->p[ j ].ToVec3( );
			dir.Normalize( );
			hullDirs[ j ] = normal.Cross( dir );
		}

		// calculate side for each hull edge
		outside = false;
		for( j = 0; j < this->numPoints; j++ ) {

			dir = p1.ToVec3( ) - this->p[ j ].ToVec3( );
			d = dir * hullDirs[ j ];
			if( d >= epsilon ) {

				outside = true;
			}

			if( d >= -epsilon ) {

				hullSide[ j ] = true;

			} else {

				hullSide[ j ] = false;
			}
		}

		// if the point is effectively inside, do nothing
		if( !outside ) {

			continue;
		}

		// find the back side to front side transition
		for( j = 0; j < this->numPoints; j++ ) {

			if( !hullSide[ j ] && hullSide[ ( j + 1 ) % this->numPoints ] ) {

				break;
			}
		}

		if( j >= this->numPoints ) {

			continue;
		}

		// insert the point here
		newHullPoints[ 0 ] = p1;
		numNewHullPoints = 1;

		// copy over all points that aren't double fronts
		j = ( j + 1 ) % this->numPoints;

		for( k = 0; k < this->numPoints; k++ ) {

			if( hullSide[ (j+k) % this->numPoints ] && hullSide[ (j+k+1) % this->numPoints ] ) {

				continue;
			}

			newHullPoints[numNewHullPoints] = this->p[ (j+k+1) % this->numPoints ];
			numNewHullPoints++;
		}

		this->numPoints = numNewHullPoints;
		memcpy( this->p, newHullPoints, numNewHullPoints * sizeof(Vec5) );
	}
}

void Winding::AddToConvexHull( const Vec3 &point, const Vec3 & normal, const float epsilon ) {

	int				j, k, numHullPoints;
	Vec3			dir;
	float			d;
	Vec3 *		hullDirs;
	bool *			hullSide;
	Vec5 *		hullPoints;
	bool			outside;

	switch( numPoints ) {

		case 0:
			p[ 0 ] = point;
			numPoints++;
			return;

		case 1:

			// don't add the same point second
			if( p[ 0 ].ToVec3( ).Compare( point, epsilon ) ) {

				return;
			}

			p[ 1 ].ToVec3( ) = point;
			numPoints++;
			return;

		case 2:

			// don't add a point if it already exists
			if( p[ 0 ].ToVec3( ).Compare( point, epsilon ) || p[ 1 ].ToVec3( ).Compare( point, epsilon ) ) {

				return;
			}

			// if only two points make sure we have the right ordering according to the normal
			dir = point - p[ 0 ].ToVec3( );
			dir = dir.Cross( p[ 1 ].ToVec3( ) - p[ 0 ].ToVec3( ) );

			if( dir[ 0 ] == 0.0f && dir[ 1 ] == 0.0f && dir[ 2 ] == 0.0f ) {

				// points don't make a plane
				
				return;
			}
			if( dir * normal > 0.0f ) {

				p[ 2 ].ToVec3( ) = point;

			} else {

				p[ 2 ] = p[ 1 ];
				p[ 1 ].ToVec3( ) = point;
			}

			numPoints++;
			return;
	}

	hullDirs = ( Vec3 * )_alloca( numPoints * sizeof( Vec3 ) );
	hullSide = ( bool * )_alloca( numPoints * sizeof( bool ) );

	// calculate hull edge vectors
	for( j = 0; j < numPoints; j++ ) {

		dir = p[ ( j + 1 ) % numPoints].ToVec3( ) - p[ j ].ToVec3( );
		hullDirs[ j ] = normal.Cross( dir );
	}

	// calculate side for each hull edge
	outside = false;
	for( j = 0; j < numPoints; j++ ) {

		dir = point - p[ j ].ToVec3( );
		d = dir * hullDirs[ j ];

		if( d >= epsilon ) {

			outside = true;
		}

		if( d >= -epsilon ) {

			hullSide[ j ] = true;

		} else {

			hullSide[ j ] = false;
		}
	}

	// if the point is effectively inside, do nothing
	if( !outside ) {

		return;
	}

	// find the back side to front side transition
	for( j = 0; j < numPoints; j++ ) {

		if( !hullSide[ j ] && hullSide[ (j + 1) % numPoints ] ) {

			break;
		}
	}

	if( j >= numPoints ) {

		return;
	}

	hullPoints = ( Vec5 * )_alloca( ( numPoints + 1 ) * sizeof( Vec5 ) );

	// insert the point here
	hullPoints[ 0 ] = point;
	numHullPoints = 1;

	// copy over all points that aren't double fronts
	j = ( j + 1 ) % numPoints;

	for( k = 0; k < numPoints; k++ ) {

		if( hullSide[ (j+k) % numPoints ] && hullSide[ (j+k+1) % numPoints ] ) {

			continue;
		}

		hullPoints[numHullPoints] = p[ (j+k+1) % numPoints ];
		numHullPoints++;
	}

	if( !EnsureAlloced( numHullPoints, false ) ) {

		return;
	}

	numPoints = numHullPoints;
	memcpy( p, hullPoints, numHullPoints * sizeof(Vec5) );
}

#define	CONTINUOUS_EPSILON	0.005f

Winding *Winding::TryMerge( const Winding &w, const Vec3 &planenormal, int keep ) const {

	Vec3			*p1, *p2, *p3, *p4, *back;
	Winding *		newf;
	const Winding	*f1, *f2;
	int				i, j, k, l;
	Vec3			normal, delta;
	float			dot;
	bool			keep1, keep2;

	f1 = this;
	f2 = &w;
	p1 = p2 = NULL;
	j = 0;
	
	for( i = 0; i < f1->numPoints; i++ ) {

		p1 = &f1->p[ i ].ToVec3( );
		p2 = &f1->p[(i+1) % f1->numPoints].ToVec3( );

		for( j = 0; j < f2->numPoints; j++ ) {

			p3 = &f2->p[ j ].ToVec3( );
			p4 = &f2->p[(j+1) % f2->numPoints].ToVec3( );

			for(k = 0; k < 3; k++ ) {

				if( fabs(( *p1 )[ k ] - (*p4)[ k ]) > 0.1f ) {

					break;
				}

				if( fabs(( *p2 )[ k ] - ( *p3 )[ k ]) > 0.1f ) {

					break;
				}
			}

			if( k == 3 ) {

				break;
			}
		}

		if( j < f2->numPoints ) {

			break;
		}
	}
	
	if( i == f1->numPoints ) {

		return NULL;			// no matching edges
	}

	//
	// check slope of connected lines
	// if the slopes are colinear, the point can be removed
	//
	back = &f1->p[ ( i + f1->numPoints - 1 ) % f1->numPoints ].ToVec3( );
	delta = ( *p1 ) - ( *back );
	normal = planenormal.Cross( delta );
	normal.Normalize( );
	
	back = &f2->p[ ( j + 2 ) % f2->numPoints ].ToVec3( );
	delta = ( *back ) - ( *p1 );
	dot = delta * normal;

	if( dot > CONTINUOUS_EPSILON ) {

		return NULL;			// not a convex polygon
	}

	keep1 = ( bool )( dot < -CONTINUOUS_EPSILON );
	
	back = &f1->p[ ( i + 2 ) % f1->numPoints ].ToVec3( );
	delta = ( *back ) - ( *p2 );
	normal = planenormal.Cross( delta );
	normal.Normalize( );

	back = &f2->p[ ( j + f2->numPoints - 1 ) % f2->numPoints ].ToVec3( );
	delta = ( *back ) - ( *p2 );
	dot = delta * normal;

	if( dot > CONTINUOUS_EPSILON ) {

		return NULL;			// not a convex polygon
	}

	keep2 = (bool)(dot < -CONTINUOUS_EPSILON);

	//
	// build the new polygon
	//
	newf = new Winding( f1->numPoints + f2->numPoints );
	
	// copy first polygon
	for( k = ( i + 1 ) % f1->numPoints; k != i; k = ( k + 1 ) % f1->numPoints ) {

		if( !keep && k == ( i + 1 ) % f1->numPoints && !keep2 ) {

			continue;
		}
		
		newf->p[ newf->numPoints ] = f1->p[ k ];
		newf->numPoints++;
	}
	
	// copy second polygon
	for( l = ( j + 1 ) % f2->numPoints; l != j; l = ( l + 1 ) % f2->numPoints ) {

		if( !keep && l == ( j + 1 ) % f2->numPoints && !keep1 ) {

			continue;
		}

		newf->p[ newf->numPoints ] = f2->p[ l ];
		newf->numPoints++;
	}

	return newf;
}

void Winding::RemovePoint( int point ) {

	if( point < 0 || point >= numPoints ) {

		Common::Com_Error( ERR_DROP, "Winding::removePoint: point out of range" );
	}

	if( point < numPoints - 1) {

		memmove( &p[ point ], &p[ point + 1 ], ( numPoints - point - 1 ) * sizeof( p[ 0 ] ) );
	}

	numPoints--;
}

void Winding::InsertPoint( const Vec3 &point, int spot ) {

	if( spot > numPoints ) {

		Common::Com_Error( ERR_DROP, "Winding::insertPoint: spot > numPoints" );
	}

	if( spot < 0 ) {

		Common::Com_Error( ERR_DROP, "Winding::insertPoint: spot < 0" );
	}

	EnsureAlloced( numPoints + 1, true );

	for( int i = numPoints; i > spot; i-- ) {

		p[ i ] = p[ i - 1 ];
	}

	p[ spot ] = point;
	numPoints++;
}

bool Winding::InsertPointIfOnEdge( const Vec3 & point, const Plane & plane, const float epsilon, int * index ) {

	int i;
	float d, len;
	Vec3 dir, projected;

	// point may not be too far from the winding plane
	if( fabs( plane.Distance( point ) ) > epsilon ) {

		return false;
	}

	for( i = 0; i < numPoints; i++ ) {

		const Vec3 &p0 = p[ i ].ToVec3( );
		const Vec3 &p1 = p[ ( i + 1 ) % numPoints].ToVec3( );

		dir = p1 - p0;
		len = dir.Normalize( );

		// skip if the point is close to one of the edge points
		d = ( ( point - p0 ) * dir );

		if( d <= epsilon || d >= len - epsilon ) {
			continue;
		}

		// make sure the point is on the edge
		projected = p0 + d * dir;
		d = ( projected - point ).LengthSqr( );

		if( d >= epsilon * epsilon ) {

			continue;
		}

		InsertPoint( projected, i + 1 );

		if( index != NULL ) {

			*index = i + 1;
		}

		return true;
	}

	return false;
}

int Winding::CreateTriangles( int *indices, const float epsilon ) const {

	int i, a, b, c, bestCorner;
	float lengthSqr, dist1Sqr, dist2Sqr, bestDistSqr;
	Vec3 v1, v2;

	assert( numPoints >= 3 );

	if( numPoints == 3 ) {

		for( i = 0; i < numPoints; i++ ) {

			indices[ i ] = i;
		}

		return numPoints;
	}

	bestDistSqr = 0.0f;
	bestCorner = -1;

	for( i = 0; i < numPoints; i++ ) {

		a = i;
		b = ( i + 1 ) % numPoints;
		c = ( i + 2 ) % numPoints;
		v1 = p[ c ].ToVec3( ) - p[ a ].ToVec3( );
		v2 = p[ b ].ToVec3( ) - p[ a ].ToVec3( );
		lengthSqr = v1.LengthSqr( );

		if( lengthSqr < epsilon * epsilon ) {

			continue;
		}

		v1 = v2 - ( v1 * v2 / lengthSqr ) * v1;
		dist1Sqr = v1.LengthSqr( );

		if( dist1Sqr < epsilon * epsilon ) {

			continue;
		}

		a = i;
		b = ( i - 2 + numPoints ) % numPoints;
		c = ( i - 1 + numPoints ) % numPoints;
		v1 = p[ c ].ToVec3( ) - p[ a ].ToVec3( );
		v2 = p[ b ].ToVec3( ) - p[ a ].ToVec3( );
		lengthSqr = v1.LengthSqr( );

		if( lengthSqr < epsilon * epsilon ) {

			continue;
		}

		v1 = v2 - ( v1 * v2 / lengthSqr ) * v1;
		dist2Sqr = v1.LengthSqr( );

		if( dist2Sqr < epsilon * epsilon ) {

			continue;
		}

		if( dist1Sqr + dist2Sqr > bestDistSqr ) {

			bestDistSqr = dist1Sqr + dist2Sqr;
			bestCorner = i;
		}
	}

	if( bestCorner != -1 ) {

		// fan out from corner
		for( i = 0; i < numPoints - 2; i++ ) {

			indices[ i * 3 + 0 ] = bestCorner;
			indices[ i * 3 + 1 ] = ( bestCorner + i + 1 ) % numPoints;
			indices[ i * 3 + 2 ] = ( bestCorner + i + 2 ) % numPoints;
		}

		return ( numPoints - 2 ) * 3;

	} else {

		// fan out from center
		// FIXME: could also try ear clipping
		for( i = 0; i < numPoints; i++ ) {

			indices[ i * 3 + 0 ] = numPoints;					// index of center = numPoints
			indices[ i * 3 + 1 ] = i;
			indices[ i * 3 + 2 ] = ( i + 1 ) % numPoints;
		}

		return numPoints * 3;
	}
}

#define	EDGE_LENGTH		0.2f

bool Winding::IsTiny( ) const {

	int		i;
	float	len;
	Vec3	delta;
	int		edges;

	edges = 0;
	for( i = 0; i < numPoints; i++ ) {

		delta = p[(i+1)%numPoints].ToVec3( ) - p[ i ].ToVec3( );
		len = delta.Length( );

		if( len > EDGE_LENGTH ) {

			if( ++edges == 3 ) {

				return false;
			}
		}
	}

	return true;
}

bool Winding::IsTiny( float epsilon ) const {

	float	len;
	Vec3	delta;
	int		edges = 0;

	for( int i = 0; i < numPoints; i++ ) {

		delta = p[(i+1)%numPoints].ToVec3( ) - p[ i ].ToVec3( );
		len = delta.Length( );

		if( len > epsilon ) {

			if( ++edges == 3 ) {

				return false;
			}
		}
	}
	return true;
}

bool Winding::IsHuge( float radius ) const {

	for( int i = 0; i < numPoints; i++ ) {

		for( int j = 0; j < 3; j++ ) {

			if( p[ i ][ j ] <= -radius || p[ i ][ j ] >= radius ) {

				return true;
			}
		}
	}

	return false;
}

bool Winding::IsHuge( ) const {

	for( int i = 0; i < numPoints; i++ ) {

		for( int j = 0; j < 3; j++ ) {

			if( p[ i ][ j ] <= MIN_WORLD_COORD || p[ i ][ j ] >= MAX_WORLD_COORD ) {

				return true;
			}
		}
	}

	return false;
}

void Winding::Print( ) const {

	for( int i = 0; i < numPoints; i++ ) {

		Common::Com_DPrintf( "( %5.1f, %5.1f, %5.1f )\n", p[ i ][ 0 ], p[ i ][ 1 ], p[ i ][ 2 ] );
	}
}

float Winding::PlaneDistance( const Plane & plane ) const {

	float min = INFINITY, max = -INFINITY;

	for( int i = 0; i < numPoints; i++ ) {

		float d = plane.Distance( p[ i ].ToVec3( ) );

		if( d < min ) {

			min = d;

			if( FLOATSIGNBITSET( min ) & FLOATSIGNBITNOTSET( max ) ) {

				return 0.0f;
			}
		}

		if( d > max ) {

			max = d;

			if( FLOATSIGNBITSET( min ) & FLOATSIGNBITNOTSET( max ) ) {

				return 0.0f;
			}
		}
	}

	if( FLOATSIGNBITNOTSET( min ) ) {

		return min;
	}

	if( FLOATSIGNBITSET( max ) ) {

		return max;
	}

	return 0.0f;
}

int Winding::PlaneSide( const Plane & plane, const float epsilon ) const {

	bool	front = false, back = false;

	for( int i = 0; i < numPoints; i++ ) {

		float d = plane.Distance( p[ i ].ToVec3( ) );

		if( d < -epsilon ) {

			if( front ) {

				return SIDE_CROSS;
			}

			back = true;
			continue;

		} else if( d > epsilon ) {

			if( back ) {

				return SIDE_CROSS;
			}

			front = true;
			continue;
		}
	}

	if( back ) {

		return SIDE_BACK;
	}

	if( front ) {

		return SIDE_FRONT;
	}

	return SIDE_ON;
}

#define WCONVEX_EPSILON		0.2f

bool Winding::PlanesConcave( const Winding &w2, const Vec3 & normal1, const Vec3 & normal2, float dist1, float dist2 ) const {

	// check if one of the points of winding 1 is at the back of the plane of winding 2
	for( int i = 0; i < numPoints; i++ ) {

		if( normal2 * p[ i ].ToVec3( ) - dist2 > WCONVEX_EPSILON ) {

			return true;
		}
	}
	// check if one of the points of winding 2 is at the back of the plane of winding 1
	for( int i = 0; i < w2.numPoints; i++ ) {

		if( normal1 * w2.p[ i ].ToVec3( ) - dist1 > WCONVEX_EPSILON ) {

			return true;
		}
	}

	return false;
}

bool Winding::PointInside( const Vec3 & normal, const Vec3 & point, const float epsilon ) const {

	int i;
	Vec3 dir, n, pointvec;

	for( i = 0; i < numPoints; i++ ) {

		dir = p[ ( i + 1 ) % numPoints].ToVec3( ) - p[ i ].ToVec3( );
		pointvec = point - p[ i ].ToVec3( );

		n = dir.Cross( normal );
		n.Normalize( );

		if( pointvec * n < -epsilon ) {

			return false;
		}
	}
	return true;
}

bool Winding::LineIntersection( const Plane & windingPlane, const Vec3 & start, const Vec3 & end, bool backFaceCull ) const {

	float front, back, frac;
	Vec3 mid;

	front = windingPlane.Distance( start );
	back = windingPlane.Distance( end );

	// if both points at the same side of the plane
	if( front < 0.0f && back < 0.0f ) {

		return false;
	}

	if( front > 0.0f && back > 0.0f ) {

		return false;
	}

	// if back face culled
	if( backFaceCull && front < 0.0f ) {

		return false;
	}

	// get point of intersection with winding plane
	if( fabs(front - back) < 0.0001f ) {

		mid = end;

	} else {

		frac = front / (front - back);
		mid[ 0 ] = start[ 0 ] + (end[ 0 ] - start[ 0 ]) * frac;
		mid[ 1 ] = start[ 1 ] + (end[ 1 ] - start[ 1 ]) * frac;
		mid[ 2 ] = start[ 2 ] + (end[ 2 ] - start[ 2 ]) * frac;
	}

	return PointInside( windingPlane.Normal( ), mid, 0.0f );
}

bool Winding::RayIntersection( const Plane & windingPlane, const Vec3 & start, const Vec3 & dir, float &scale, bool backFaceCull ) const {

	bool side, lastside = false;
	Pluecker pl1, pl2;

	scale = 0.0f;
	pl1.FromRay( start, dir );

	for( int i = 0; i < numPoints; i++ ) {

		pl2.FromLine( p[ i ].ToVec3( ), p[ ( i + 1 ) % numPoints ].ToVec3( ) );
		side = pl1.PermutedInnerProduct( pl2 ) > 0.0f;

		if( i && side != lastside ) {

			return false;
		}

		lastside = side;
	}

	if( !backFaceCull || lastside ) {

		windingPlane.RayIntersection( start, dir, scale );
		return true;
	}

	return false;
}

bool GrowingWinding::ReAllocate( int n, bool keep ) {

	Vec5 * oldP = p;

	n = ( n + 3 ) & ~3;	// align up to multiple of four
	p = new Vec5[ n ];

	if( oldP ) {

		if( keep ) {

			memcpy( p, oldP, numPoints * sizeof( p[ 0 ] ) );
		}

		if( oldP != data ) {

			delete[ ] oldP;
		}
	}

	allocedSize = n;

	return true;
}

bool FixedWinding::ReAllocate( int n, bool keep ) {

	assert( n <= MAX_POINTS_ON_WINDING );

	if( n > MAX_POINTS_ON_WINDING ) {

		Common::Com_DPrintf("WARNING: FixedWinding -> MAX_POINTS_ON_WINDING overflowed\n");
		return false;
	}

	return true;
}

int FixedWinding::SplitInPlace( const Plane & plane, const float epsilon, FixedWinding * back ) {

	int				counts[ 3 ];
	double			dists[ MAX_POINTS_ON_WINDING + 4 ];
	byte			sides[ MAX_POINTS_ON_WINDING + 4 ];
	int				i, j;
	Vec5 *			p1, * p2;
	Vec5			mid;
	FixedWinding	out;

	CalculateSides( p, numPoints, plane, epsilon, dists, sides, counts );

	if( !counts[ SIDE_BACK ] ) {

		if( !counts[ SIDE_FRONT ] ) {

			return SIDE_ON;

		} else {

			return SIDE_FRONT;
		}
	}
	
	if( !counts[ SIDE_FRONT ] ) {

		return SIDE_BACK;
	}

	out.numPoints = 0;
	back->numPoints = 0;

	for( i = 0; i < numPoints; i++ ) {

		p1 = &p[ i ];

		if( !out.EnsureAlloced( out.numPoints + 1, true ) ) {

			return SIDE_FRONT;		// can't split -- fall back to original
		}

		if( !back->EnsureAlloced( back->numPoints + 1, true ) ) {

			return SIDE_FRONT;		// can't split -- fall back to original
		}

		if( sides[ i ] == SIDE_ON ) {

			out.p[ out.numPoints ] = *p1;
			out.numPoints++;
			back->p[ back->numPoints ] = *p1;
			back->numPoints++;
			continue;
		}
	
		if( sides[ i ] == SIDE_FRONT ) {

			out.p[ out.numPoints ] = *p1;
			out.numPoints++;
		}

		if( sides[ i ] == SIDE_BACK ) {

			back->p[ back->numPoints ] = *p1;
			back->numPoints++;
		}
		
		if( sides[ i + 1 ] == SIDE_ON || sides[ i + 1 ] == sides[ i ] ) {

			continue;
		}
			
		if( !out.EnsureAlloced( out.numPoints+1, true ) ) {

			return SIDE_FRONT;		// can't split -- fall back to original
		}

		if( !back->EnsureAlloced( back->numPoints+1, true ) ) {

			return SIDE_FRONT;		// can't split -- fall back to original
		}

		// generate a split point
		j = i + 1;
		if( j >= numPoints ) {

			p2 = &p[ 0 ];

		} else {

			p2 = &p[ j ];
		}

		double d = dists[ i ] / ( dists[ i ] - dists[ i + 1 ] );

		for( j = 0; j < 3; j++ ) {

			// avoid round off error when possible
			if( plane.Normal( )[ j ] == 1.0f ) {

				mid[ j ] = plane.Dist( );

			} else if( plane.Normal( )[ j ] == -1.0f ) {

				mid[ j ] = -plane.Dist( );

			} else {

				mid[ j ] = ( float )( ( double )( *p1 )[ j ] + d * ( ( double )( *p2 )[ j ] - ( double )( *p1 )[ j ] ) );
			}
		}

		mid.s = ( float )( ( double )p1->s + d * ( ( double )p2->s - ( double )p1->s ) );
		mid.t = ( float )( ( double )p1->t + d * ( ( double )p2->t - ( double )p1->t ) );
			
		out.p[out.numPoints] = mid;
		out.numPoints++;
		back->p[back->numPoints] = mid;
		back->numPoints++;
	}

	for( i = 0; i < out.numPoints; i++ ) {

		p[ i ] = out.p[ i ];
	}

	numPoints = out.numPoints;

	return SIDE_CROSS;
}

bool Plane2D::GetAxialBevel( const Plane2D & plane1, const Plane2D & plane2, const Vec2 & point ) {
	if( FLOATSIGNBITSET( plane1.d_a ) ^ FLOATSIGNBITSET( plane2.d_a ) ) {
		if( fabsf( plane1.d_a ) > 0.1f && fabsf( plane2.d_a ) > 0.1f ) {
			d_a = 0.0f;
			if( FLOATSIGNBITSET( plane1.d_b ) )
				d_b = -1.0f;
			else
				d_b = 1.0f;
			d_d = - ( point.d_x * d_a + point.d_y * d_b );
			return true;
		}
	}
	if( FLOATSIGNBITSET( plane1.d_b ) ^ FLOATSIGNBITSET( plane2.d_b ) ) {
		if( fabsf( plane1.d_b ) > 0.1f && fabsf( plane2.d_b ) > 0.1f ) {
			d_b = 0.0f;
			if( FLOATSIGNBITSET( plane1.d_a ) )
				d_a = -1.0f;
			else
				d_a = 1.0f;
			d_d = - ( point.d_x * d_a + point.d_y * d_b );
			return true;
		}
	}
	return false;
}

void Winding2D::ExpandForAxialBox( const Bounds2D & bounds ) {
	int i, j, numPlanes;
	Vec2 v;
	Plane2D planes[ MAX_POINTS_ON_WINDING_2D ], plane, bevel;
	// get planes for the edges and add bevels
	for( numPlanes = i = 0; i < d_numPoints; i++ ) {
		j = ( i + 1 ) % d_numPoints;
		if( ( d_points[ j ] - d_points[ i ] ).LengthSqr( ) < 0.01f )
			continue;
		plane.FromPoints( d_points[ i ], d_points[ j ], true );
		if( i ) {
			if( bevel.GetAxialBevel( planes[numPlanes-1], plane, d_points[i] ) )
				planes[ numPlanes++ ] = bevel;
		}
		assert( numPlanes < MAX_POINTS_ON_WINDING_2D );
		planes[ numPlanes++ ] = plane;
	}
	if( bevel.GetAxialBevel( planes[numPlanes-1], planes[0], d_points[0] ) )
		planes[ numPlanes++ ] = bevel;
	// expand the planes
	for( i = 0; i < numPlanes; i++ ) {
		v.d_x = bounds[ FLOATSIGNBITSET( planes[i][0] ) ][0];
		v.d_y = bounds[ FLOATSIGNBITSET( planes[i][1] ) ][1];
		planes[i][2] += v.d_x * planes[i][0] + v.d_y * planes[i][1];
	}
	// get intersection points of the planes
	for( d_numPoints = i = 0; i < numPlanes; i++ ) {
		if( planes[(i+numPlanes-1) % numPlanes].PlaneIntersection( planes[i], d_points[d_numPoints] ) )
			d_numPoints++;
	}
}

void Winding2D::Expand( const float d ) {
	Vec2 edgeNormals[ MAX_POINTS_ON_WINDING_2D ];
	for( int i = 0; i < d_numPoints; i++ ) {
		Vec2 & start = d_points[i];
		Vec2 & end = d_points[ (i+1) % d_numPoints ];
		edgeNormals[i].d_x = start.d_y - end.d_y;
		edgeNormals[i].d_y = end.d_x - start.d_x;
		edgeNormals[i].Normalize();
		edgeNormals[i] *= d;
	}
	for( int i = 0; i < d_numPoints; i++ )
		d_points[i] += edgeNormals[i] + edgeNormals[ (i+d_numPoints-1) % d_numPoints ];
}

int Winding2D::Split( const Plane2D & plane, const float epsilon, Winding2D ** front, Winding2D ** back ) const {
	float			dists[ MAX_POINTS_ON_WINDING_2D ];
	byte			sides[ MAX_POINTS_ON_WINDING_2D ];
	int				counts[ 3 ];
	float			dot;
	int				i, j;
	const Vec2 *	p1, *p2;
	Vec2			mid;
	Winding2D *	f;
	Winding2D *	b;
	int				maxpts;
	counts[ 0 ] = counts[ 1 ] = counts[ 2 ] = 0;
	// determine sides for each point
	for( i = 0; i < d_numPoints; i++ ) {
		dists[i] = dot = plane.Distance( d_points[i] );
		if( dot > epsilon )
			sides[i] = SIDE_FRONT;
		else if( dot < -epsilon )
			sides[i] = SIDE_BACK;
		else
			sides[i] = SIDE_ON;
		counts[sides[i]]++;
	}
	sides[i] = sides[0];
	dists[i] = dists[0];	
	*front = *back = NULL;
	// if nothing at the front of the clipping plane
	if( !counts[SIDE_FRONT] ) {
		*back = Copy();
		return SIDE_BACK;
	}
	// if nothing at the back of the clipping plane
	if( !counts[SIDE_BACK] ) {
		*front = Copy();
		return SIDE_FRONT;
	}
	maxpts = d_numPoints+4;	// cant use counts[0]+2 because of fp grouping errors
	*front = f = new Winding2D;
	*back = b = new Winding2D;		
	for( i = 0; i < d_numPoints; i++ ) {
		p1 = &d_points[i];		
		if( sides[i] == SIDE_ON ) {
			f->d_points[f->d_numPoints] = *p1;
			f->d_numPoints++;
			b->d_points[b->d_numPoints] = *p1;
			b->d_numPoints++;
			continue;
		}	
		if( sides[i] == SIDE_FRONT ) {
			f->d_points[f->d_numPoints] = *p1;
			f->d_numPoints++;
		}
		if( sides[i] == SIDE_BACK ) {
			b->d_points[b->d_numPoints] = *p1;
			b->d_numPoints++;
		}
		if( sides[i+1] == SIDE_ON || sides[i+1] == sides[i] )
			continue;			
		// generate a split point
		p2 = &d_points[ (i+1) % d_numPoints ];		
		// always calculate the split going from the same side
		// or minor epsilon issues can happen
		if( sides[i] == SIDE_FRONT ) {
			dot = dists[i] / ( dists[i] - dists[i+1] );
			for( j = 0; j < 2; j++ ) {
				// avoid round off error when possible
				if( plane[j] == 1.0f )
					mid[j] = -plane[2];
				else if( plane[j] == -1.0f )
					mid[j] = plane[2];
				else
					mid[j] = (*p1)[j] + dot * ((*p2)[j] - (*p1)[j]);
			}
		} else {
			dot = dists[i+1] / ( dists[i+1] - dists[i] );
			for( j = 0; j < 2; j++ ) {
				// avoid round off error when possible
				if( plane[j] == 1.0f )
					mid[j] = -plane[2];
				else if( plane[j] == -1.0f )
					mid[j] = plane[2];
				else
					mid[j] = (*p2)[j] + dot * ( (*p1)[j] - (*p2)[j] );
			}
		}
		f->d_points[f->d_numPoints] = mid;
		f->d_numPoints++;
		b->d_points[b->d_numPoints] = mid;
		b->d_numPoints++;
	}
	return SIDE_CROSS;
}

bool Winding2D::ClipInPlace( const Plane2D & plane, const float epsilon, const bool keepOn ) {
	int i, j, maxpts, newNumPoints;
	int sides[ MAX_POINTS_ON_WINDING_2D + 1 ], counts[ 3 ];
	float dot, dists[ MAX_POINTS_ON_WINDING_2D + 1 ];
	Vec2 *p1, *p2, mid, newPoints[ MAX_POINTS_ON_WINDING_2D + 4 ];
	counts[ SIDE_FRONT ] = counts[ SIDE_BACK ] = counts[ SIDE_ON ] = 0;
	for( i = 0; i < d_numPoints; i++ ) {
		dists[i] = dot = plane.Distance( d_points[i] );
		if( dot > epsilon )
			sides[i] = SIDE_FRONT;
		else if( dot < -epsilon )
			sides[i] = SIDE_BACK;
		else
			sides[i] = SIDE_ON;
		counts[sides[i]]++;
	}
	sides[i] = sides[0];
	dists[i] = dists[0];	
	// if the winding is on the plane and we should keep it
	if( keepOn && !counts[SIDE_FRONT] && !counts[SIDE_BACK] )
		return true;
	if( !counts[SIDE_FRONT] ) {
		d_numPoints = 0;
		return false;
	}
	if( !counts[SIDE_BACK] )
		return true;
	maxpts = d_numPoints + 4;		// cant use counts[0]+2 because of fp grouping errors
	newNumPoints = 0;
	for( i = 0; i < d_numPoints; i++ ) {
		p1 = &d_points[i];
		if( newNumPoints+1 > maxpts )
			return true;		// can't split -- fall back to original		
		if( sides[i] == SIDE_ON ) {
			newPoints[newNumPoints] = *p1;
			newNumPoints++;
			continue;
		}	
		if( sides[i] == SIDE_FRONT ) {
			newPoints[newNumPoints] = *p1;
			newNumPoints++;
		}
		if( sides[i+1] == SIDE_ON || sides[i+1] == sides[i] )
			continue;			
		if( newNumPoints+1 > maxpts )
			return true;		// can't split -- fall back to original
		// generate a split point
		p2 = &d_points[ (i+1) % d_numPoints ];		
		dot = dists[i] / ( dists[i] - dists[i+1] );
		for( j = 0; j < 2; j++ ) {
			// avoid round off error when possible
			if( plane[j] == 1.0f )
				mid[j] = -plane[2];
			else if( plane[j] == -1.0f )
				mid[j] = plane[2];
			else
				mid[j] = (*p1)[j] + dot * ((*p2)[j] - (*p1)[j]);
		}
		newPoints[ newNumPoints ] = mid;
		newNumPoints++;
	}
	if( newNumPoints >= MAX_POINTS_ON_WINDING_2D )
		return true;
	d_numPoints = newNumPoints;
	Common::Com_Memcpy( d_points, newPoints, NumSizeOf( Vec2, newNumPoints ) );
	return true;
}

Winding2D * Winding2D::Copy( ) const {
	Winding2D * w;
	w = new Winding2D;
	w->d_numPoints = d_numPoints;
	Common::Com_Memcpy( w->d_points, d_points, NumSizeOf( Vec2, d_numPoints ) );
	return w;
}

Winding2D * Winding2D::Reverse( ) const {
	Winding2D * w;
	w = new Winding2D;
	w->d_numPoints = d_numPoints;
	for( int i = 0; i < d_numPoints; i++ )
		w->d_points[ d_numPoints - i - 1 ] = d_points[i];
	return w;
}

float Winding2D::GetArea( ) const {
	Vec2 d1, d2;
	float total;
	total = 0.0f;
	for( int i = 2; i < d_numPoints; i++ ) {
		d1 = d_points[i-1] - d_points[0];
		d2 = d_points[i] - d_points[0];
		total += d1.d_x * d2.d_y - d1.d_y * d2.d_x;
	}
	return total * 0.5f;
}

Vec2 Winding2D::GetCenter( ) const {
	Vec2 center;
	center.Zero();
	for( int i = 0; i < d_numPoints; i++ )
		center += d_points[i];
	center *= ( 1.0f / d_numPoints );
	return center;
}

float Winding2D::GetRadius( const Vec2 & center ) const {
	float radius, r;
	Vec2 dir;
	radius = 0.0f;
	for( int i = 0; i < d_numPoints; i++ ) {
		dir = d_points[i] - center;
		r = dir * dir;
		if( r > radius )
			radius = r;
	}
	return sqrtf( radius );
}

void Winding2D::GetBounds( Bounds2D & bounds ) const {
	bounds.Clear( );
	for( int i = 0; i < d_numPoints; i++ )
		bounds.AddPoint( d_points[i] );
}

#define	EDGE_LENGTH		0.2f

bool Winding2D::IsTiny( ) const {
	int		i;
	float	len;
	Vec2	delta;
	int		edges;
	edges = 0;
	for( i = 0; i < d_numPoints; i++ ) {
		delta = d_points[ (i+1) % d_numPoints ] - d_points[i];
		len = delta.Length( );
		if( len > EDGE_LENGTH ) {
			if( ++edges == 3 )
				return false;
		}
	}
	return true;
}

bool Winding2D::IsHuge( ) const {
	for( int i = 0; i < d_numPoints; i++ ) {
		for( int j = 0; j < 2; j++ ) {
			if( d_points[i][j] <= MIN_WORLD_COORD || d_points[i][j] >= MAX_WORLD_COORD )
				return true;
		}
	}
	return false;
}

void Winding2D::Print( ) const {
	Common::Com_Printf( "Winding2D %i {\n", d_numPoints );
	for( int i = 0; i < d_numPoints; i++ )
		Common::Com_Printf( "\t( %6.1f, %6.1f )\n", d_points[i][0], d_points[i][1] );
	Common::Com_Printf( "}\n" );
}

float Winding2D::PlaneDistance( const Plane2D & plane ) const {
	int		i;
	float	d, min, max;
	min = INFINITY;
	max = -INFINITY;
	for( i = 0; i < d_numPoints; i++ ) {
		d = plane.Distance( d_points[i] );
		if( d < min ) {
			min = d;
			if( FLOATSIGNBITSET( min ) & FLOATSIGNBITNOTSET( max ) )
				return 0.0f;
		}
		if( d > max ) {
			max = d;
			if( FLOATSIGNBITSET( min ) & FLOATSIGNBITNOTSET( max ) )
				return 0.0f;
		}
	}
	if( FLOATSIGNBITNOTSET( min ) )
		return min;
	if( FLOATSIGNBITSET( max ) )
		return max;
	return 0.0f;
}

int Winding2D::PlaneSide( const Plane2D & plane, const float epsilon ) const {
	bool	front, back;
	int		i;
	float	d;
	front = false;
	back = false;
	for( i = 0; i < d_numPoints; i++ ) {
		d = plane.Distance( d_points[i] );
		if( d < -epsilon ) {
			if( front )
				return SIDE_CROSS;
			back = true;
			continue;
		}
		else if( d > epsilon ) {
			if( back )
				return SIDE_CROSS;
			front = true;
			continue;
		}
	}
	if( back )
		return SIDE_BACK;
	if( front )
		return SIDE_FRONT;
	return SIDE_ON;
}

bool Winding2D::PointInside( const Vec2 & point, const float epsilon ) const {
	int i;
	float d;
	Plane2D plane;
	for( i = 0; i < d_numPoints; i++ ) {
		plane.FromPoints( d_points[i], d_points[ (i+1) % d_numPoints ] );
		d = plane.Distance( point );
		if( d > epsilon )
			return false;
	}
	return true;
}

bool Winding2D::LineIntersection( const Vec2 & start, const Vec2 & end ) const {
	int i, numEdges;
	int sides[ MAX_POINTS_ON_WINDING_2D + 1 ], counts[ 3 ];
	float d1, d2, epsilon = 0.1f;
	Plane2D plane, edges[ 2 ];
	counts[ SIDE_FRONT ] = counts[ SIDE_BACK ] = counts[ SIDE_ON ] = 0;
	plane.FromPoints( start, end );
	for( i = 0; i < d_numPoints; i++ ) {
		d1 = plane.Distance( d_points[ i ] );
		if( d1 > epsilon )
			sides[i] = SIDE_FRONT;
		else if( d1 < -epsilon )
			sides[i] = SIDE_BACK;
		else
			sides[i] = SIDE_ON;
		counts[sides[i]]++;
	}
	sides[i] = sides[0];
	if( !counts[SIDE_FRONT] )
		return false;
	if( !counts[SIDE_BACK] )
		return false;
	numEdges = 0;
	for( i = 0; i < d_numPoints; i++ ) {
		if( sides[ i ] != sides[ i+1 ] && sides[ i+1 ] != SIDE_ON ) {
			edges[ numEdges++ ].FromPoints( d_points[i], d_points[ (i+1) % d_numPoints ] );
			if( numEdges >= 2 )
				break;
		}
	}
	if( numEdges < 2 )
		return false;
	d1 = edges[0].Distance( start );
	d2 = edges[0].Distance( end );
	if( FLOATSIGNBITNOTSET( d1 ) & FLOATSIGNBITNOTSET( d2 ) )
		return false;
	d1 = edges[1].Distance( start );
	d2 = edges[1].Distance( end );
	if( FLOATSIGNBITNOTSET( d1 ) & FLOATSIGNBITNOTSET( d2 ) )
		return false;
	return true;
}

bool Winding2D::RayIntersection( const Vec2 & start, const Vec2 & dir, float & scale1, float & scale2, int * edgeNums ) const {
	int i, numEdges, localEdgeNums[ 2 ];
	int sides[ MAX_POINTS_ON_WINDING_2D + 1 ], counts[ 3 ];
	float d1, d2, epsilon = 0.1f;
	Plane2D plane, edges[ 2 ];
	scale1 = scale2 = 0.0f;
	counts[SIDE_FRONT] = counts[SIDE_BACK] = counts[SIDE_ON] = 0;
	plane.FromVecs( start, dir );
	for( i = 0; i < d_numPoints; i++ ) {
		d1 = plane.Distance( d_points[ i ] );
		if( d1 > epsilon )
			sides[i] = SIDE_FRONT;
		else if( d1 < -epsilon )
			sides[i] = SIDE_BACK;
		else
			sides[i] = SIDE_ON;
		counts[sides[i]]++;
	}
	sides[i] = sides[0];
	if( !counts[SIDE_FRONT] )
		return false;
	if( !counts[SIDE_BACK] )
		return false;
	numEdges = 0;
	for( i = 0; i < d_numPoints; i++ ) {
		if( sides[i] != sides[i+1] && sides[i+1] != SIDE_ON ) {
			localEdgeNums[numEdges] = i;
			edges[numEdges++].FromPoints( d_points[i], d_points[ (i+1) % d_numPoints ] );
			if( numEdges >= 2 )
				break;
		}
	}
	if( numEdges < 2 )
		return false;
	d1 = edges[0].Distance( start );
	d2 = -( edges[0][0] * dir.d_x + edges[0][1] * dir.d_y );
	if( d2 == 0.0f ) {
		return false;
	}
	scale1 = d1 / d2;
	d1 = edges[1].Distance( start );
	d2 = -( edges[1][0] * dir.d_x + edges[1][1] * dir.d_y );
	if( d2 == 0.0f )
		return false;
	scale2 = d1 / d2;
	if( fabsf( scale1 ) > fabsf( scale2 ) ) {
		Swap( scale1, scale2 );
		Swap( localEdgeNums[0], localEdgeNums[1] );
	}
	if( edgeNums ) {
		edgeNums[0] = localEdgeNums[0];
		edgeNums[1] = localEdgeNums[1];
	}
	return true;
}
