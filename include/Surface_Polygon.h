#define POLYGON_VERTEX_EPSILON		0.1f

class PolygonBuilder : public Surface, public FixedWinding {

public:

							PolygonBuilder( );
							PolygonBuilder( const CAListBase< Plane > & plane_set, int base_plane_index );

	const DrawVert &		operator[ ]( const int index ) const;
	DrawVert &				operator[ ]( const int index );

	virtual void			Clear( );

							// starts a new polygon from this plane
	void					BaseForPlane( const Plane & plane );
	bool					AddClippingPlane( const Plane & plane );
	bool					Build( );

	bool					BuildCustom( const CAListBase< Plane > & plane_set, int base_plane_index, int plane_count, ... );

protected:

	bool					basePlaneOK;
	Plane					basePlane;
	CAList< Plane, 32 >		clippingPlanes;
};

INLINE PolygonBuilder::PolygonBuilder( ) {

	basePlaneOK = false;
}

INLINE const DrawVert & PolygonBuilder::operator[ ]( const int index ) const {

	return verts[ index ];
};

INLINE DrawVert & PolygonBuilder::operator[ ]( const int index ) {

	return verts[ index ];
};

INLINE void PolygonBuilder::Clear( ) {

	basePlaneOK = false;
	clippingPlanes.Clear( );
	FixedWinding::Clear( );
	Surface::Clear( );
}

INLINE void PolygonBuilder::BaseForPlane( const Plane & plane ) {

	if( basePlaneOK )
		Clear( );
	basePlane = plane;
	FixedWinding::BaseForPlane( plane, MAX_WORLD_SIZE );
	basePlaneOK = true;
}

INLINE bool PolygonBuilder::AddClippingPlane( const Plane & plane ) {

	Vec3 start, dir;
	if( !basePlaneOK )
		return false;
	if( basePlane.PlaneIntersection( plane, start, dir ) ) {
		clippingPlanes.Append( plane );
		return true;
	}
	return false;
}
