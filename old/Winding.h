// Copyright (C) 2004 Id Software, Inc.
//

#ifndef __WINDING_H__
#define __WINDING_H__

// maximum world size
#define MAX_WORLD_COORD			( 128 * 1024 )
#define MIN_WORLD_COORD			( -128 * 1024 )
#define MAX_WORLD_SIZE			( MAX_WORLD_COORD - MIN_WORLD_COORD )

/*
===============================================================================

	A winding is an arbitrary convex polygon defined by an array of points.

===============================================================================
*/

class Winding {

public:
					Winding( void );
					explicit Winding( const int n );								// allocate for n points
					explicit Winding( const Vec3 *verts, const int n );			// winding from points
					explicit Winding( const Vec3 &normal, const float dist );	// base winding for plane
					explicit Winding( const Plane &plane );						// base winding for plane
					explicit Winding( const Winding &winding );
	virtual			~Winding( void );

	Winding &		operator=( const Winding &winding );
	const Vec5 &	operator[]( const int index ) const;
	Vec5 &			operator[]( const int index );

					// add a point to the end of the winding point array
	Winding &		operator+=( const Vec3 &v );
	Winding &		operator+=( const Vec5 &v );
	void			AddPoint( const Vec3 &v );
	void			AddPoint( const Vec5 &v );

					// number of points on winding
	int				GetNumPoints( void ) const;
	void			SetNumPoints( int n );
	virtual void	Clear( void );

					// huge winding for plane, the points go counter clockwise when facing the front of the plane
	void			BaseForPlane( const Vec3 &normal, const float dist );
	void			BaseForPlane( const Plane &plane );

					// splits the winding into a front and back winding, the winding itself stays unchanged
					// returns a SIDE_?
	int				Split( const Plane &plane, const float epsilon, Winding **front, Winding **back ) const;
					// returns the winding fragment at the front of the clipping plane,
					// if there is nothing at the front the winding itself is destroyed and NULL is returned
	Winding *		Clip( const Plane &plane, const float epsilon = ON_EPSILON, const bool keepOn = false );
					// cuts off the part at the back side of the plane, returns true if some part was at the front
					// if there is nothing at the front the number of points is set to zero
	bool			ClipInPlace( const Plane &plane, const float epsilon = ON_EPSILON, const bool keepOn = false );

					// returns a copy of the winding
	Winding *		Copy( void ) const;
	Winding *		Reverse( void ) const;
	Winding &		ReverseSelf( void );
	void			RemoveEqualPoints( const float epsilon = ON_EPSILON );
	void			RemoveColinearPoints( const Vec3 &normal, const float epsilon = ON_EPSILON );
	void			RemovePoint( int point );
	void			InsertPoint( const Vec3 &point, int spot );
	bool			InsertPointIfOnEdge( const Vec3 &point, const Plane &plane, const float epsilon = ON_EPSILON );
					// add a winding to the convex hull
	void			AddToConvexHull( const Winding *winding, const Vec3 &normal, const float epsilon = ON_EPSILON );
					// add a point to the convex hull
	void			AddToConvexHull( const Vec3 &point, const Vec3 &normal, const float epsilon = ON_EPSILON );
					// tries to merge 'this' with the given winding, returns NULL if merge fails, both 'this' and 'w' stay intact
					// 'keep' tells if the contacting points should stay even if they create colinear edges
	Winding *		TryMerge( const Winding &w, const Vec3 &normal, int keep = false ) const;
					// check whether the winding is valid or not
	bool			Check( bool print = true ) const;

	float			GetArea( void ) const;
	Vec3			GetCenter( void ) const;
	float			GetRadius( const Vec3 &center ) const;
	void			GetPlane( Vec3 &normal, float &dist ) const;
	void			GetPlane( Plane &plane ) const;
	void			GetBounds( Bounds &bounds ) const;

	bool			IsTiny( void ) const;
	bool			IsHuge( void ) const;	// base winding for a plane is typically huge
	void			Print( void ) const;

	float			PlaneDistance( const Plane &plane ) const;
	int				PlaneSide( const Plane &plane, const float epsilon = ON_EPSILON ) const;

	bool			PlanesConcave( const Winding &w2, const Vec3 &normal1, const Vec3 &normal2, float dist1, float dist2 ) const;

	bool			PointInside( const Vec3 &normal, const Vec3 &point, const float epsilon ) const;
					// returns true if the line or ray intersects the winding
	bool			LineIntersection( const Plane &windingPlane, const Vec3 &start, const Vec3 &end, bool backFaceCull = false ) const;
					// intersection point is start + dir * scale
	bool			RayIntersection( const Plane &windingPlane, const Vec3 &start, const Vec3 &dir, float &scale, bool backFaceCull = false ) const;

	static float	TriangleArea( const Vec3 &a, const Vec3 &b, const Vec3 &c );

protected:
	int				numPoints;				// number of points
	Vec5 *			p;						// pointer to point data
	int				allocedSize;

	bool			EnsureAlloced( int n, bool keep = false );
	virtual bool	ReAllocate( int n, bool keep = false );
};

INLINE Winding::Winding( void ) {
	numPoints = allocedSize = 0;
	p = NULL;
}

INLINE Winding::Winding( int n ) {
	numPoints = allocedSize = 0;
	p = NULL;
	EnsureAlloced( n );
}

INLINE Winding::Winding( const Vec3 *verts, const int n ) {
	int i;

	numPoints = allocedSize = 0;
	p = NULL;
	if ( !EnsureAlloced( n ) ) {
		numPoints = 0;
		return;
	}
	for ( i = 0; i < n; i++ ) {
		p[i].ToVec3() = verts[i];
		p[i].s = p[i].t = 0.0f;
	}
	numPoints = n;
}

INLINE Winding::Winding( const Vec3 &normal, const float dist ) {
	numPoints = allocedSize = 0;
	p = NULL;
	BaseForPlane( normal, dist );
}

INLINE Winding::Winding( const Plane &plane ) {
	numPoints = allocedSize = 0;
	p = NULL;
	BaseForPlane( plane );
}

INLINE Winding::Winding( const Winding &winding ) {
	int i;
	if ( !EnsureAlloced( winding.GetNumPoints() ) ) {
		numPoints = 0;
		return;
	}
	for ( i = 0; i < winding.GetNumPoints(); i++ ) {
		p[i] = winding[i];
	}
	numPoints = winding.GetNumPoints();
}

INLINE Winding::~Winding( void ) {
	delete[] p;
	p = NULL;
}

INLINE Winding &Winding::operator=( const Winding &winding ) {
	int i;

	if ( !EnsureAlloced( winding.numPoints ) ) {
		numPoints = 0;
		return *this;
	}
	for ( i = 0; i < winding.numPoints; i++ ) {
		p[i] = winding.p[i];
	}
	numPoints = winding.numPoints;
	return *this;
}

INLINE const Vec5 &Winding::operator[]( const int index ) const {
	//assert( index >= 0 && index < numPoints );
	return p[ index ];
}

INLINE Vec5 &Winding::operator[]( const int index ) {
	//assert( index >= 0 && index < numPoints );
	return p[ index ];
}

INLINE Winding &Winding::operator+=( const Vec3 &v ) {
	AddPoint( v );
	return *this;
}

INLINE Winding &Winding::operator+=( const Vec5 &v ) {
	AddPoint( v );
	return *this;
}

INLINE void Winding::AddPoint( const Vec3 &v ) {
	if ( !EnsureAlloced(numPoints+1, true) ) {
		return;
	}
	p[numPoints] = v;
	numPoints++;
}

INLINE void Winding::AddPoint( const Vec5 &v ) {
	if ( !EnsureAlloced(numPoints+1, true) ) {
		return;
	}
	p[numPoints] = v;
	numPoints++;
}

INLINE int Winding::GetNumPoints( void ) const {
	return numPoints;
}

INLINE void Winding::SetNumPoints( int n ) {
	if ( !EnsureAlloced( n, true ) ) {
		return;
	}
	numPoints = n;
}

INLINE void Winding::Clear( void ) {
	numPoints = 0;
	delete[] p;
	p = NULL;
}

INLINE void Winding::BaseForPlane( const Plane &plane ) {
	BaseForPlane( plane.Normal(), plane.Dist() );
}

INLINE bool Winding::EnsureAlloced( int n, bool keep ) {
	if ( n > allocedSize ) {
		return ReAllocate( n, keep );
	}
	return true;
}


/*
===============================================================================

	FixedWinding is a fixed buffer size winding not using
	memory allocations.

	When an operation would overflow the fixed buffer a warning
	is printed and the operation is safely cancelled.

===============================================================================
*/

#define	MAX_POINTS_ON_WINDING	64

class FixedWinding : public Winding {

public:
					FixedWinding( void );
					explicit FixedWinding( const int n );
					explicit FixedWinding( const Vec3 *verts, const int n );
					explicit FixedWinding( const Vec3 &normal, const float dist );
					explicit FixedWinding( const Plane &plane );
					explicit FixedWinding( const Winding &winding );
					explicit FixedWinding( const FixedWinding &winding );
	virtual			~FixedWinding( void );

	FixedWinding &operator=( const Winding &winding );

	virtual void	Clear( void );

					// splits the winding in a back and front part, 'this' becomes the front part
					// returns a SIDE_?
	int				Split( FixedWinding *back, const Plane &plane, const float epsilon = ON_EPSILON );

protected:
	Vec5			data[MAX_POINTS_ON_WINDING];	// point data

	virtual bool	ReAllocate( int n, bool keep = false );
};

INLINE FixedWinding::FixedWinding( void ) {
	numPoints = 0;
	p = data;
	allocedSize = MAX_POINTS_ON_WINDING;
}

INLINE FixedWinding::FixedWinding( int n ) {
	numPoints = 0;
	p = data;
	allocedSize = MAX_POINTS_ON_WINDING;
}

INLINE FixedWinding::FixedWinding( const Vec3 *verts, const int n ) {
	int i;

	numPoints = 0;
	p = data;
	allocedSize = MAX_POINTS_ON_WINDING;
	if ( !EnsureAlloced( n ) ) {
		numPoints = 0;
		return;
	}
	for ( i = 0; i < n; i++ ) {
		p[i].ToVec3() = verts[i];
		p[i].s = p[i].t = 0;
	}
	numPoints = n;
}

INLINE FixedWinding::FixedWinding( const Vec3 &normal, const float dist ) {
	numPoints = 0;
	p = data;
	allocedSize = MAX_POINTS_ON_WINDING;
	BaseForPlane( normal, dist );
}

INLINE FixedWinding::FixedWinding( const Plane &plane ) {
	numPoints = 0;
	p = data;
	allocedSize = MAX_POINTS_ON_WINDING;
	BaseForPlane( plane );
}

INLINE FixedWinding::FixedWinding( const Winding &winding ) {
	int i;

	p = data;
	allocedSize = MAX_POINTS_ON_WINDING;
	if ( !EnsureAlloced( winding.GetNumPoints() ) ) {
		numPoints = 0;
		return;
	}
	for ( i = 0; i < winding.GetNumPoints(); i++ ) {
		p[i] = winding[i];
	}
	numPoints = winding.GetNumPoints();
}

INLINE FixedWinding::FixedWinding( const FixedWinding &winding ) {
	int i;

	p = data;
	allocedSize = MAX_POINTS_ON_WINDING;
	if ( !EnsureAlloced( winding.GetNumPoints() ) ) {
		numPoints = 0;
		return;
	}
	for ( i = 0; i < winding.GetNumPoints(); i++ ) {
		p[i] = winding[i];
	}
	numPoints = winding.GetNumPoints();
}

INLINE FixedWinding::~FixedWinding( void ) {
	p = NULL;	// otherwise it tries to free the fixed buffer
}

INLINE FixedWinding &FixedWinding::operator=( const Winding &winding ) {
	int i;

	if ( !EnsureAlloced( winding.GetNumPoints() ) ) {
		numPoints = 0;
		return *this;
	}
	for ( i = 0; i < winding.GetNumPoints(); i++ ) {
		p[i] = winding[i];
	}
	numPoints = winding.GetNumPoints();
	return *this;
}

INLINE void FixedWinding::Clear( void ) {
	numPoints = 0;
}
#endif	/* !__WINDING_H__ */
