#include "precompiled.h"
#pragma hdrstop

#define POLYTOPE_VERTEX_EPSILON		0.5f

/*
====================
Polytope::FromPlanes
====================
*/
void Polytope::FromPlanes( const Plane *planes, const int numPlanes ) {
	int i, j, k, *windingVerts;
	FixedWinding w;
	DrawVert newVert;
	windingVerts = (int *) _alloca( MAX_POINTS_ON_WINDING * sizeof( int ) );
	Common::Com_Memset( &newVert, 0, sizeof( newVert ) );
	for( i = 0; i < numPlanes; i++ ) {
		w.BaseForPlane( planes[i], MAX_WORLD_SIZE );
		for ( j = 0; j < numPlanes; j++ ) {
			if ( j == i )
				continue;
			if ( !w.ClipInPlace( -planes[j], ON_EPSILON, true ) )
				break;
		}
		if( !w.GetNumPoints( ) )
			continue;
		for( j = 0; j < w.GetNumPoints(); j++ ) {
			for( k = 0; k < verts.Num(); k++ ) {
				if( verts[k].xyz.Compare( w[j].ToVec3(), POLYTOPE_VERTEX_EPSILON ) )
					break;
			}
			if ( k >= verts.Num() ) {
				newVert.xyz = w[j].ToVec3();
				k = verts.Append( newVert );
			}
			windingVerts[j] = k;
		}
		for( j = 2; j < w.GetNumPoints(); j++ ) {
			indexes.Append( windingVerts[0] );
			indexes.Append( windingVerts[j-1] );
			indexes.Append( windingVerts[j] );
		}
	}
	GenerateEdgeIndexes();
}

Polytope & Polytope::FromPlanes( const PlaneSet & plane_set ) {

	int i, j, k, *windingVerts;
	FixedWinding w;
	DrawVert newVert;

	windingVerts = ( int * )_alloca( MAX_POINTS_ON_WINDING * sizeof( int ) );
	Common::Com_Memset( &newVert, 0, sizeof( newVert ) );
	for( i = 0; i < plane_set.Num( ); i++ ) {
		w.BaseForPlane( *plane_set[ i ], MAX_WORLD_SIZE );
		for ( j = 0; j < plane_set.Num( ); j++ ) {
			if ( j == i )
				continue;
			if ( !w.ClipInPlace( -*plane_set[ j ], ON_EPSILON, true ) )
				break;
		}
		if( !w.GetNumPoints( ) )
			continue;
		for( j = 0; j < w.GetNumPoints( ); j++ ) {
			for( k = 0; k < verts.Num( ); k++ ) {
				if ( verts[ k ].xyz.Compare( w[ j ].ToVec3( ), POLYTOPE_VERTEX_EPSILON ) )
					break;
			}
			if( k >= verts.Num( ) ) {
				newVert.xyz = w[ j ].ToVec3( );
				newVert.normal = *plane_set[ i ];
				k = verts.Append( newVert );
			} else {
				verts[ k ].normal = verts[ k ].normal + *plane_set[ i ];
			}
			windingVerts[ j ] = k;
		}
		for( j = 2; j < w.GetNumPoints( ); j++ ) {
			indexes.Append( windingVerts[ 0 ] );
			indexes.Append( windingVerts[ j - 1 ] );
			indexes.Append( windingVerts[ j ] );
		}
	}
	for( i = 0; i < verts.Num( ); i++ ) {
		verts[ i ].Normalize( );
	}
	GenerateEdgeIndexes( );
	return *this;
}

void Polytope::SetupHelix( const Vec3 & origin, float radius, int twists ) {

	Vec3 coord;
	GLfloat v,u;												// Angles
	int baseVertex, baseIndex;

	for( float phi = 0; phi <= 360; phi += 20.0 ) {							// 360 Degrees In Steps Of 20
		for( float theta = 0; theta <= 360 * twists; theta += 20.0 ) {			// 360 Degrees * Number Of Twists In Steps Of 20

			baseVertex = verts.Num( );
			verts.SetNum( baseVertex + 4 );

			v=(phi/180.0f*3.142f);								// Calculate Angle Of First Point	(  0 )
			u=(theta/180.0f*3.142f);							// Calculate Angle Of First Point	(  0 )

			coord[ 0 ] = float(cos(u)*(2.0f+cos(v) ))*radius;					// Calculate x Position (1st Point)
			coord[ 1 ] = float(sin(u)*(2.0f+cos(v) ))*radius;					// Calculate y Position (1st Point)
			coord[ 2 ] = float((( u-(2.0f*3.142f)) + sin(v) ) * radius);		// Calculate z Position (1st Point)

			verts[ baseVertex + 0 ].xyz = origin + coord;

			v=(phi/180.0f*3.142f);								// Calculate Angle Of Second Point	(  0 )
			u=((theta+20)/180.0f*3.142f);						// Calculate Angle Of Second Point	( 20 )

			coord[ 0 ] = float(cos(u)*(2.0f+cos(v) ))*radius;					// Calculate x Position (2nd Point)
			coord[ 1 ] = float(sin(u)*(2.0f+cos(v) ))*radius;					// Calculate y Position (2nd Point)
			coord[ 2 ] = float((( u-(2.0f*3.142f)) + sin(v) ) * radius);		// Calculate z Position (2nd Point)

			verts[ baseVertex + 1 ].xyz = origin + coord;

			v=((phi+20)/180.0f*3.142f);							// Calculate Angle Of Third Point	( 20 )
			u=((theta+20)/180.0f*3.142f);						// Calculate Angle Of Third Point	( 20 )

			coord[ 0 ] = float(cos(u)*(2.0f+cos(v) ))*radius;					// Calculate x Position (3rd Point)
			coord[ 1 ] = float(sin(u)*(2.0f+cos(v) ))*radius;					// Calculate y Position (3rd Point)
			coord[ 2 ] = float((( u-(2.0f*3.142f)) + sin(v) ) * radius);		// Calculate z Position (3rd Point)

			verts[ baseVertex + 2 ].xyz = origin + coord;

			v=((phi+20)/180.0f*3.142f);							// Calculate Angle Of Fourth Point	( 20 )
			u=((theta)/180.0f*3.142f);							// Calculate Angle Of Fourth Point	(  0 )

			coord[ 0 ] = float(cos(u)*(2.0f+cos(v) ))*radius;					// Calculate x Position (4th Point)
			coord[ 1 ] = float(sin(u)*(2.0f+cos(v) ))*radius;					// Calculate y Position (4th Point)
			coord[ 2 ] = float((( u-(2.0f*3.142f)) + sin(v) ) * radius);		// Calculate z Position (4th Point)

			verts[ baseVertex + 3 ].xyz = origin + coord;

			coord = ( verts[ baseVertex + 0 ].xyz - verts[ baseVertex + 1 ].xyz ).Cross( verts[ baseVertex + 1 ].xyz - verts[ baseVertex + 2 ].xyz );
			coord.Normalize( );

			verts[ baseVertex + 0 ].normal.Normal( ) = coord;
			verts[ baseVertex + 1 ].normal.Normal( ) = coord;
			verts[ baseVertex + 2 ].normal.Normal( ) = coord;
			verts[ baseVertex + 3 ].normal.Normal( ) = coord;

			baseIndex = indexes.Num( );
			indexes.SetNum( baseIndex + 6 );

			indexes[ baseIndex + 0 ] = baseVertex + 2;
			indexes[ baseIndex + 1 ] = baseVertex + 1;
			indexes[ baseIndex + 2 ] = baseVertex + 0;
			indexes[ baseIndex + 3 ] = baseVertex + 0;
			indexes[ baseIndex + 4 ] = baseVertex + 3;
			indexes[ baseIndex + 5 ] = baseVertex + 2;
		}
	}

	GenerateEdgeIndexes( );
}

/*
====================
Polytope::SetupTetrahedron
====================
*/
void Polytope::SetupTetrahedron( const Bounds &bounds ) {

	Vec3 center, scale;
	float c1, c2, c3;

	c1 = 0.4714045207f;
	c2 = 0.8164965809f;
	c3 = -0.3333333333f;

	center = bounds.GetCenter();
	scale = bounds[1] - center;

	verts.SetNum( 4 );
	verts[0].xyz = center + Vec3( 0.0f, 0.0f, scale.z );
	verts[1].xyz = center + Vec3( 2.0f * c1 * scale.x, 0.0f, c3 * scale.z );
	verts[2].xyz = center + Vec3( -c1 * scale.x, c2 * scale.y, c3 * scale.z );
	verts[3].xyz = center + Vec3( -c1 * scale.x, -c2 * scale.y, c3 * scale.z );

	indexes.SetNum( 4*3 );
	indexes[0*3+0] = 0;
	indexes[0*3+1] = 1;
	indexes[0*3+2] = 2;
	indexes[1*3+0] = 0;
	indexes[1*3+1] = 2;
	indexes[1*3+2] = 3;
	indexes[2*3+0] = 0;
	indexes[2*3+1] = 3;
	indexes[2*3+2] = 1;
	indexes[3*3+0] = 1;
	indexes[3*3+1] = 3;
	indexes[3*3+2] = 2;

	GenerateEdgeIndexes();
}

/*
====================
Polytope::SetupHexahedron
====================
*/
void Polytope::SetupHexahedron( const Bounds &bounds ) {

	Vec3 center, scale;

	center = bounds.GetCenter();
	scale = bounds[1] - center;

	verts.SetNum( 8 );
	verts[0].xyz = center + Vec3( -scale.x, -scale.y, -scale.z );
	verts[1].xyz = center + Vec3(  scale.x, -scale.y, -scale.z );
	verts[2].xyz = center + Vec3(  scale.x,  scale.y, -scale.z );
	verts[3].xyz = center + Vec3( -scale.x,  scale.y, -scale.z );
	verts[4].xyz = center + Vec3( -scale.x, -scale.y,  scale.z );
	verts[5].xyz = center + Vec3(  scale.x, -scale.y,  scale.z );
	verts[6].xyz = center + Vec3(  scale.x,  scale.y,  scale.z );
	verts[7].xyz = center + Vec3( -scale.x,  scale.y,  scale.z );

	indexes.SetNum( 12*3 );
	indexes[ 0*3+0] = 0;
	indexes[ 0*3+1] = 3;
	indexes[ 0*3+2] = 2;
	indexes[ 1*3+0] = 0;
	indexes[ 1*3+1] = 2;
	indexes[ 1*3+2] = 1;
	indexes[ 2*3+0] = 0;
	indexes[ 2*3+1] = 1;
	indexes[ 2*3+2] = 5;
	indexes[ 3*3+0] = 0;
	indexes[ 3*3+1] = 5;
	indexes[ 3*3+2] = 4;
	indexes[ 4*3+0] = 0;
	indexes[ 4*3+1] = 4;
	indexes[ 4*3+2] = 7;
	indexes[ 5*3+0] = 0;
	indexes[ 5*3+1] = 7;
	indexes[ 5*3+2] = 3;
	indexes[ 6*3+0] = 6;
	indexes[ 6*3+1] = 5;
	indexes[ 6*3+2] = 1;
	indexes[ 7*3+0] = 6;
	indexes[ 7*3+1] = 1;
	indexes[ 7*3+2] = 2;
	indexes[ 8*3+0] = 6;
	indexes[ 8*3+1] = 2;
	indexes[ 8*3+2] = 3;
	indexes[ 9*3+0] = 6;
	indexes[ 9*3+1] = 3;
	indexes[ 9*3+2] = 7;
	indexes[10*3+0] = 6;
	indexes[10*3+1] = 7;
	indexes[10*3+2] = 4;
	indexes[11*3+0] = 6;
	indexes[11*3+1] = 4;
	indexes[11*3+2] = 5;

	GenerateEdgeIndexes();
}

/*
====================
Polytope::SetupOctahedron
====================
*/
void Polytope::SetupOctahedron( const Bounds &bounds ) {

	Vec3 center, scale;

	center = bounds.GetCenter();
	scale = bounds[1] - center;

	verts.SetNum( 6 );
	verts[0].xyz = center + Vec3(  scale.x, 0.0f, 0.0f );
	verts[1].xyz = center + Vec3( -scale.x, 0.0f, 0.0f );
	verts[2].xyz = center + Vec3( 0.0f,  scale.y, 0.0f );
	verts[3].xyz = center + Vec3( 0.0f, -scale.y, 0.0f );
	verts[4].xyz = center + Vec3( 0.0f, 0.0f,  scale.z );
	verts[5].xyz = center + Vec3( 0.0f, 0.0f, -scale.z );

	indexes.SetNum( 8*3 );
	indexes[0*3+0] = 4;
	indexes[0*3+1] = 0;
	indexes[0*3+2] = 2;
	indexes[1*3+0] = 4;
	indexes[1*3+1] = 2;
	indexes[1*3+2] = 1;
	indexes[2*3+0] = 4;
	indexes[2*3+1] = 1;
	indexes[2*3+2] = 3;
	indexes[3*3+0] = 4;
	indexes[3*3+1] = 3;
	indexes[3*3+2] = 0;
	indexes[4*3+0] = 5;
	indexes[4*3+1] = 2;
	indexes[4*3+2] = 0;
	indexes[5*3+0] = 5;
	indexes[5*3+1] = 1;
	indexes[5*3+2] = 2;
	indexes[6*3+0] = 5;
	indexes[6*3+1] = 3;
	indexes[6*3+2] = 1;
	indexes[7*3+0] = 5;
	indexes[7*3+1] = 0;
	indexes[7*3+2] = 3;

	GenerateEdgeIndexes();
}

/*
====================
Polytope::SetupDodecahedron
====================
*/
void Polytope::SetupDodecahedron( const Bounds &bounds ) {
}

/*
====================
Polytope::SetupIcosahedron
====================
*/
void Polytope::SetupIcosahedron( const Bounds &bounds ) {
}

/*
====================
Polytope::SetupCylinder
====================
*/
void Polytope::SetupCylinder( const Bounds &bounds, const int numSides ) {
}

/*
====================
Polytope::SetupCone
====================
*/
void Polytope::SetupCone( const Bounds &bounds, const int numSides ) {
}

/*
====================
Polytope::SplitPolytope
====================
*/
int Polytope::SplitPolytope( const Plane &plane, const float epsilon, Polytope **front, Polytope **back ) const {

	int side, i, j, s, v0, v1, v2, edgeNum;
	Surface *surface[2];
	Polytope *polytopeSurfaces[2], *surf;
	int *onPlaneEdges[2];

	onPlaneEdges[0] = (int *) _alloca( indexes.Num() / 3 * sizeof( int ) );
	onPlaneEdges[1] = (int *) _alloca( indexes.Num() / 3 * sizeof( int ) );

	side = Split( plane, epsilon, &surface[0], &surface[1], onPlaneEdges[0], onPlaneEdges[1] );

	*front = polytopeSurfaces[0] = new Polytope;
	*back = polytopeSurfaces[1] = new Polytope;

	for ( s = 0; s < 2; s++ ) {
		if ( surface[s] ) {
			polytopeSurfaces[s] = new Polytope;
			polytopeSurfaces[s]->SwapTriangles( *surface[s] );
			delete surface[s];
			surface[s] = NULL;
		}
	}

	*front = polytopeSurfaces[0];
	*back = polytopeSurfaces[1];

	if ( side != SIDE_CROSS ) {
		return side;
	}

	// add triangles to close off the front and back polytope
	for ( s = 0; s < 2; s++ ) {

		surf = polytopeSurfaces[s];

		edgeNum = surf->edgeIndexes[onPlaneEdges[s][0]];
		v0 = surf->edges[abs(edgeNum)].verts[INTSIGNBITSET(edgeNum)];
		v1 = surf->edges[abs(edgeNum)].verts[INTSIGNBITNOTSET(edgeNum)];

		for ( i = 1; onPlaneEdges[s][i] >= 0; i++ ) {
			for ( j = i+1; onPlaneEdges[s][j] >= 0; j++ ) {
				edgeNum = surf->edgeIndexes[onPlaneEdges[s][j]];
				if ( v1 == surf->edges[abs(edgeNum)].verts[INTSIGNBITSET(edgeNum)] ) {
					v1 = surf->edges[abs(edgeNum)].verts[INTSIGNBITNOTSET(edgeNum)];
					Swap( onPlaneEdges[s][i], onPlaneEdges[s][j] );
					break;
				}
			}
		}

		for ( i = 2; onPlaneEdges[s][i] >= 0; i++ ) {
			edgeNum = surf->edgeIndexes[onPlaneEdges[s][i]];
			v1 = surf->edges[abs(edgeNum)].verts[INTSIGNBITNOTSET(edgeNum)];
			v2 = surf->edges[abs(edgeNum)].verts[INTSIGNBITSET(edgeNum)];
			surf->indexes.Append( v0 );
			surf->indexes.Append( v1 );
			surf->indexes.Append( v2 );
		}

		surf->GenerateEdgeIndexes();
	}

	return side;
}
