#ifndef __SURFACE_H__
#define __SURFACE_H__

typedef enum {

	SURF_NULL		= 0,

	/// value will hold the light strength
	SURF_LIGHT		= BIT( 0 ),

	/// effects game physics
	SURF_SLICK		= BIT( 1 ),

	/// don't draw, but add to skybox
	SURF_SKY		= BIT( 2 ),

	/// turbulent water warp
	SURF_WARP		= BIT( 3 ),
	SURF_TRANS33	= BIT( 4 ),
	SURF_TRANS66	= BIT( 5 ),

	/// scroll towards angle
	SURF_FLOWING	= BIT( 6 ),

	/// don't bother referencing the texture
	SURF_NODRAW		= BIT( 7 )

} surfaceFlags_t;

typedef struct surfaceEdge_s {

	/// edge vertices always with ( verts[0] < verts[1] )
	int						verts[ 2 ];
	/// edge triangles
	int						tris[ 2 ];

	bool					ContainsVert( int vert ) const;

} surfaceEdge_t;

INLINE bool surfaceEdge_t::ContainsVert( int vert ) const {

	if( verts[ 0 ] == vert ) return true;
	if( verts[ 1 ] == vert ) return true;
	return false;
}

/// * * * * * * * * * * * * * * *
///
/// Surface
///
/// A surface is tesselated to a triangle mesh with each edge shared by at most two triangles.
///
/// * * * * * * * * * * * * * * *

class Surface {

public:

								Surface( void );
								explicit Surface( const Surface &surf );
								explicit Surface( const DrawVert *verts, const int numVerts, const int *indexes, const int numIndexes );
								~Surface( void );

	const DrawVert &			operator[]( const int index ) const;
	DrawVert &					operator[]( const int index );
	Surface &					operator+=( const Surface &surf );

	Bounds						ComputeBounds( ) const;
	Vec3						ComputeCenter( ) const;

	int							GetNumIndexes( void ) const { return indexes.Num( ); }
	const int *					GetIndexes( void ) const { return indexes.Ptr( ); }
	const List< int > &			GetIndexesList( void ) const { return indexes; }
	int							GetNumVertices( void ) const { return verts.Num( ); }
	const DrawVert *			GetVertices( void ) const { return verts.Ptr( ); }
	const List< DrawVert > &	GetVerticesList( void ) const { return verts; }
	int							GetNumEdgeIndexes( void ) const { return edgeIndexes.Num( ); }
	const int *					GetEdgeIndexes( void ) const { return edgeIndexes.Ptr( ); }
	int							GetNumEdges( void ) const { return edges.Num( ); }
	const surfaceEdge_t *		GetEdges( void ) const { return edges.Ptr( ); }

	void						Clear( void );
	void						SwapTriangles( Surface &surf );
	void						TranslateSelf( const Vec3 &translation );
	void						RotateSelf( const Mat3 &rotation );

	/// splits the surface into a front and back surface, the surface itself stays unchanged
	/// frontOnPlaneEdges and backOnPlaneEdges optionally store the indexes to the edges that lay on the split plane
	/// returns a SIDE_?
	int							Split( const Plane &plane, const float epsilon, Surface **front, Surface **back, int *frontOnPlaneEdges = NULL, int *backOnPlaneEdges = NULL ) const;

	/// cuts off the part at the back side of the plane, returns true if some part was at the front
	/// if there is nothing at the front the number of points is set to zero
	bool						ClipInPlace( const Plane &plane, const float epsilon = ON_EPSILON, const bool keepOn = false );

	/// returns true if each triangle can be reached from any other triangle by a traversal
	bool						IsConnected( void ) const;

	/// returns true if the surface is closed
	bool						IsClosed( void ) const;

	/// returns true if the surface is a convex hull
	bool						IsPolytope( const float epsilon = 0.1f ) const;

	float						PlaneDistance( const Plane &plane ) const;
	int							PlaneSide( const Plane &plane, const float epsilon = ON_EPSILON ) const;

	/// returns true if the line intersects one of the surface triangles
	bool						LineIntersection( const Vec3 &start, const Vec3 &end, bool backFaceCull = false ) const;

	/// intersection point is start + dir * scale
	bool						RayIntersection( const Vec3 &start, const Vec3 &dir, float &scale, bool backFaceCull = false ) const;

	bool						RayIntersection( const Vec3 &start, const Vec3 &dir, float &scale, Plane & outPlane, bool backFaceCull = false ) const;

	bool						ComputeEdgeVerts( const int index, DrawVert & vert1, DrawVert & vert2 ) const;

	void						Print( ) const;

protected:

	/// vertices
	List< DrawVert >			verts;

	/// 3 references to vertices for each triangle
	List< int >					indexes;

	/// edges
	List< surfaceEdge_t >		edges;

	/// 3 references to edges for each triangle, may be negative for reversed edge
	List< int >					edgeIndexes;

protected:

	/// Assumes each edge is shared by at most two triangles.
	void						GenerateEdgeIndexes( void );

	int							FindEdge( int v1, int v2 ) const;
};

INLINE Surface::Surface( void ) {

}

INLINE Surface::Surface( const DrawVert *verts, const int numVerts, const int *indexes, const int numIndexes ) {
	
	assert( verts != NULL && indexes != NULL && numVerts > 0 && numIndexes > 0 );
	this->verts.SetNum( numVerts );
	memcpy( this->verts.Ptr(), verts, numVerts * sizeof( verts[0] ) );
	this->indexes.SetNum( numIndexes );
	memcpy( this->indexes.Ptr(), indexes, numIndexes * sizeof( indexes[0] ) );
	GenerateEdgeIndexes();
}

INLINE Surface::Surface( const Surface &surf ) {

#if 0
	this->verts = surf.verts;
	this->indexes = surf.indexes;
	this->edges = surf.edges;
	this->edgeIndexes = surf.edgeIndexes;
#endif

	verts.SetNum( surf.verts.Num( ) );
	Common::Com_Memcpy( verts.Ptr( ), surf.verts.Ptr( ), sizeof( DrawVert ) * surf.verts.Num( ) );

	indexes.SetNum( surf.indexes.Num( ) );
	Common::Com_Memcpy( indexes.Ptr( ), surf.indexes.Ptr( ), sizeof( int ) * surf.indexes.Num( ) );

	edges.SetNum( surf.edges.Num( ) );
	Common::Com_Memcpy( edges.Ptr( ), surf.edges.Ptr( ), sizeof( surfaceEdge_t ) * surf.edges.Num( ) );

	edgeIndexes.SetNum( surf.edgeIndexes.Num( ) );
	Common::Com_Memcpy( edgeIndexes.Ptr( ), surf.edgeIndexes.Ptr( ), sizeof( int ) * surf.edgeIndexes.Num( ) );
}

INLINE Surface::~Surface( void ) {
}

INLINE const DrawVert &Surface::operator[]( const int index ) const {
	
	return verts[ index ];
};

INLINE DrawVert &Surface::operator[]( const int index ) {

	return verts[ index ];
};

INLINE Surface &Surface::operator+=( const Surface &surf ) {

	int i, m, n;
	n = verts.Num( );
	m = indexes.Num( );
	verts.Append( surf.verts );			// merge verts where possible ?
	indexes.Append( surf.indexes );

	for( i = m; i < indexes.Num( ); i++ ) {
	
		indexes[i] += n;
	}

	GenerateEdgeIndexes( );
	return *this;
}

INLINE Bounds Surface::ComputeBounds( ) const {

	Bounds out;
	out.Clear( );
	
	for( int j = 0; j < GetNumVertices( ); j++ ) {

		out.AddPoint( verts[ j ].xyz );
	}

	return out;
}

INLINE Vec3 Surface::ComputeCenter( ) const {

	Vec3 out = vec3_origin;

	for( int j = 0; j < GetNumVertices( ); j++ ) {

		out += verts[ j ].xyz;
	}

	out /= ( float )GetNumVertices( );

	return out;
}

INLINE void Surface::Clear( void ) {

	verts.Clear( );
	indexes.Clear( );
	edges.Clear( );
	edgeIndexes.Clear( );
}

INLINE void Surface::SwapTriangles( Surface &surf ) {

	verts.Swap( surf.verts );
	indexes.Swap( surf.indexes );
	edges.Swap( surf.edges );
	edgeIndexes.Swap( surf.edgeIndexes );
}

INLINE void Surface::TranslateSelf( const Vec3 &translation ) {

	for( int i = 0; i < verts.Num(); i++ ) {

		verts[i].xyz += translation;
	}
}

INLINE void Surface::RotateSelf( const Mat3 &rotation ) {

	for( int i = 0; i < verts.Num(); i++ ) {

		verts[i].xyz *= rotation;
		verts[i].normal *= rotation;
#if 0
		verts[i].tangents[0] *= rotation;
		verts[i].tangents[1] *= rotation;
#endif
	}
}

#endif /* !__SURFACE_H__ */
