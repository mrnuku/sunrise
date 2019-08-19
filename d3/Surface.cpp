#include "precompiled.h"
#pragma hdrstop

PolygonBuilder::PolygonBuilder( const CAListBase< Plane > & plane_set, int base_plane_index ) {

	basePlaneOK = false;
	BaseForPlane( plane_set[ base_plane_index ] );
	for( int i = 0; i < plane_set.Num( ); i++ ) {
		if( i == base_plane_index )
			continue;
		AddClippingPlane( plane_set[ i ] );
	}
	Build( );
}

bool PolygonBuilder::Build( ) {

	if( !basePlaneOK || !clippingPlanes.Num( ) )
		return false;
	int * windingVerts = ( int * )_alloca( MAX_POINTS_ON_WINDING * sizeof( int ) );
	for( int i = 0; i < clippingPlanes.Num( ); i++ ) {
		if( !FixedWinding::ClipInPlace( -clippingPlanes[ i ], ON_EPSILON, true ) ) {
			CAList< Plane, 32 > planeSet = clippingPlanes;
			planeSet.Insert( basePlane );
			Polytope polyTope;
			polyTope.FromPlanes( planeSet.Ptr( ), planeSet.Num( ) );
			for( int j = 0; j < polyTope.GetNumVertices( ); j++ ) {
				if( basePlane.Side( polyTope[ j ].xyz, ON_EPSILON ) == PLANESIDE_ON )
					FixedWinding::AddPoint( polyTope[ j ].xyz );
			}
			//Clear( );
			//return false;
			break;
		}
	}
	for( int i = 0; i < FixedWinding::GetNumPoints( ); i++ ) {
		int j;
		for( j = 0; j < verts.Num( ); j++ ) {
			if( verts[ j ].xyz.Compare( FixedWinding::operator[ ]( i ).ToVec3( ), POLYGON_VERTEX_EPSILON ) )
				break;
		}
		if( j >= verts.Num( ) ) {
			DrawVert newVert;
			newVert.Clear( );
			newVert.xyz = FixedWinding::operator[ ]( i ).ToVec3( );
			j = verts.Append( newVert );
		}
		windingVerts[ i ] = j;
	}
	for( int i = 2; i < FixedWinding::GetNumPoints( ); i++ ) {
		indexes.Append( windingVerts[ 0 ] );
		indexes.Append( windingVerts[ i - 1 ] );
		indexes.Append( windingVerts[ i ] );
	}
	GenerateEdgeIndexes( );
	return true;
}

bool PolygonBuilder::BuildCustom( const CAListBase< Plane > & plane_set, int base_plane_index, int plane_count, ... ) {
	va_list argPtr;
	Clear( );
	BaseForPlane( plane_set[ base_plane_index ] );
	va_start( argPtr, plane_count );
	for( int i = 0; i < plane_count; i++ )
		AddClippingPlane( plane_set[ va_arg( argPtr, int ) ] );
	va_end( argPtr );
	return Build( );
}

bool Surface::RayIntersection( const Vec3 &start, const Vec3 &dir, float &scale, Plane & outPlane, bool backFaceCull ) const {

	int i, i0, i1, i2, s0, s1, s2;
	float d, s;
	byte *sidedness;
	Pluecker rayPl, pl;
	Plane plane;

	sidedness = (byte *)_alloca( edges.Num( ) * sizeof(byte) );
	scale = INFINITY;

	rayPl.FromRay( start, dir );

	// ray sidedness for edges
	for( i = 0; i < edges.Num( ); i++ ) {

		pl.FromLine( verts[ edges[ i ].verts[ 1 ] ].xyz, verts[ edges[ i ].verts[ 0 ] ].xyz );
		d = pl.PermutedInnerProduct( rayPl );
		sidedness[ i ] = FLOATSIGNBITSET( d );
	}

	// test triangles
	for( i = 0; i < edgeIndexes.Num( ); i += 3 ) {

		i0 = edgeIndexes[ i + 0 ];
		i1 = edgeIndexes[ i + 1 ];
		i2 = edgeIndexes[ i + 2 ];
		s0 = sidedness[ abs( i0 ) ] ^ INTSIGNBITSET( i0 );
		s1 = sidedness[ abs( i1 ) ] ^ INTSIGNBITSET( i1 );
		s2 = sidedness[ abs( i2 ) ] ^ INTSIGNBITSET( i2 );

		if( s0 & s1 & s2 ) {

			plane.FromPoints( verts[ indexes[ i + 0 ] ].xyz, verts[ indexes[ i + 1 ] ].xyz, verts[ indexes[ i + 2 ] ].xyz );
			plane.RayIntersection( start, dir, s );

			if( fabs( s ) < fabs( scale ) ) {

				outPlane = plane;
				scale = s;
			}

		} else if( !backFaceCull && !( s0 | s1 | s2 ) ) {

			plane.FromPoints( verts[ indexes[ i + 0 ] ].xyz, verts[ indexes[ i + 1 ] ].xyz, verts[ indexes[ i + 2 ] ].xyz );
			plane.RayIntersection( start, dir, s );

			if( fabs( s ) < fabs( scale ) ) {
				outPlane = plane;
				scale = s;
			}
		}
	}

	if( fabs( scale ) < INFINITY ) {

		return true;
	}

	return false;
}

int Surface::PlaneSide( const Plane & plane, const float epsilon ) const {

	bool front = false;
	bool back = false;

	for( int i = 0; i < verts.Num( ); i++ ) {

		float d = plane.Distance( verts[ i ].xyz );

		if( d < -epsilon ) {

			if( front ) {

				return PLANESIDE_CROSS;
			}

			back = true;
			continue;

		} else if( d > epsilon ) {

			if( back ) {

				return PLANESIDE_CROSS;
			}

			front = true;
			continue;
		}
	}

	if( back ) {

		return PLANESIDE_BACK;
	}

	if( front ) {

		return PLANESIDE_FRONT;
	}

	return PLANESIDE_ON;
}

INLINE int UpdateVertexIndex( int vertexIndexNum[ 2 ], int *vertexRemap, int *vertexCopyIndex, int vertNum ) {

	int s = INTSIGNBITSET( vertexRemap[ vertNum ] );
	vertexIndexNum[ 0 ] = vertexRemap[ vertNum ];
	vertexRemap[ vertNum ] = vertexIndexNum[ s ];
	vertexIndexNum[ 1 ] += s;
	vertexCopyIndex[ vertexRemap[ vertNum ] ] = vertNum;
	return vertexRemap[ vertNum ];
}

int Surface::Split( const Plane &plane, const float epsilon, Surface **front, Surface **back, int *frontOnPlaneEdges, int *backOnPlaneEdges ) const {

	float *			dists;
	float			f;
	byte *			sides;
	int				counts[ 3 ];
	int *			edgeSplitVertex;
	int				numEdgeSplitVertexes;
	int *			vertexRemap[ 2 ];
	int				vertexIndexNum[ 2 ][ 2 ];
	int *			vertexCopyIndex[ 2 ];
	int *			indexPtr[ 2 ];
	int				indexNum[ 2 ];
	int *			index;
	int *			onPlaneEdges[ 2 ];
	int				numOnPlaneEdges[ 2 ];
	int				maxOnPlaneEdges;
	int				i;
	Surface *		surface[ 2 ];
	DrawVert		v;

	dists = ( float * )_alloca( verts.Num( ) * sizeof( float ) );
	sides = ( byte * )_alloca( verts.Num( ) * sizeof( byte ) );

	counts[ 0 ] = counts[ 1 ] = counts[ 2 ] = 0;

	// determine side for each vertex
	for( i = 0; i < verts.Num( ); i++ ) {

		dists[ i ] = f = plane.Distance( verts[ i ].xyz );

		if( f > epsilon ) {

			sides[ i ] = SIDE_FRONT;

		} else if( f < -epsilon ) {

			sides[ i ] = SIDE_BACK;

		} else {

			sides[ i ] = SIDE_ON;

		}

		counts[ sides[ i ] ]++;
	}
	
	*front = *back = NULL;

	// if coplanar, put on the front side if the normals match
	if( !counts[ SIDE_FRONT ] && !counts[ SIDE_BACK ] ) {

		f = ( verts[ indexes[ 1 ] ].xyz - verts[ indexes[ 0 ] ].xyz ).Cross( verts[ indexes[ 0 ] ].xyz - verts[ indexes[ 2 ] ].xyz ) * plane.Normal( );

		if( FLOATSIGNBITSET( f ) ) {

			*back = new Surface( *this );
			return SIDE_BACK;

		} else {

			*front = new Surface( *this );
			return SIDE_FRONT;
		}
	}

	// if nothing at the front of the clipping plane
	if( !counts[ SIDE_FRONT ] ) {

		*back = new Surface( *this );
		return SIDE_BACK;
	}

	// if nothing at the back of the clipping plane
	if( !counts[ SIDE_BACK ] ) {

		*front = new Surface( *this );
		return SIDE_FRONT;
	}

	// allocate front and back surface
	*front = surface[ 0 ] = new Surface( );
	*back = surface[ 1 ] = new Surface( );

	edgeSplitVertex = ( int * )_alloca( edges.Num( ) * sizeof( int ) );
	numEdgeSplitVertexes = 0;

	maxOnPlaneEdges = 4 * counts[ SIDE_ON ];
	counts[ SIDE_FRONT ] = counts[ SIDE_BACK ] = counts[ SIDE_ON ] = 0;

	// split edges
	for( i = 0; i < edges.Num( ); i++ ) {

		int v0 = edges[ i ].verts[ 0 ];
		int v1 = edges[ i ].verts[ 1 ];
		int sidesOr = ( sides[ v0 ] | sides[ v1 ] );

		// if both vertexes are on the same side or one is on the clipping plane
		if( !( sides[ v0 ] ^ sides[ v1 ] ) || ( sidesOr & SIDE_ON ) ) {

			edgeSplitVertex[ i ] = -1;
			counts[ sidesOr & SIDE_BACK ]++;
			counts[ SIDE_ON ] += ( sidesOr & SIDE_ON ) >> 1;

		} else {

			f = dists[ v0 ] / ( dists[ v0 ] - dists[ v1 ] );
			v.LerpAll( verts[ v0 ], verts[ v1 ], f );
			edgeSplitVertex[ i ] = numEdgeSplitVertexes++;
			surface[ 0 ]->verts.Append( v );
			surface[ 1 ]->verts.Append( v );
		}
	}

	// each edge is shared by at most two triangles, as such there can never be more indexes than twice the number of edges
	surface[ 0 ]->indexes.Resize( ( ( counts[ SIDE_FRONT ] + counts[ SIDE_ON ] ) * 2 ) + ( numEdgeSplitVertexes * 4 ) );
	surface[ 1 ]->indexes.Resize( ( ( counts[ SIDE_BACK ] + counts[ SIDE_ON ] ) * 2 ) + ( numEdgeSplitVertexes * 4 ) );

	// allocate indexes to construct the triangle indexes for the front and back surface
	vertexRemap[ 0 ] = ( int * )_alloca( verts.Num( ) * sizeof( int ) );
	Common::Com_Memset( vertexRemap[ 0 ], -1, verts.Num( ) * sizeof( int ) );

	vertexRemap[ 1 ] = ( int * )_alloca( verts.Num( ) * sizeof( int ) );
	Common::Com_Memset( vertexRemap[ 1 ], -1, verts.Num( ) * sizeof( int ) );

	vertexCopyIndex[ 0 ] = ( int * )_alloca( ( numEdgeSplitVertexes + verts.Num( ) ) * sizeof( int ) );
	vertexCopyIndex[ 1 ] = ( int * )_alloca( ( numEdgeSplitVertexes + verts.Num( ) ) * sizeof( int ) );

	vertexIndexNum[ 0 ][ 0 ] = vertexIndexNum[ 1 ][ 0 ] = 0;
	vertexIndexNum[ 0 ][ 1 ] = vertexIndexNum[ 1 ][ 1 ] = numEdgeSplitVertexes;

	indexPtr[ 0 ] = surface[ 0 ]->indexes.Ptr( );
	indexPtr[ 1 ] = surface[ 1 ]->indexes.Ptr( );
	indexNum[ 0 ] = surface[ 0 ]->indexes.Num( );
	indexNum[ 1 ] = surface[ 1 ]->indexes.Num( );

	maxOnPlaneEdges += 4 * numEdgeSplitVertexes;
	// allocate one more in case no triangles are actually split which may happen for a disconnected surface
	onPlaneEdges[ 0 ] = ( int * )_alloca( ( maxOnPlaneEdges + 1 ) * sizeof( int ) );
	onPlaneEdges[ 1 ] = ( int * )_alloca( ( maxOnPlaneEdges + 1 ) * sizeof( int ) );
	numOnPlaneEdges[ 0 ] = numOnPlaneEdges[ 1 ] = 0;

	// split surface triangles
	for( i = 0; i < edgeIndexes.Num( ); i += 3 ) {

		int e0, e1, e2, v0, v1, v2, s, n;

		e0 = abs( edgeIndexes[ i + 0 ] );
		e1 = abs( edgeIndexes[ i + 1 ] );
		e2 = abs( edgeIndexes[ i + 2 ] );

		v0 = indexes[ i + 0 ];
		v1 = indexes[ i + 1 ];
		v2 = indexes[ i + 2 ];

		switch( ( INTSIGNBITSET( edgeSplitVertex[ e0 ] ) | ( INTSIGNBITSET( edgeSplitVertex[ e1 ] ) << 1 ) | ( INTSIGNBITSET( edgeSplitVertex[ e2 ] ) << 2 ) ) ^ 7 ) {

			case 0:	// no edges split

				if( ( sides[ v0 ] & sides[ v1 ] & sides[ v2 ] ) & SIDE_ON ) {

					// coplanar
					f = ( verts[ v1 ].xyz - verts[ v0 ].xyz ).Cross( verts[ v0 ].xyz - verts[ v2 ].xyz ) * plane.Normal( );
					s = FLOATSIGNBITSET( f );

				} else {

					s = ( sides[ v0 ] | sides[ v1 ] | sides[ v2 ] ) & SIDE_BACK;
				}

				n = indexNum[ s ];

				onPlaneEdges[ s ][ numOnPlaneEdges[ s ] ] = n;
				numOnPlaneEdges[ s ] += ( sides[ v0 ] & sides[ v1 ] ) >> 1;

				onPlaneEdges[ s ][ numOnPlaneEdges[ s ] ] = n + 1;
				numOnPlaneEdges[ s ] += ( sides[ v1 ] & sides[ v2 ] ) >> 1;

				onPlaneEdges[ s ][ numOnPlaneEdges[ s ] ] = n + 2;
				numOnPlaneEdges[ s ] += ( sides[ v2 ] & sides[ v0 ] ) >> 1;

				index = indexPtr[ s ];

				index[ n++ ] = UpdateVertexIndex( vertexIndexNum[ s ], vertexRemap[ s ], vertexCopyIndex[ s ], v0 );
				index[ n++ ] = UpdateVertexIndex( vertexIndexNum[ s ], vertexRemap[ s ], vertexCopyIndex[ s ], v1 );
				index[ n++ ] = UpdateVertexIndex( vertexIndexNum[ s ], vertexRemap[ s ], vertexCopyIndex[ s ], v2 );

				indexNum[ s ] = n;

				break;

			case 1:	// first edge split

				s = sides[ v0 ] & SIDE_BACK;
				n = indexNum[ s ];

				onPlaneEdges[ s ][ numOnPlaneEdges[ s ]++ ] = n;

				index = indexPtr[ s ];

				index[ n++ ] = edgeSplitVertex[ e0 ];
				index[ n++ ] = UpdateVertexIndex( vertexIndexNum[ s ], vertexRemap[ s ], vertexCopyIndex[ s ], v2 );
				index[ n++ ] = UpdateVertexIndex( vertexIndexNum[ s ], vertexRemap[ s ], vertexCopyIndex[ s ], v0 );
				indexNum[ s ] = n;

				s ^= 1;
				n = indexNum[ s ];

				onPlaneEdges[ s ][ numOnPlaneEdges[ s ]++ ] = n;
				index = indexPtr[ s ];

				index[ n++ ] = UpdateVertexIndex( vertexIndexNum[ s ], vertexRemap[ s ], vertexCopyIndex[ s ], v2 );
				index[ n++ ] = edgeSplitVertex[ e0 ];
				index[ n++ ] = UpdateVertexIndex( vertexIndexNum[ s ], vertexRemap[ s ], vertexCopyIndex[ s ], v1 );

				indexNum[ s ] = n;

				break;

			case 2:	// second edge split

				s = sides[ v1 ] & SIDE_BACK;
				n = indexNum[ s ];

				onPlaneEdges[ s ][ numOnPlaneEdges[ s ]++ ] = n;

				index = indexPtr[ s ];

				index[ n++ ] = edgeSplitVertex[ e1 ];
				index[ n++ ] = UpdateVertexIndex( vertexIndexNum[ s ], vertexRemap[ s ], vertexCopyIndex[ s ], v0 );
				index[ n++ ] = UpdateVertexIndex( vertexIndexNum[ s ], vertexRemap[ s ], vertexCopyIndex[ s ], v1 );
				indexNum[ s ] = n;

				s ^= 1;
				n = indexNum[ s ];

				onPlaneEdges[ s ][ numOnPlaneEdges[ s ]++ ] = n;
				index = indexPtr[ s ];

				index[ n++ ] = UpdateVertexIndex( vertexIndexNum[ s ], vertexRemap[ s ], vertexCopyIndex[ s ], v0 );
				index[ n++ ] = edgeSplitVertex[ e1 ];
				index[ n++ ] = UpdateVertexIndex( vertexIndexNum[ s ], vertexRemap[ s ], vertexCopyIndex[ s ], v2 );

				indexNum[ s ] = n;

				break;

			case 3:	// first and second edge split

				s = sides[ v1 ] & SIDE_BACK;
				n = indexNum[ s ];

				onPlaneEdges[ s ][ numOnPlaneEdges[ s ]++ ] = n;

				index = indexPtr[ s ];

				index[ n++ ] = edgeSplitVertex[ e1 ];
				index[ n++ ] = edgeSplitVertex[ e0 ];
				index[ n++ ] = UpdateVertexIndex( vertexIndexNum[ s ], vertexRemap[ s ], vertexCopyIndex[ s ], v1 );
				indexNum[ s ] = n;

				s ^= 1;
				n = indexNum[ s ];

				onPlaneEdges[ s ][ numOnPlaneEdges[ s ]++ ] = n;
				index = indexPtr[ s ];

				index[ n++ ] = edgeSplitVertex[ e0 ];
				index[ n++ ] = edgeSplitVertex[ e1 ];
				index[ n++ ] = UpdateVertexIndex( vertexIndexNum[ s ], vertexRemap[ s ], vertexCopyIndex[ s ], v0 );

				index[ n++ ] = edgeSplitVertex[ e1 ];
				index[ n++ ] = UpdateVertexIndex( vertexIndexNum[ s ], vertexRemap[ s ], vertexCopyIndex[ s ], v2 );
				index[ n++ ] = UpdateVertexIndex( vertexIndexNum[ s ], vertexRemap[ s ], vertexCopyIndex[ s ], v0 );

				indexNum[ s ] = n;

				break;

			case 4:	// third edge split

				s = sides[ v2 ] & SIDE_BACK;
				n = indexNum[ s ];

				onPlaneEdges[ s ][ numOnPlaneEdges[ s ]++ ] = n;

				index = indexPtr[ s ];

				index[ n++ ] = edgeSplitVertex[ e2 ];
				index[ n++ ] = UpdateVertexIndex( vertexIndexNum[ s ], vertexRemap[ s ], vertexCopyIndex[ s ], v1 );
				index[ n++ ] = UpdateVertexIndex( vertexIndexNum[ s ], vertexRemap[ s ], vertexCopyIndex[ s ], v2 );
				indexNum[ s ] = n;

				s ^= 1;
				n = indexNum[ s ];

				onPlaneEdges[ s ][ numOnPlaneEdges[ s ]++ ] = n;
				index = indexPtr[ s ];

				index[ n++ ] = UpdateVertexIndex( vertexIndexNum[ s ], vertexRemap[ s ], vertexCopyIndex[ s ], v1 );
				index[ n++ ] = edgeSplitVertex[ e2 ];
				index[ n++ ] = UpdateVertexIndex( vertexIndexNum[ s ], vertexRemap[ s ], vertexCopyIndex[ s ], v0 );

				indexNum[ s ] = n;

				break;

			case 5:	// first and third edge split

				s = sides[ v0 ] & SIDE_BACK;
				n = indexNum[ s ];

				onPlaneEdges[ s ][ numOnPlaneEdges[ s ]++ ] = n;

				index = indexPtr[ s ];

				index[ n++ ] = edgeSplitVertex[ e0 ];
				index[ n++ ] = edgeSplitVertex[ e2 ];
				index[ n++ ] = UpdateVertexIndex( vertexIndexNum[ s ], vertexRemap[ s ], vertexCopyIndex[ s ], v0 );
				indexNum[ s ] = n;

				s ^= 1;
				n = indexNum[ s ];

				onPlaneEdges[ s ][ numOnPlaneEdges[ s ]++ ] = n;
				index = indexPtr[ s ];

				index[ n++ ] = edgeSplitVertex[ e2 ];
				index[ n++ ] = edgeSplitVertex[ e0 ];
				index[ n++ ] = UpdateVertexIndex( vertexIndexNum[ s ], vertexRemap[ s ], vertexCopyIndex[ s ], v1 );

				index[ n++ ] = UpdateVertexIndex( vertexIndexNum[ s ], vertexRemap[ s ], vertexCopyIndex[ s ], v1 );
				index[ n++ ] = UpdateVertexIndex( vertexIndexNum[ s ], vertexRemap[ s ], vertexCopyIndex[ s ], v2 );
				index[ n++ ] = edgeSplitVertex[ e2 ];

				indexNum[ s ] = n;

				break;

			case 6:	// second and third edge split

				s = sides[ v2 ] & SIDE_BACK;
				n = indexNum[ s ];

				onPlaneEdges[ s ][ numOnPlaneEdges[ s ]++ ] = n;

				index = indexPtr[ s ];

				index[ n++ ] = edgeSplitVertex[ e2 ];
				index[ n++ ] = edgeSplitVertex[ e1 ];
				index[ n++ ] = UpdateVertexIndex( vertexIndexNum[ s ], vertexRemap[ s ], vertexCopyIndex[ s ], v2 );
				indexNum[ s ] = n;

				s ^= 1;
				n = indexNum[ s ];

				onPlaneEdges[ s ][ numOnPlaneEdges[ s ]++ ] = n;
				index = indexPtr[ s ];

				index[ n++ ] = edgeSplitVertex[ e1 ];
				index[ n++ ] = edgeSplitVertex[ e2 ];
				index[ n++ ] = UpdateVertexIndex( vertexIndexNum[ s ], vertexRemap[ s ], vertexCopyIndex[ s ], v1 );

				index[ n++ ] = UpdateVertexIndex( vertexIndexNum[ s ], vertexRemap[ s ], vertexCopyIndex[ s ], v0 );
				index[ n++ ] = UpdateVertexIndex( vertexIndexNum[ s ], vertexRemap[ s ], vertexCopyIndex[ s ], v1 );
				index[ n++ ] = edgeSplitVertex[ e2 ];

				indexNum[ s ] = n;

				break;
		}
	}

	surface[ 0 ]->indexes.SetNum( indexNum[ 0 ], false );
	surface[ 1 ]->indexes.SetNum( indexNum[ 1 ], false );

	// copy vertexes
	surface[ 0 ]->verts.SetNum( vertexIndexNum[ 0 ][ 1 ], false );
	index = vertexCopyIndex[ 0 ];

	for( i = numEdgeSplitVertexes; i < surface[ 0 ]->verts.Num( ); i++ ) {

		surface[ 0 ]->verts[ i ] = verts[index[ i ]];
	}

	surface[ 1 ]->verts.SetNum( vertexIndexNum[ 1 ][ 1 ], false );
	index = vertexCopyIndex[ 1 ];

	for( i = numEdgeSplitVertexes; i < surface[ 1 ]->verts.Num( ); i++ ) {

		surface[ 1 ]->verts[ i ] = verts[index[ i ]];
	}

	// generate edge indexes
	surface[ 0 ]->GenerateEdgeIndexes( );
	surface[ 1 ]->GenerateEdgeIndexes( );

	if( frontOnPlaneEdges ) {

		memcpy( frontOnPlaneEdges, onPlaneEdges[ 0 ], numOnPlaneEdges[ 0 ] * sizeof( int ) );
		frontOnPlaneEdges[ numOnPlaneEdges[ 0 ]] = -1;
	}

	if( backOnPlaneEdges ) {

		memcpy( backOnPlaneEdges, onPlaneEdges[ 1 ], numOnPlaneEdges[ 1 ] * sizeof( int ) );
		backOnPlaneEdges[ numOnPlaneEdges[ 1 ]] = -1;
	}

	return SIDE_CROSS;
}

bool Surface::ClipInPlace( const Plane &plane, const float epsilon, const bool keepOn ) {

	float *				dists;
	float				f;
	byte *				sides;
	int					counts[ 3 ];
	int					i;
	int *				edgeSplitVertex;
	int *				vertexRemap;
	int					vertexIndexNum[ 2 ];
	int *				vertexCopyIndex;
	int *				indexPtr;
	int					indexNum;
	int					numEdgeSplitVertexes;
	DrawVert			v;
	List< DrawVert >	newVerts;
	List< int >			newIndexes;

	dists = ( float * )_alloca( verts.Num( ) * sizeof( float ) );
	sides = ( byte * )_alloca( verts.Num( ) * sizeof( byte ) );

	counts[ 0 ] = counts[ 1 ] = counts[ 2 ] = 0;

	// determine side for each vertex
	for( i = 0; i < verts.Num( ); i++ ) {

		dists[ i ] = f = plane.Distance( verts[ i ].xyz );

		if( f > epsilon ) {

			sides[ i ] = SIDE_FRONT;

		} else if( f < -epsilon ) {

			sides[ i ] = SIDE_BACK;

		} else {

			sides[ i ] = SIDE_ON;
		}

		counts[ sides[ i ] ]++;
	}
	
	// if coplanar, put on the front side if the normals match
	if( !counts[ SIDE_FRONT ] && !counts[ SIDE_BACK ] ) {

		f = ( verts[ indexes[ 1 ] ].xyz - verts[ indexes[ 0 ] ].xyz ).Cross( verts[ indexes[ 0 ] ].xyz - verts[ indexes[ 2 ] ].xyz ) * plane.Normal( );

		if( FLOATSIGNBITSET( f ) ) {

			Clear( );
			return false;

		} else {

			return true;
		}
	}

	// if nothing at the front of the clipping plane
	if( !counts[ SIDE_FRONT ] ) {

		Clear( );
		return false;
	}

	// if nothing at the back of the clipping plane
	if( !counts[ SIDE_BACK ] ) {

		return true;
	}

	edgeSplitVertex = ( int * )_alloca( edges.Num( ) * sizeof( int ) );
	numEdgeSplitVertexes = 0;

	counts[ SIDE_FRONT ] = counts[ SIDE_BACK ] = 0;

	// split edges
	for( i = 0; i < edges.Num( ); i++ ) {

		int v0 = edges[ i ].verts[ 0 ];
		int v1 = edges[ i ].verts[ 1 ];

		// if both vertexes are on the same side or one is on the clipping plane
		if( !( sides[ v0 ] ^ sides[ v1 ] ) || ( ( sides[ v0 ] | sides[ v1 ] ) & SIDE_ON ) ) {

			edgeSplitVertex[ i ] = -1;
			counts[(sides[ v0 ]|sides[ v1 ]) & SIDE_BACK]++;

		} else {

			f = dists[ v0 ] / ( dists[ v0 ] - dists[ v1 ] );
			v.LerpAll( verts[ v0 ], verts[ v1 ], f );
			edgeSplitVertex[ i ] = numEdgeSplitVertexes++;
			newVerts.Append( v );
		}
	}

	// each edge is shared by at most two triangles, as such there can never be
	// more indexes than twice the number of edges
	newIndexes.Resize( ( counts[ SIDE_FRONT ] << 1 ) + ( numEdgeSplitVertexes << 2 ) );

	// allocate indexes to construct the triangle indexes for the front and back surface
	vertexRemap = ( int * )_alloca( verts.Num( ) * sizeof( int ) );
	Common::Com_Memset( vertexRemap, -1, verts.Num( ) * sizeof( int ) );

	vertexCopyIndex = ( int * )_alloca( ( numEdgeSplitVertexes + verts.Num( ) ) * sizeof( int ) );

	vertexIndexNum[ 0 ] = 0;
	vertexIndexNum[ 1 ] = numEdgeSplitVertexes;

	indexPtr = newIndexes.Ptr( );
	indexNum = newIndexes.Num( );

	// split surface triangles
	for( i = 0; i < edgeIndexes.Num( ); i += 3 ) {

		int e0, e1, e2, v0, v1, v2;

		e0 = abs( edgeIndexes[ i + 0 ] );
		e1 = abs( edgeIndexes[ i + 1 ] );
		e2 = abs( edgeIndexes[ i + 2 ] );

		v0 = indexes[ i + 0 ];
		v1 = indexes[ i + 1 ];
		v2 = indexes[ i + 2 ];

		switch( ( INTSIGNBITSET( edgeSplitVertex[ e0 ] ) | ( INTSIGNBITSET( edgeSplitVertex[ e1 ] ) << 1 ) | ( INTSIGNBITSET( edgeSplitVertex[ e2 ] ) << 2 ) ) ^ 7 ) {

			case 0:

				if( ( sides[ v0 ] | sides[ v1 ] | sides[ v2 ] ) & SIDE_BACK ) {

					break;
				}

				if( ( sides[ v0 ] & sides[ v1 ] & sides[ v2 ] ) & SIDE_ON ) {

					// coplanar
					if( !keepOn ) {

						break;
					}

					f = ( verts[ v1 ].xyz - verts[ v0 ].xyz ).Cross( verts[ v0 ].xyz - verts[ v2 ].xyz ) * plane.Normal( );

					if( FLOATSIGNBITSET( f ) ) {

						break;
					}
				}

				indexPtr[ indexNum++ ] = UpdateVertexIndex( vertexIndexNum, vertexRemap, vertexCopyIndex, v0 );
				indexPtr[ indexNum++ ] = UpdateVertexIndex( vertexIndexNum, vertexRemap, vertexCopyIndex, v1 );
				indexPtr[ indexNum++ ] = UpdateVertexIndex( vertexIndexNum, vertexRemap, vertexCopyIndex, v2 );

				break;

			case 1:	// first edge split

				if( !( sides[ v0 ] & SIDE_BACK ) ) {

					indexPtr[ indexNum++ ] = UpdateVertexIndex( vertexIndexNum, vertexRemap, vertexCopyIndex, v0 );
					indexPtr[ indexNum++ ] = edgeSplitVertex[ e0 ];
					indexPtr[ indexNum++ ] = UpdateVertexIndex( vertexIndexNum, vertexRemap, vertexCopyIndex, v2 );

				} else {

					indexPtr[ indexNum++ ] = edgeSplitVertex[ e0 ];
					indexPtr[ indexNum++ ] = UpdateVertexIndex( vertexIndexNum, vertexRemap, vertexCopyIndex, v1 );
					indexPtr[ indexNum++ ] = UpdateVertexIndex( vertexIndexNum, vertexRemap, vertexCopyIndex, v2 );
				}

				break;

			case 2:	// second edge split

				if( !( sides[ v1 ] & SIDE_BACK ) ) {

					indexPtr[ indexNum++ ] = UpdateVertexIndex( vertexIndexNum, vertexRemap, vertexCopyIndex, v1 );
					indexPtr[ indexNum++ ] = edgeSplitVertex[ e1 ];
					indexPtr[ indexNum++ ] = UpdateVertexIndex( vertexIndexNum, vertexRemap, vertexCopyIndex, v0 );

				} else {

					indexPtr[ indexNum++ ] = edgeSplitVertex[ e1 ];
					indexPtr[ indexNum++ ] = UpdateVertexIndex( vertexIndexNum, vertexRemap, vertexCopyIndex, v2 );
					indexPtr[ indexNum++ ] = UpdateVertexIndex( vertexIndexNum, vertexRemap, vertexCopyIndex, v0 );
				}

				break;

			case 3:	// first and second edge split

				if( !( sides[ v1 ] & SIDE_BACK ) ) {

					indexPtr[ indexNum++ ] = UpdateVertexIndex( vertexIndexNum, vertexRemap, vertexCopyIndex, v1 );
					indexPtr[ indexNum++ ] = edgeSplitVertex[ e1 ];
					indexPtr[ indexNum++ ] = edgeSplitVertex[ e0 ];

				} else {

					indexPtr[ indexNum++ ] = UpdateVertexIndex( vertexIndexNum, vertexRemap, vertexCopyIndex, v0 );
					indexPtr[ indexNum++ ] = edgeSplitVertex[ e0 ];
					indexPtr[ indexNum++ ] = edgeSplitVertex[ e1 ];

					indexPtr[ indexNum++ ] = edgeSplitVertex[ e1 ];
					indexPtr[ indexNum++ ] = UpdateVertexIndex( vertexIndexNum, vertexRemap, vertexCopyIndex, v2 );
					indexPtr[ indexNum++ ] = UpdateVertexIndex( vertexIndexNum, vertexRemap, vertexCopyIndex, v0 );
				}

				break;

			case 4:	// third edge split

				if( !( sides[ v2 ] & SIDE_BACK ) ) {

					indexPtr[ indexNum++ ] = UpdateVertexIndex( vertexIndexNum, vertexRemap, vertexCopyIndex, v2 );
					indexPtr[ indexNum++ ] = edgeSplitVertex[ e2 ];
					indexPtr[ indexNum++ ] = UpdateVertexIndex( vertexIndexNum, vertexRemap, vertexCopyIndex, v1 );

				} else {

					indexPtr[ indexNum++ ] = edgeSplitVertex[ e2 ];
					indexPtr[ indexNum++ ] = UpdateVertexIndex( vertexIndexNum, vertexRemap, vertexCopyIndex, v0 );
					indexPtr[ indexNum++ ] = UpdateVertexIndex( vertexIndexNum, vertexRemap, vertexCopyIndex, v1 );
				}

				break;

			case 5:	// first and third edge split

				if( !( sides[ v0 ] & SIDE_BACK ) ) {

					indexPtr[ indexNum++ ] = UpdateVertexIndex( vertexIndexNum, vertexRemap, vertexCopyIndex, v0 );
					indexPtr[ indexNum++ ] = edgeSplitVertex[ e0 ];
					indexPtr[ indexNum++ ] = edgeSplitVertex[ e2 ];

				} else {

					indexPtr[ indexNum++ ] = edgeSplitVertex[ e0 ];
					indexPtr[ indexNum++ ] = UpdateVertexIndex( vertexIndexNum, vertexRemap, vertexCopyIndex, v1 );
					indexPtr[ indexNum++ ] = edgeSplitVertex[ e2 ];

					indexPtr[ indexNum++ ] = UpdateVertexIndex( vertexIndexNum, vertexRemap, vertexCopyIndex, v1 );
					indexPtr[ indexNum++ ] = UpdateVertexIndex( vertexIndexNum, vertexRemap, vertexCopyIndex, v2 );
					indexPtr[ indexNum++ ] = edgeSplitVertex[ e2 ];
				}

				break;

			case 6:	// second and third edge split
				if( !( sides[ v2 ] & SIDE_BACK ) ) {

					indexPtr[ indexNum++ ] = UpdateVertexIndex( vertexIndexNum, vertexRemap, vertexCopyIndex, v2 );
					indexPtr[ indexNum++ ] = edgeSplitVertex[ e2 ];
					indexPtr[ indexNum++ ] = edgeSplitVertex[ e1 ];

				} else {

					indexPtr[ indexNum++ ] = edgeSplitVertex[ e2 ];
					indexPtr[ indexNum++ ] = UpdateVertexIndex( vertexIndexNum, vertexRemap, vertexCopyIndex, v1 );
					indexPtr[ indexNum++ ] = edgeSplitVertex[ e1 ];

					indexPtr[ indexNum++ ] = UpdateVertexIndex( vertexIndexNum, vertexRemap, vertexCopyIndex, v0 );
					indexPtr[ indexNum++ ] = UpdateVertexIndex( vertexIndexNum, vertexRemap, vertexCopyIndex, v1 );
					indexPtr[ indexNum++ ] = edgeSplitVertex[ e2 ];
				}

				break;

		}
	}

	newIndexes.SetNum( indexNum, false );

	// copy vertexes
	newVerts.SetNum( vertexIndexNum[ 1 ], false );

	for( i = numEdgeSplitVertexes; i < newVerts.Num( ); i++ ) {

		newVerts[ i ] = verts[vertexCopyIndex[ i ]];
	}

	// copy back to this surface
	indexes = newIndexes;
	verts = newVerts;

	GenerateEdgeIndexes( );

	return true;
}

bool Surface::IsConnected( void ) const {

	int i, j, numIslands, numTris;
	int queueStart, queueEnd;
	int *queue, *islandNum;
	int curTri, nextTri, edgeNum;
	const int *index;

	numIslands = 0;
	numTris = indexes.Num( ) / 3;
	islandNum = (int *) _alloca16( numTris * sizeof( int ) );
	Common::Com_Memset( islandNum, -1, numTris * sizeof( int ) );
	queue = (int *) _alloca16( numTris * sizeof( int ) );

	for( i = 0; i < numTris; i++ ) {

		if( islandNum[ i ] != -1 ) {
			continue;
		}

        queueStart = 0;
		queueEnd = 1;
		queue[ 0 ] = i;
		islandNum[ i ] = numIslands;

		for( curTri = queue[ queueStart ]; queueStart < queueEnd; curTri = queue[ ++queueStart ] ) {

			index = &edgeIndexes[curTri * 3];

			for( j = 0; j < 3; j++ ) {

				edgeNum = index[ j ];
				nextTri = edges[ abs( edgeNum ) ].tris[ INTSIGNBITNOTSET( edgeNum ) ];

				if( nextTri == -1 ) {

					continue;
				}

				nextTri /= 3;

				if( islandNum[ nextTri ] != -1 ) {

					continue;
				}

				queue[ queueEnd++ ] = nextTri;
				islandNum[ nextTri ] = numIslands;
			}
		}

		numIslands++;
	}

	return ( numIslands == 1 );
}

bool Surface::IsClosed( void ) const {

	for( int i = 0; i < edges.Num( ); i++ ) {

		if( edges[ i ].tris[ 0 ] < 0 || edges[ i ].tris[ 1 ] < 0 ) {

			return false;
		}
	}

	return true;
}

bool Surface::IsPolytope( const float epsilon ) const {

	int i, j;
	Plane plane;

	if( !IsClosed( ) ) {

		return false;
	}

	for( i = 0; i < indexes.Num( ); i += 3 ) {

		plane.FromPoints( verts[ indexes[ i + 0 ] ].xyz, verts[ indexes[ i + 1 ] ].xyz, verts[ indexes[ i + 2 ] ].xyz );

		for( j = 0; j < verts.Num( ); j++ ) {

			if( plane.Side( verts[ j ].xyz, epsilon ) == SIDE_FRONT ) {

				return false;
			}
		}
	}

	return true;
}

float Surface::PlaneDistance( const Plane & plane ) const {

	int		i;
	float	d, min, max;

	min = INFINITY;
	max = -min;

	for( i = 0; i < verts.Num( ); i++ ) {

		d = plane.Distance( verts[ i ].xyz );

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

bool Surface::LineIntersection( const Vec3 & start, const Vec3 & end, bool backFaceCull ) const {

	float scale;

	RayIntersection( start, end - start, scale, false );

	return ( scale >= 0.0f && scale <= 1.0f );
}

bool Surface::RayIntersection( const Vec3 &start, const Vec3 &dir, float &scale, bool backFaceCull ) const {

	int i, i0, i1, i2, s0, s1, s2;
	float d, s;
	byte *sidedness;
	Pluecker rayPl, pl;
	Plane plane;

	sidedness = (byte *)_alloca( edges.Num( ) * sizeof(byte) );
	scale = INFINITY;

	rayPl.FromRay( start, dir );

	// ray sidedness for edges
	for( i = 0; i < edges.Num( ); i++ ) {

		pl.FromLine( verts[ edges[ i ].verts[ 1 ] ].xyz, verts[ edges[ i ].verts[ 0 ] ].xyz );
		d = pl.PermutedInnerProduct( rayPl );
		sidedness[ i ] = FLOATSIGNBITSET( d );
	}

	// test triangles
	for( i = 0; i < edgeIndexes.Num( ); i += 3 ) {

		i0 = edgeIndexes[ i + 0 ];
		i1 = edgeIndexes[ i + 1 ];
		i2 = edgeIndexes[ i + 2 ];
		s0 = sidedness[abs(i0)] ^ INTSIGNBITSET( i0 );
		s1 = sidedness[abs(i1)] ^ INTSIGNBITSET( i1 );
		s2 = sidedness[abs(i2)] ^ INTSIGNBITSET( i2 );

		if( s0 & s1 & s2 ) {

			plane.FromPoints( verts[ indexes[ i + 0 ] ].xyz, verts[ indexes[ i + 1 ] ].xyz, verts[ indexes[ i + 2 ] ].xyz );
			plane.RayIntersection( start, dir, s );

			if( fabs( s ) < fabs( scale ) ) {

				scale = s;
			}

		} else if( !backFaceCull && !(s0 | s1 | s2) ) {

			plane.FromPoints( verts[ indexes[ i + 0 ] ].xyz, verts[ indexes[ i + 1 ] ].xyz, verts[ indexes[ i + 2 ] ].xyz );
			plane.RayIntersection( start, dir, s );

			if( fabs( s ) < fabs( scale ) ) {

				scale = s;
			}
		}
	}

	if( fabs( scale ) < INFINITY ) {

		return true;
	}

	return false;
}

void Surface::GenerateEdgeIndexes( void ) {

	int i, j, i0, i1, i2, s, v0, v1, edgeNum;
	int *index, *vertexEdges, *edgeChain;
	surfaceEdge_t e[ 3 ];

	vertexEdges = ( int * )_alloca16( verts.Num( ) * sizeof( int ) );
	Common::Com_Memset( vertexEdges, -1, verts.Num( ) * sizeof( int ) );
	edgeChain = ( int * )_alloca16( indexes.Num( ) * sizeof( int ) );

	edgeIndexes.SetNum( indexes.Num( ), true );

	edges.Clear( );

	// the first edge is a dummy
	e[ 0 ].verts[ 0 ] = e[ 0 ].verts[ 1 ] = e[ 0 ].tris[ 0 ] = e[ 0 ].tris[ 1 ] = 0;
	edges.Append( e[ 0 ] );

	for( i = 0; i < indexes.Num( ); i += 3 ) {

		index = indexes.Ptr( ) + i;

		// vertex numbers
		i0 = index[ 0 ];
		i1 = index[ 1 ];
		i2 = index[ 2 ];

		// setup edges each with smallest vertex number first
		s = INTSIGNBITSET( i1 - i0 );
		e[ 0 ].verts[ 0 ] = index[ s ];
		e[ 0 ].verts[ 1 ] = index[ s ^ 1 ];

		s = INTSIGNBITSET( i2 - i1 ) + 1;
		e[ 1 ].verts[ 0 ] = index[ s ];
		e[ 1 ].verts[ 1 ] = index[ s ^ 3 ];

		s = INTSIGNBITSET( i2 - i0 ) << 1;
		e[ 2 ].verts[ 0 ] = index[ s ];
		e[ 2 ].verts[ 1 ] = index[ s ^ 2 ];

		// get edges
		for( j = 0; j < 3; j++ ) {

			v0 = e[ j ].verts[ 0 ];
			v1 = e[ j ].verts[ 1 ];

			for( edgeNum = vertexEdges[ v0 ]; edgeNum >= 0; edgeNum = edgeChain[ edgeNum ] ) {

				if( edges[ edgeNum ].verts[ 1 ] == v1 ) {

					break;
				}
			}

			// if the edge does not yet exist
			if( edgeNum < 0 ) {

				e[ j ].tris[ 0 ] = e[ j ].tris[ 1 ] = -1;
				edgeNum = edges.Append( e[ j ] );
				edgeChain[ edgeNum ] = vertexEdges[ v0 ];
				vertexEdges[ v0 ] = edgeNum;
			}

			// update edge index and edge tri references
			if( index[ j ] == v0 ) {

				assert( edges[ edgeNum ].tris[ 0 ] == -1 ); // edge may not be shared by more than two triangles
				edges[ edgeNum ].tris[ 0 ] = i;
				edgeIndexes[ i + j ] = edgeNum;

			} else {

				assert( edges[ edgeNum ].tris[ 1 ] == -1 ); // edge may not be shared by more than two triangles
				edges[ edgeNum ].tris[ 1 ] = i;
				edgeIndexes[ i + j ] = -edgeNum;
			}
		}
	}
}

int Surface::FindEdge( int v1, int v2 ) const {

	int i, firstVert, secondVert;

	if( v1 < v2 ) {

		firstVert = v1;
		secondVert = v2;
	} else {

		firstVert = v2;
		secondVert = v1;
	}

	for( i = 1; i < edges.Num( ); i++ ) {

		if( edges[ i ].verts[ 0 ] == firstVert ) {

			if( edges[ i ].verts[ 1 ] == secondVert ) {

				break;
			}
		}
	}

	if( i < edges.Num( ) ) {

		return v1 < v2 ? i : -i;
	}

	return 0;
}

bool Surface::ComputeEdgeVerts( const int index, DrawVert & vert1, DrawVert & vert2 ) const {

	const surfaceEdge_t * edge = &this->edges[ index ];
	int t1ev = -1, t2ev = -1;

	for( int i = 0; i < 3 && edge->tris[ 0 ] != -1; i++ ) {

		int v = this->indexes[ edge->tris[ 0 ] + i ];
		if( edge->ContainsVert( v ) ) continue;
		t1ev = v;
		break;
	}

	for( int i = 0; i < 3 && edge->tris[ 1 ] != -1; i++ ) {

		int v = this->indexes[ edge->tris[ 1 ] + i ];
		if( edge->ContainsVert( v ) ) continue;
		t2ev = v;
		break;
	}

	assert( t1ev != -1 || t2ev != -1 );

	vert1.Clear( );
	vert2.Clear( );
	vert1.xyz = this->verts[ edge->verts[ 0 ] ].xyz;
	vert2.xyz = this->verts[ edge->verts[ 1 ] ].xyz;

	if( t1ev != -1 ) vert1.normal.Normal( ) = ( this->verts[ t1ev ].xyz - this->verts[ edge->verts[ 0 ] ].xyz ).Cross( this->verts[ edge->verts[ 1 ] ].xyz - this->verts[ edge->verts[ 0 ] ].xyz );

	if( t2ev != -1 ) vert2.normal.Normal( ) = ( this->verts[ t2ev ].xyz - this->verts[ edge->verts[ 1 ] ].xyz ).Cross( this->verts[ edge->verts[ 0 ] ].xyz - this->verts[ edge->verts[ 1 ] ].xyz );

	if( t1ev == -1 ) vert1.normal.Normal( ) = ( this->verts[ edge->verts[ 1 ] ].xyz - this->verts[ edge->verts[ 0 ] ].xyz ).Cross( vert2.normal.Normal( ) );

	if( t2ev == -1 ) vert2.normal.Normal( ) = ( this->verts[ edge->verts[ 0 ] ].xyz - this->verts[ edge->verts[ 1 ] ].xyz ).Cross( vert1.normal.Normal( ) );

	vert1.Normalize( );
	vert1.normal[ 3 ] = ( float )t1ev;
	vert2.Normalize( );
	vert2.normal[ 3 ] = ( float )t2ev;

	if( vert1.xyz.Compare( vert2.xyz, 0.0f ) ) return false;

	//if( vert1.normal.Normal( ).Compare( vert2.normal.Normal( ), 0.0f ) ) return false;

	return true;
}

void Surface::Print( ) const {

	Common::Com_Printf( "Surface ind: %i, vert: %i, edges: %i, edgeInd: %i {\n",
		this->indexes.Num( ), this->verts.Num( ), this->edges.Num( ), this->edgeIndexes.Num( ) );

	if( this->indexes.Num( ) ) {

		Common::Com_Printf( "\tIndexes:" );

		for( int i = 0; i < this->indexes.Num( ); i++ ) {

			if( !( i % 3 ) ) Common::Com_Printf( "\n\t\t" );
			Common::Com_Printf( "%s%i", ( i % 3 ) ? " " : "", this->indexes[ i ] );
		}

		Common::Com_Printf( "\n" );
	}

	if( this->verts.Num( ) ) {

		Common::Com_Printf( "\tVertices:" );

		for( int i = 0; i < this->verts.Num( ); i++ ) {

			Common::Com_Printf( "\n\t\t" );
			Common::Com_Printf( "xyz( %s ) st( %s )", this->verts[ i ].xyz.ToString( ).c_str( ), this->verts[ i ].st.ToString( ).c_str( ) );
		}

		Common::Com_Printf( "\n" );
	}

	if( this->edges.Num( ) ) {

		Common::Com_Printf( "\tEdges:" );

		for( int i = 0; i < this->edges.Num( ); i++ ) {

			Common::Com_Printf( "\n\t\t" );
			Common::Com_Printf( "tri( %i %i ) verts( %i %i )", this->edges[ i ].tris[ 0 ], this->edges[ i ].tris[ 1 ], this->edges[ i ].verts[ 0 ], this->edges[ i ].verts[ 1 ] );
		}

		Common::Com_Printf( "\n" );
	}
	
	if( this->edgeIndexes.Num( ) ) {

		Common::Com_Printf( "\tEdgeIndexes:" );

		for( int i = 0; i < this->edgeIndexes.Num( ); i++ ) {

			Common::Com_Printf( " %i", this->edgeIndexes[ i ] );
		}

		Common::Com_Printf( "\n" );
	}

	Common::Com_Printf( "}\n" );
}