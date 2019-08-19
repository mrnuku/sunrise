#ifndef WINDING_H
#define WINDING_H

// maximum world size
#define MAX_WORLD_COORD			( 128 * 1024 )
#define MIN_WORLD_COORD			( -128 * 1024 )
#define MAX_WORLD_SIZE			( MAX_WORLD_COORD - MIN_WORLD_COORD )

// // //// // //// // //// //
/// Winding
//
/// A winding is an arbitrary convex polygon defined by an array of points.
// //// // //// // //// //

class Winding {
protected:
	/// number of points
	int							numPoints;
	/// pointer to point data
	Vec5 *						p;
	int							allocedSize;
	bool						EnsureAlloced( int n, bool keep = false );

public:
								Winding( );
								/// allocate for n points
								explicit Winding( const int n );
								/// winding from points
								explicit Winding( const Vec3 * verts, const int n );
								/// base winding for plane
								explicit Winding( const Vec3 & normal, const float dist, const float radius );
								/// base winding for plane
								explicit Winding( const Plane & plane, const float radius );
								explicit Winding( const Winding & winding );

	virtual						~Winding( );

	Winding &					operator=( const Winding & winding );
	const Vec5 &				operator[ ]( const int index ) const;
	Vec5 &						operator[ ]( const int index );

	/// add a point to the end of the winding point array
	Winding &					operator+=( const Vec3 & v );

	Winding &					operator+=( const Vec5 & v );
	void						AddPoint( const Vec3 & v );
	void						AddPoint( const Vec5 & v );

	/// number of points on winding
	int							GetNumPoints( ) const;

	void						SetNumPoints( int n );
	virtual void				Clear( );

	void						Rotate( const Vec3& origin, const Mat3& axis );

	/// huge winding for plane, the points go counter clockwise when facing the front of the plane
	void						BaseForPlane( const Vec3 & normal, const float dist, const float radius );

	/// huge winding for plane, the points go counter clockwise when facing the front of the plane
	void						BaseForPlane( const Plane & plane, const float radius );

	/// splits the winding into a front and back winding, the winding itself stays unchanged, returns a SIDE_?
	int							Split( const Plane & plane, const float epsilon, Winding ** front, Winding ** back ) const;
					
	/// idential to the above version, but avoids heap allocations
	int							Split( const Plane & plane, const float epsilon, Winding& front, Winding& back ) const;

	/// chops of the part of the winding at the back of the plane, if there is nothing at the front the number of points is set to zero, returns a SIDE_?
	int							SplitInPlace( const Plane & plane, const float epsilon, Winding ** back );

	/// cuts off the part at the back side of the plane, returns true if some part was at the front, if there is nothing at the front the number of points is set to zero
	bool						ClipInPlace( const Plane & plane, const float epsilon = ON_EPSILON, const bool keepOn = false );

	/// splits the winding into a front and back winding, the winding itself stays unchanged, returns a SIDE_?
	int							SplitWithEdgeNums( const int *edgeNums, const Plane & plane, const int edgeNum, const float epsilon, Winding ** front, Winding ** back, int ** frontEdgePlanes, int ** backEdgePlanes ) const;

	/// chops of the part of the winding at the back of the plane, if there is nothing at the front the number of points is set to zero, returns a SIDE_?
	int							SplitInPlaceWithEdgeNums( List< int > & edgeNums, const Plane & plane, const float epsilon, Winding ** back, int ** backEdgePlanes );

	/// cuts off the part at the back side of the plane, returns true if some part was at the front, if there is nothing at the front the number of points is set to zero
	int							ClipInPlaceWithEdgeNums( List< int > & edgeNums, const Plane & plane, const int edgeNum, const float epsilon = ON_EPSILON, const bool keepOn = false );

	/// returns a copy of the winding
	Winding *					Copy( ) const;
	Winding *					Reverse( ) const;
	void						ReverseSelf( );
	void						RemoveEqualPoints( const float epsilon = ON_EPSILON );
	void						RemoveColinearPoints( const Vec3 & normal, const float epsilon = ON_EPSILON );
	void						RemovePoint( int point );
	void						InsertPoint( const Vec3 & point, int spot );
	bool						InsertPointIfOnEdge( const Vec3 & point, const Plane & plane, const float epsilon = ON_EPSILON, int * index = NULL );

	/// Adds the given winding to the convex hull. Assumes the current winding already is a convex hull with three or more points.
	void						AddToConvexHull( const Winding *winding, const Vec3 & normal, const float epsilon = ON_EPSILON );

	///   Add a point to the convex hull. The current winding must be convex but may be degenerate and can have less than three points.
	void						AddToConvexHull( const Vec3 & point, const Vec3 & normal, const float epsilon = ON_EPSILON );

	/// tries to merge 'this' with the given winding, returns NULL if merge fails, both 'this' and 'w' stay intact
	/// 'keep' tells if the contacting points should stay even if they create colinear edges
	Winding *					TryMerge( const Winding & w, const Vec3 & normal, int keep = false ) const;

	/// The parameter indices should point to an array with at least GetNumPoints( ) * 3 integers.
	/// If no triangle fan could be created from one of the corners an assumed center of the
	/// winding is used to create a fan and the indices referncing this additional vertex will
	/// equal GetNumPoints( ) and indices[0] will always be equal to GetNumPoints( ).
	/// Returns the number of indices written out.
	int							CreateTriangles( int * indices, const float epsilon ) const;

	/// check whether the winding is valid or not
	bool						Check( bool print = true ) const;

	float						GetArea( ) const;
	Vec3						GetCenter( ) const;
	Vec3						GetNormal( ) const;
	float						GetRadius( const Vec3 &center ) const;
	void						GetPlane( Vec3 & normal, float &dist ) const;
	void						GetPlane( Plane & plane ) const;
	void						GetBounds( Bounds &bounds ) const;

	Plane						GetPlane( ) const;
	Bounds						GetBounds( ) const;

	bool						IsTiny( ) const;

	/// base winding for a plane is typically huge
	bool						IsHuge( ) const;

	bool						IsTiny( float epsilon ) const;
	bool						IsHuge( float radius ) const;

	void						Print( ) const;

	float						PlaneDistance( const Plane & plane ) const;
	int							PlaneSide( const Plane & plane, const float epsilon = ON_EPSILON ) const;

	bool						PlanesConcave( const Winding & w2, const Vec3 & normal1, const Vec3 & normal2, float dist1, float dist2 ) const;

	bool						PointInside( const Vec3 & normal, const Vec3 & point, const float epsilon ) const;

	/// returns true if the line or ray intersects the winding
	bool						LineIntersection( const Plane &windingPlane, const Vec3 &start, const Vec3 &end, bool backFaceCull = false ) const;

	/// intersection point is start + dir * scale
	bool						RayIntersection( const Plane &windingPlane, const Vec3 &start, const Vec3 &dir, float &scale, bool backFaceCull = false ) const;

	static float				TriangleArea( const Vec3 &a, const Vec3 &b, const Vec3 &c );

	virtual bool				ReAllocate( int n, bool keep = false );
};

#define	MAX_POINTS_ON_WINDING	64

// // //// // //// // //// //
/// GrowingWinding
// //// // //// // //// //

class GrowingWinding : public Winding {
private:
	/// point data
	Vec5						data[ MAX_POINTS_ON_WINDING ];

public:
								GrowingWinding( );
								~GrowingWinding( );

	virtual void				Clear( );
	virtual bool				ReAllocate( int n, bool keep = false );
};

// // //// // //// // //// //
/// FixedWinding
//
/// FixedWinding is a fixed buffer size winding not using memory allocations.
/// When an operation would overflow the fixed buffer a warning is printed and the operation is safely cancelled.
// //// // //// // //// //

class FixedWinding : public Winding {
protected:
	/// point data
	Vec5						data[ MAX_POINTS_ON_WINDING ];

public:
								FixedWinding( );
								explicit FixedWinding( const int n );
								explicit FixedWinding( const Vec3 *verts, const int n );
								explicit FixedWinding( const Vec3 & normal, const float dist, const float radius );
								explicit FixedWinding( const Plane & plane, const float radius );
								explicit FixedWinding( const Winding & winding );
								explicit FixedWinding( const FixedWinding & winding );
	virtual						~FixedWinding( );

	FixedWinding &				operator=( const Winding & winding );

	virtual void				Clear( );

	/// chops off the part of the winding at the back of the plane, if there is nothing at the front the number of points is set to zero, returns a SIDE_?
	int							SplitInPlace( const Plane & plane, const float epsilon, FixedWinding *back );

	virtual bool				ReAllocate( int n, bool keep = false );
};

// // //// // //// // //// //
// Winding
// //// // //// // //// //

INLINE Winding::Winding( ) {
	numPoints = allocedSize = 0;
	p = NULL;
}

INLINE Winding::Winding( int n ) {
	numPoints = allocedSize = 0;
	p = NULL;
	EnsureAlloced( n );
}

INLINE Winding::Winding( const Vec3 * verts, const int n ) {
	int i;
	numPoints = allocedSize = 0;
	p = NULL;
	if( !EnsureAlloced( n ) ) {
		numPoints = 0;
		return;
	}
	for( i = 0; i < n; i++ ) {

		p[ i ].ToVec3( ) = verts[ i ];
		p[ i ].s = p[ i ].t = 0.0f;
	}
	numPoints = n;
}

INLINE Winding::Winding( const Vec3 & normal, const float dist, const float radius ) {
	numPoints = allocedSize = 0;
	p = NULL;
	BaseForPlane( normal, dist, radius );
}

INLINE Winding::Winding( const Plane & plane, const float radius ) {
	numPoints = allocedSize = 0;
	p = NULL;
	BaseForPlane( plane, radius );
}

INLINE Winding::Winding( const Winding & winding ) {
	numPoints = allocedSize = 0;
	p = NULL;

	if( !EnsureAlloced( winding.GetNumPoints( ) ) ) {
		numPoints = 0;
		return;
	}
	for( int i = 0; i < winding.GetNumPoints( ); i++ )
		p[ i ] = winding[ i ];
	numPoints = winding.GetNumPoints( );
}

INLINE Winding::~Winding( ) {
	delete[ ] p;
	p = NULL;
}

INLINE Winding & Winding::operator=( const Winding & winding ) {
	if( !EnsureAlloced( winding.numPoints ) ) {
		numPoints = 0;
		return *this;
	}
	for( int i = 0; i < winding.numPoints; i++ )
		p[ i ] = winding.p[ i ];
	numPoints = winding.numPoints;
	return *this;
}

INLINE const Vec5 &Winding::operator[ ]( const int index ) const {
	assert( index >= 0 && index < numPoints );
	return p[ index ];
}

INLINE Vec5 &Winding::operator[ ]( const int index ) {
	assert( index >= 0 && index < numPoints );
	return p[ index ];
}

INLINE Winding & Winding::operator+=( const Vec3 & v ) {
	AddPoint( v );
	return *this;
}

INLINE Winding & Winding::operator+=( const Vec5 & v ) {
	AddPoint( v );
	return *this;
}

INLINE void Winding::AddPoint( const Vec3 & v ) {
	if( !EnsureAlloced( numPoints + 1, true ) )
		return;
	p[ numPoints ] = v;
	numPoints++;
}

INLINE void Winding::AddPoint( const Vec5 & v ) {
	if( !EnsureAlloced( numPoints + 1, true ) )
		return;
	p[ numPoints ] = v;
	numPoints++;
}

INLINE int Winding::GetNumPoints( ) const {
	return numPoints;
}

INLINE void Winding::SetNumPoints( int n ) {
	if( !EnsureAlloced( n, true ) )
		return;
	numPoints = n;
}

INLINE void Winding::Clear( ) {
	numPoints = 0;
	allocedSize = 0;
	delete[ ] p;
	p = NULL;
}

INLINE void Winding::BaseForPlane( const Plane & plane, const float radius ) {
	BaseForPlane( plane.Normal( ), plane.Dist( ), radius );
}

INLINE bool Winding::EnsureAlloced( int n, bool keep ) {
	if( n > allocedSize )
		return ReAllocate( n, keep );
	return true;
}

INLINE void Winding::Rotate( const Vec3 & origin, const Mat3 & axis ) {
	for( int i = 0; i < numPoints; i++ ) {
		p[ i ].ToVec3( ) -= origin;
		p[ i ].ToVec3( ) *= axis;
		p[ i ].ToVec3( ) += origin;
	}
}

INLINE float Winding::TriangleArea( const Vec3 & a, const Vec3 & b, const Vec3 & c ) {
	Vec3 v1 = b - a;
	Vec3 v2 = c - a;
	Vec3 cross = v1.Cross( v2 );
	return 0.5f * cross.Length( );
}

// // //// // //// // //// //
// GrowingWinding
// //// // //// // //// //

INLINE GrowingWinding::GrowingWinding( ) {
	numPoints = 0;
	p = data;
	allocedSize = MAX_POINTS_ON_WINDING;
}

INLINE GrowingWinding::~GrowingWinding( ) {
	if( p == data )
		p = NULL;
}

INLINE void GrowingWinding::Clear( ) {
	numPoints = 0;
	allocedSize = MAX_POINTS_ON_WINDING;
	if( p != data )
		delete[ ] p;
	p = data;
}

// // //// // //// // //// //
// FixedWinding
// //// // //// // //// //

INLINE FixedWinding::FixedWinding( ) {
	numPoints = 0;
	p = data;
	allocedSize = MAX_POINTS_ON_WINDING;
}

INLINE FixedWinding::FixedWinding( int n ) {
	numPoints = 0;
	p = data;
	allocedSize = MAX_POINTS_ON_WINDING;
}

INLINE FixedWinding::FixedWinding( const Vec3 * verts, const int n ) {
	int i;
	numPoints = 0;
	p = data;
	allocedSize = MAX_POINTS_ON_WINDING;
	if( !EnsureAlloced( n ) ) {
		numPoints = 0;
		return;
	}
	for( i = 0; i < n; i++ ) {
		p[ i ].ToVec3( ) = verts[ i ];
		p[ i ].s = p[ i ].t = 0;
	}
	numPoints = n;
}

INLINE FixedWinding::FixedWinding( const Vec3 & normal, const float dist, const float radius ) {
	numPoints = 0;
	p = data;
	allocedSize = MAX_POINTS_ON_WINDING;
	BaseForPlane( normal, dist, radius );
}

INLINE FixedWinding::FixedWinding( const Plane & plane, const float radius ) {
	numPoints = 0;
	p = data;
	allocedSize = MAX_POINTS_ON_WINDING;
	BaseForPlane( plane, radius );
}

INLINE FixedWinding::FixedWinding( const Winding & winding ) {
	p = data;
	allocedSize = MAX_POINTS_ON_WINDING;
	if( !EnsureAlloced( winding.GetNumPoints( ) ) ) {
		numPoints = 0;
		return;
	}
	for( int i = 0; i < winding.GetNumPoints( ); i++ )
		p[ i ] = winding[ i ];
	numPoints = winding.GetNumPoints( );
}

INLINE FixedWinding::FixedWinding( const FixedWinding & winding ) {
	p = data;
	allocedSize = MAX_POINTS_ON_WINDING;
	if( !EnsureAlloced( winding.GetNumPoints( ) ) ) {
		numPoints = 0;
		return;
	}
	for( int i = 0; i < winding.GetNumPoints( ); i++ )
		p[ i ] = winding[ i ];
	numPoints = winding.GetNumPoints( );
}

INLINE FixedWinding::~FixedWinding( ) {
	p = NULL;	// otherwise it tries to free the fixed buffer
}

INLINE FixedWinding &FixedWinding::operator=( const Winding & winding ) {
	if( !EnsureAlloced( winding.GetNumPoints( ) ) ) {
		numPoints = 0;
		return *this;
	}
	for( int i = 0; i < winding.GetNumPoints( ); i++ )
		p[ i ] = winding[ i ];
	numPoints = winding.GetNumPoints( );
	return *this;
}

INLINE void FixedWinding::Clear( ) {
	numPoints = 0;
}

#define	MAX_POINTS_ON_WINDING_2D		16

// // //// // //// // //// //
/// Winding2D
//
/// A 2D winding is an arbitrary convex 2D polygon defined by an array of points.
// //// // //// // //// //

class Winding2D {
private:
	int						d_numPoints;
	Vec2					d_points[ MAX_POINTS_ON_WINDING_2D ];

public:
							Winding2D( );

	Winding2D &				operator=( const Winding2D & winding );
	const Vec2 &			operator[ ]( const int index ) const;
	Vec2 &					operator[ ]( const int index );
	void					Clear( );
	void					AddPoint( const Vec2 & point );
	int						GetNumPoints( ) const;
	void					Expand( const float d );
	void					ExpandForAxialBox( const Bounds2D & bounds );
	/// splits the winding into a front and back winding, the winding itself stays unchanged, returns a SIDE_?
	int						Split( const Plane2D & plane, const float epsilon, Winding2D ** front, Winding2D ** back ) const;
	/// cuts off the part at the back side of the plane, returns true if some part was at the front
	/// if there is nothing at the front the number of points is set to zero
	bool					ClipInPlace( const Plane2D & plane, const float epsilon = ON_EPSILON, const bool keepOn = false );
	Winding2D *				Copy( ) const;
	Winding2D *				Reverse( ) const;
	float					GetArea( ) const;
	Vec2					GetCenter( ) const;
	float					GetRadius( const Vec2 & center ) const;
	void					GetBounds( Bounds2D & bounds ) const;

	bool					IsTiny( ) const;
	bool					IsHuge( ) const;	// base winding for a plane is typically huge
	void					Print( ) const;

	float					PlaneDistance( const Plane2D & plane ) const;
	int						PlaneSide( const Plane2D & plane, const float epsilon = ON_EPSILON ) const;

	bool					PointInside( const Vec2 & point, const float epsilon ) const;
	bool					LineIntersection( const Vec2 & start, const Vec2 & end ) const;
	bool					RayIntersection( const Vec2 & start, const Vec2 & dir, float & scale1, float & scale2, int * edgeNums = NULL ) const;
};

// // //// // //// // //// //
// Winding2D
// //// // //// // //// //

INLINE Winding2D::Winding2D( ) {
	d_numPoints = 0;
}

INLINE Winding2D & Winding2D::operator=( const Winding2D & winding ) {
	for( int i = 0; i < winding.d_numPoints; i++ )
		d_points[ i ] = winding.d_points[ i ];
	d_numPoints = winding.d_numPoints;
	return *this;
}

INLINE const Vec2 & Winding2D::operator[ ]( const int index ) const {
	return d_points[ index ];
}

INLINE Vec2 & Winding2D::operator[ ]( const int index ) {
	return d_points[ index ];
}

INLINE void Winding2D::Clear( ) {
	d_numPoints = 0;
}

INLINE void Winding2D::AddPoint( const Vec2 & point ) {
	d_points[ d_numPoints++ ] = point;
}

INLINE int Winding2D::GetNumPoints( ) const {
	return d_numPoints;
}

#endif
