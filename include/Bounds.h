#ifndef BOUNDS_H
#define BOUNDS_H

/*
===============================================================================

	Axis Aligned Bounding Box

===============================================================================
*/

class Bounds {
public:
					Bounds( );
					explicit Bounds( const Vec3 &mins, const Vec3 &maxs );
					explicit Bounds( const Vec3 &point );

	const Vec3 &	operator[]( const int index ) const;
	Vec3 &			operator[]( const int index );
	Bounds			operator+( const Vec3 &t ) const;				// returns translated bounds
	Bounds &		operator+=( const Vec3 &t );					// translate the bounds
	Bounds			operator*( const Mat3 &r ) const;				// returns rotated bounds
	Bounds &		operator*=( const Mat3 &r );					// rotate the bounds
	Bounds			operator+( const Bounds &a ) const;
	Bounds &		operator+=( const Bounds &a );
	Bounds			operator-( const Bounds &a ) const;
	Bounds &		operator-=( const Bounds &a );

	bool			Compare( const Bounds &a ) const;							// exact compare, no epsilon
	bool			Compare( const Bounds &a, const float epsilon ) const;	// compare with epsilon
	bool			operator==(	const Bounds &a ) const;						// exact compare, no epsilon
	bool			operator!=(	const Bounds &a ) const;						// exact compare, no epsilon

	void			Clear( );									// inside out bounds
	void			Zero( );									// single point at origin

	Vec3			GetCenter( ) const;						// returns center of bounds
	float			GetRadius( ) const;						// returns the radius relative to the bounds origin
	float			GetRadius( const Vec3 &center ) const;		// returns the radius relative to the given center
	float			GetVolume( ) const;						// returns the volume of the bounds
	bool			IsCleared( ) const;						// returns true if bounds are inside out

	bool			AddPoint( const Vec3 &v );					// add the point, returns true if the bounds expanded
	bool			AddBounds( const Bounds &a );					// add the bounds, returns true if the bounds expanded
	Bounds			Intersect( const Bounds &a ) const;			// return intersection of this bounds with the given bounds
	Bounds &		IntersectSelf( const Bounds &a );				// intersect this bounds with the given bounds
	Bounds			Expand( const float d ) const;					// return bounds expanded in all directions with the given value
	Bounds &		ExpandSelf( const float d );					// expand bounds in all directions with the given value
	Bounds			Translate( const Vec3 &translation ) const;	// return translated bounds
	Bounds &		TranslateSelf( const Vec3 &translation );		// translate this bounds
	Bounds			Rotate( const Mat3 &rotation ) const;			// return rotated bounds
	Bounds &		RotateSelf( const Mat3 &rotation );			// rotate this bounds

	float			PlaneDistance( const Plane &plane ) const;
	int				PlaneSide( const Plane &plane, const float epsilon = ON_EPSILON ) const;

	bool			ContainsPoint( const Vec3 &p ) const;			// includes touching
	bool			IntersectsBounds( const Bounds &a ) const;	// includes touching
	bool			LineIntersection( const Vec3 &start, const Vec3 &end ) const;
					// intersection point is start + dir * scale
	bool			RayIntersection( const Vec3 &start, const Vec3 &dir, float &scale ) const;

					// most tight bounds for the given transformed bounds
	void			FromTransformedBounds( const Bounds &bounds, const Vec3 &origin, const Mat3 &axis );
					// most tight bounds for a point set
	void			FromPoints( const Vec3 *points, const int numPoints );
					// most tight bounds for a translation
	void			FromPointTranslation( const Vec3 &point, const Vec3 &translation );
	void			FromBoundsTranslation( const Bounds &bounds, const Vec3 &origin, const Mat3 &axis, const Vec3 &translation );
					// most tight bounds for a rotation
	void			FromPointRotation( const Vec3 &point, const Rotation &rotation );
	void			FromBoundsRotation( const Bounds &bounds, const Vec3 &origin, const Mat3 &axis, const Rotation &rotation );

	void			ToPoints( Vec3 points[8] ) const;
	void			ToPlanes( Plane planes[ 6 ] ) const;
	Sphere			ToSphere( ) const;

	void			AxisProjection( const Vec3 &dir, float &min, float &max ) const;
	void			AxisProjection( const Vec3 &origin, const Mat3 &axis, const Vec3 &dir, float &min, float &max ) const;

private:
	Vec3			b[2];
};

extern Bounds	bounds_zero;

INLINE Bounds::Bounds( ) {
}

INLINE Bounds::Bounds( const Vec3 &mins, const Vec3 &maxs ) {
	b[0] = mins;
	b[1] = maxs;
}

INLINE Bounds::Bounds( const Vec3 &point ) {
	b[0] = point;
	b[1] = point;
}

INLINE const Vec3 &Bounds::operator[]( const int index ) const {
	return b[index];
}

INLINE Vec3 &Bounds::operator[]( const int index ) {
	return b[index];
}

INLINE Bounds Bounds::operator+( const Vec3 &t ) const {
	return Bounds( b[0] + t, b[1] + t );
}

INLINE Bounds &Bounds::operator+=( const Vec3 &t ) {
	b[0] += t;
	b[1] += t;
	return *this;
}

INLINE Bounds Bounds::operator*( const Mat3 &r ) const {
	Bounds bounds;
	bounds.FromTransformedBounds( *this, vec3_origin, r );
	return bounds;
}

INLINE Bounds &Bounds::operator*=( const Mat3 &r ) {
	this->FromTransformedBounds( *this, vec3_origin, r );
	return *this;
}

INLINE Bounds Bounds::operator+( const Bounds &a ) const {
	Bounds newBounds;
	newBounds = *this;
	newBounds.AddBounds( a );
	return newBounds;
}

INLINE Bounds &Bounds::operator+=( const Bounds &a ) {
	Bounds::AddBounds( a );
	return *this;
}

INLINE Bounds Bounds::operator-( const Bounds &a ) const {
	assert( b[1][0] - b[0][0] > a.b[1][0] - a.b[0][0] &&
				b[1][1] - b[0][1] > a.b[1][1] - a.b[0][1] &&
					b[1][2] - b[0][2] > a.b[1][2] - a.b[0][2] );
	return Bounds( Vec3( b[0][0] + a.b[1][0], b[0][1] + a.b[1][1], b[0][2] + a.b[1][2] ),
					Vec3( b[1][0] + a.b[0][0], b[1][1] + a.b[0][1], b[1][2] + a.b[0][2] ) );
}

INLINE Bounds &Bounds::operator-=( const Bounds &a ) {
	assert( b[1][0] - b[0][0] > a.b[1][0] - a.b[0][0] &&
				b[1][1] - b[0][1] > a.b[1][1] - a.b[0][1] &&
					b[1][2] - b[0][2] > a.b[1][2] - a.b[0][2] );
	b[0] += a.b[1];
	b[1] += a.b[0];
	return *this;
}

INLINE bool Bounds::Compare( const Bounds &a ) const {
	return ( b[0].Compare( a.b[0] ) && b[1].Compare( a.b[1] ) );
}

INLINE bool Bounds::Compare( const Bounds &a, const float epsilon ) const {
	return ( b[0].Compare( a.b[0], epsilon ) && b[1].Compare( a.b[1], epsilon ) );
}

INLINE bool Bounds::operator==( const Bounds &a ) const {
	return Compare( a );
}

INLINE bool Bounds::operator!=( const Bounds &a ) const {
	return !Compare( a );
}

INLINE void Bounds::Clear( ) {
	b[0][0] = b[0][1] = b[0][2] = INFINITY;
	b[1][0] = b[1][1] = b[1][2] = -INFINITY;
}

INLINE void Bounds::Zero( ) {
	b[0][0] = b[0][1] = b[0][2] =
	b[1][0] = b[1][1] = b[1][2] = 0;
}

INLINE Vec3 Bounds::GetCenter( ) const {
	return Vec3( ( b[1][0] + b[0][0] ) * 0.5f, ( b[1][1] + b[0][1] ) * 0.5f, ( b[1][2] + b[0][2] ) * 0.5f );
}

INLINE float Bounds::GetVolume( ) const {
	if ( b[0][0] >= b[1][0] || b[0][1] >= b[1][1] || b[0][2] >= b[1][2] ) {
		return 0.0f;
	}
	return ( ( b[1][0] - b[0][0] ) * ( b[1][1] - b[0][1] ) * ( b[1][2] - b[0][2] ) );
}

INLINE bool Bounds::IsCleared( ) const {
	return b[0][0] > b[1][0];
}

INLINE bool Bounds::AddPoint( const Vec3 &v ) {
	bool expanded = false;
	if ( v[0] < b[0][0]) {
		b[0][0] = v[0];
		expanded = true;
	}
	if ( v[0] > b[1][0]) {
		b[1][0] = v[0];
		expanded = true;
	}
	if ( v[1] < b[0][1] ) {
		b[0][1] = v[1];
		expanded = true;
	}
	if ( v[1] > b[1][1]) {
		b[1][1] = v[1];
		expanded = true;
	}
	if ( v[2] < b[0][2] ) {
		b[0][2] = v[2];
		expanded = true;
	}
	if ( v[2] > b[1][2]) {
		b[1][2] = v[2];
		expanded = true;
	}
	return expanded;
}

INLINE bool Bounds::AddBounds( const Bounds &a ) {
	bool expanded = false;
	if ( a.b[0][0] < b[0][0] ) {
		b[0][0] = a.b[0][0];
		expanded = true;
	}
	if ( a.b[0][1] < b[0][1] ) {
		b[0][1] = a.b[0][1];
		expanded = true;
	}
	if ( a.b[0][2] < b[0][2] ) {
		b[0][2] = a.b[0][2];
		expanded = true;
	}
	if ( a.b[1][0] > b[1][0] ) {
		b[1][0] = a.b[1][0];
		expanded = true;
	}
	if ( a.b[1][1] > b[1][1] ) {
		b[1][1] = a.b[1][1];
		expanded = true;
	}
	if ( a.b[1][2] > b[1][2] ) {
		b[1][2] = a.b[1][2];
		expanded = true;
	}
	return expanded;
}

INLINE Bounds Bounds::Intersect( const Bounds &a ) const {
	Bounds n;
	n.b[0][0] = ( a.b[0][0] > b[0][0] ) ? a.b[0][0] : b[0][0];
	n.b[0][1] = ( a.b[0][1] > b[0][1] ) ? a.b[0][1] : b[0][1];
	n.b[0][2] = ( a.b[0][2] > b[0][2] ) ? a.b[0][2] : b[0][2];
	n.b[1][0] = ( a.b[1][0] < b[1][0] ) ? a.b[1][0] : b[1][0];
	n.b[1][1] = ( a.b[1][1] < b[1][1] ) ? a.b[1][1] : b[1][1];
	n.b[1][2] = ( a.b[1][2] < b[1][2] ) ? a.b[1][2] : b[1][2];
	return n;
}

INLINE Bounds &Bounds::IntersectSelf( const Bounds &a ) {
	if ( a.b[0][0] > b[0][0] ) {
		b[0][0] = a.b[0][0];
	}
	if ( a.b[0][1] > b[0][1] ) {
		b[0][1] = a.b[0][1];
	}
	if ( a.b[0][2] > b[0][2] ) {
		b[0][2] = a.b[0][2];
	}
	if ( a.b[1][0] < b[1][0] ) {
		b[1][0] = a.b[1][0];
	}
	if ( a.b[1][1] < b[1][1] ) {
		b[1][1] = a.b[1][1];
	}
	if ( a.b[1][2] < b[1][2] ) {
		b[1][2] = a.b[1][2];
	}
	return *this;
}

INLINE Bounds Bounds::Expand( const float d ) const {
	return Bounds( Vec3( b[0][0] - d, b[0][1] - d, b[0][2] - d ),
						Vec3( b[1][0] + d, b[1][1] + d, b[1][2] + d ) );
}

INLINE Bounds &Bounds::ExpandSelf( const float d ) {
	b[0][0] -= d;
	b[0][1] -= d;
	b[0][2] -= d;
	b[1][0] += d;
	b[1][1] += d;
	b[1][2] += d;
	return *this;
}

INLINE Bounds Bounds::Translate( const Vec3 &translation ) const {
	return Bounds( b[0] + translation, b[1] + translation );
}

INLINE Bounds &Bounds::TranslateSelf( const Vec3 &translation ) {
	b[0] += translation;
	b[1] += translation;
	return *this;
}

INLINE Bounds Bounds::Rotate( const Mat3 &rotation ) const {
	Bounds bounds;
	bounds.FromTransformedBounds( *this, vec3_origin, rotation );
	return bounds;
}

INLINE Bounds &Bounds::RotateSelf( const Mat3 &rotation ) {
	FromTransformedBounds( *this, vec3_origin, rotation );
	return *this;
}

INLINE int Bounds::PlaneSide( const Plane &plane, const float epsilon ) const {

	Vec3 center = ( b[0] + b[1] ) * 0.5f;

	float d1 = plane.Distance( center );
	float d2 =	fabs( ( b[1][0] - center[0] ) * plane.Normal()[0] ) +
				fabs( ( b[1][1] - center[1] ) * plane.Normal()[1] ) +
				fabs( ( b[1][2] - center[2] ) * plane.Normal()[2] );

	if ( d1 - d2 > epsilon ) {

		return PLANESIDE_FRONT;
	}

	if ( d1 + d2 < -epsilon ) {

		return PLANESIDE_BACK;
	}

	return PLANESIDE_CROSS;
}

INLINE bool Bounds::ContainsPoint( const Vec3 &p ) const {
	if ( p[0] < b[0][0] || p[1] < b[0][1] || p[2] < b[0][2]
		|| p[0] > b[1][0] || p[1] > b[1][1] || p[2] > b[1][2] ) {
		return false;
	}
	return true;
}

INLINE bool Bounds::IntersectsBounds( const Bounds &a ) const {
	if ( a.b[1][0] < b[0][0] || a.b[1][1] < b[0][1] || a.b[1][2] < b[0][2]
		|| a.b[0][0] > b[1][0] || a.b[0][1] > b[1][1] || a.b[0][2] > b[1][2] ) {
		return false;
	}
	return true;
}

INLINE Sphere Bounds::ToSphere( ) const {
	Sphere sphere;
	sphere.SetOrigin( ( b[0] + b[1] ) * 0.5f );
	sphere.SetRadius( ( b[1] - sphere.GetOrigin() ).Length() );
	return sphere;
}

INLINE void Bounds::AxisProjection( const Vec3 &dir, float &min, float &max ) const {
	float d1, d2;
	Vec3 center, extents;

	center = ( b[0] + b[1] ) * 0.5f;
	extents = b[1] - center;

	d1 = dir * center;
	d2 = fabs( extents[0] * dir[0] ) +
			fabs( extents[1] * dir[1] ) +
				fabs( extents[2] * dir[2] );

	min = d1 - d2;
	max = d1 + d2;
}

INLINE void Bounds::AxisProjection( const Vec3 &origin, const Mat3 &axis, const Vec3 &dir, float &min, float &max ) const {
	float d1, d2;
	Vec3 center, extents;

	center = ( b[0] + b[1] ) * 0.5f;
	extents = b[1] - center;
	center = origin + center * axis;

	d1 = dir * center;
	d2 = fabs( extents[0] * ( dir * axis[0] ) ) +
			fabs( extents[1] * ( dir * axis[1] ) ) +
				fabs( extents[2] * ( dir * axis[2] ) );

	min = d1 - d2;
	max = d1 + d2;
}

INLINE void Bounds::ToPoints( Vec3 points[ 8 ] ) const {

	for( int i = 0; i < 8; i++ ) {

		points[ i ][ 0 ] = b[ ( i ^ ( i >> 1 ) ) & 1 ][ 0 ];
		points[ i ][ 1 ] = b[ ( i >> 1 ) & 1 ][ 1 ];
		points[ i ][ 2 ] = b[ ( i >> 2 ) & 1 ][ 2 ];
	}
}

INLINE void Bounds::ToPlanes( Plane planes[ 6 ] ) const {

	float value;

	for( int i = 0; i < 6; i++ ) {

		int iMod3 =				i % 3;
		bool negativeSide =		i < 3;
		if( !iMod3 )			value = negativeSide ? -1.0f : 1.0f;
		else					planes[ i ][ 0 ] = 0.0f;
		if( iMod3 != 1 )		planes[ i ][ 1 ] = 0.0f;
		if( iMod3 != 2 )		planes[ i ][ 2 ] = 0.0f;
		planes[ i ][ iMod3 ] =	value;
		planes[ i ][ 3 ] =		negativeSide ? b[ !negativeSide ][ iMod3 ] : -b[ !negativeSide ][ iMod3 ];
	}
}

// // //// // //// // //// //
/// Bounds2D
// //// // //// // //// //

class Bounds2D {
private:
	Vec2					d_border[ 2 ];

public:

							Bounds2D( );
							explicit Bounds2D( const Vec2 & mins, const Vec2 & maxs );
							explicit Bounds2D( float min_x, float min_y, float max_x, float max_y );
	/// returns true if 'a' box fits in caller box
	bool					operator >=( const Bounds2D & a ) const;
	/// returns true if 'a' box smaller or equal than caller box
	bool					operator <=( const Bounds2D & a ) const;
	const Vec2 &			operator[ ]( const int index ) const;
	Vec2 &					operator[ ]( const int index );
	/// most tight bounds for the given transformed bounds
	void					FromTransformedBounds( const Bounds2D & bounds, const Vec2 & origin, const Mat2 & axis );
	/// inside out bounds
	void					Clear( );
	/// single point at origin
	void					Zero( );
	/// returns center of bounds
	Vec2					GetCenter( ) const;
	/// returns the radius relative to the bounds origin
	float					GetRadius( ) const;
	/// returns the radius relative to the given center
	float					GetRadius( const Vec2 & center ) const;
	float					GetArea( ) const;
	/// add the point, returns true if the bounds expanded
	bool					AddPoint( const Vec2 & v );
	/// add the bounds, returns true if the bounds expanded
	bool					AddBounds( const Bounds2D & a );
	/// return intersection of this bounds with the given bounds
	Bounds2D				Intersect( const Bounds2D & a ) const;
	/// intersect this bounds with the given bounds
	Bounds2D &				IntersectSelf( const Bounds2D & a );
	/// return bounds expanded in all directions with the given value
	Bounds2D				Expand( const float d ) const;
	/// expand bounds in all directions with the given value
	Bounds2D &				ExpandSelf( const float d );
	/// return translated bounds
	Bounds2D				Translate( const Vec2 & translation ) const;
	/// translate this bounds
	Bounds2D &				TranslateSelf( const Vec2 & translation );
	/// return rotated bounds
	Bounds2D				Rotate( const Mat2 & rotation ) const;
	/// rotate this bounds
	Bounds2D &				RotateSelf( const Mat2 & rotation );
	Bounds2D				FlipEndPoint( ) const;
	Bounds2D &				FlipEndPointSelf( );
	/// includes touching
	bool					ContainsPoint( const Vec2 & p ) const;
	/// includes touching
	bool					IntersectsBounds( const Bounds2D & a ) const;
	bool					LineIntersection( const Vec2 & start, const Vec2 & end ) const;	
	/// intersection point is start + dir * scale
	bool					RayIntersection( const Vec2 & start, const Vec2 & dir, float & scale ) const;
	Vec2					GetUpperRight( ) const;
	Vec2					GetLowerLeft( ) const;
};

// // //// // //// // //// //
// Bounds2D
// //// // //// // //// //

INLINE Bounds2D::Bounds2D( ) {
}

INLINE Bounds2D::Bounds2D( const Vec2 & mins, const Vec2 & maxs ) {
	d_border[ 0 ] = mins;
	d_border[ 1 ] = maxs;
}

INLINE Bounds2D::Bounds2D( float min_x, float min_y, float max_x, float max_y ) {
	d_border[ 0 ][ 0 ] = min_x;
	d_border[ 0 ][ 1 ] = min_y;
	d_border[ 1 ][ 0 ] = max_x;
	d_border[ 1 ][ 1 ] = max_y;
}

INLINE void Bounds2D::FromTransformedBounds( const Bounds2D & bounds, const Vec2 & origin, const Mat2 & axis ) {
	Vec2 center = ( bounds[ 0 ] + bounds[ 1 ] ) * 0.5f;
	Vec2 extents = bounds[ 1 ] - center;
	Vec2 rotatedExtents;
	for( int i = 0; i < 2; i++ )
		rotatedExtents[ i ] = fabs( extents[ 0 ] * axis[ 0 ][ i ] ) + fabs( extents[ 1 ] * axis[ 1 ][ i ] );
	center = origin + center * axis;
	d_border[ 0 ] = center - rotatedExtents;
	d_border[ 1 ] = center + rotatedExtents;
}

INLINE bool Bounds2D::operator >=( const Bounds2D & a ) const {
	Vec2 extentsThis = d_border[ 1 ] - d_border[ 0 ];
	Vec2 extentsA = a.d_border[ 1 ] - a.d_border[ 0 ];	
	if( extentsThis[ 0 ] >= extentsA[ 0 ] && extentsThis[ 1 ] >= extentsA[ 1 ] ) // un-rotated test
		return true;	
	if( extentsThis[ 1 ] >= extentsA[ 0 ] && extentsThis[ 0 ] >= extentsA[ 1 ] ) // second, rotated test
		return true;
	return false; // cant fit
}

INLINE bool Bounds2D::operator <=( const Bounds2D & a ) const {
	Vec2 extentsThis = d_border[ 1 ] - d_border[ 0 ];
	Vec2 extentsA = a.d_border[ 1 ] - a.d_border[ 0 ];	
	if( extentsThis[ 0 ] <= extentsA[ 0 ] && extentsThis[ 1 ] <= extentsA[ 1 ] ) // un-rotated test
		return true;	
	if( extentsThis[ 1 ] <= extentsA[ 0 ] && extentsThis[ 0 ] <= extentsA[ 1 ] ) // second, rotated test
		return true;	
	return false; // out-fit
}

INLINE const Vec2 & Bounds2D::operator [ ]( const int index ) const {
	return d_border[ index ];
}

INLINE Vec2 & Bounds2D::operator[ ]( const int index ) {
	return d_border[ index ];
}

INLINE void Bounds2D::Clear( ) {
	d_border[ 0 ][ 0 ] = d_border[ 0 ][ 1 ] = INFINITY;
	d_border[ 1 ][ 0 ] = d_border[ 1 ][ 1 ] = -INFINITY;
}

INLINE void Bounds2D::Zero( ) {
	d_border[ 0 ][ 0 ] = d_border[ 0 ][ 1 ] =
	d_border[ 1 ][ 0 ] = d_border[ 1 ][ 1 ] = 0;
}

INLINE Vec2 Bounds2D::GetCenter( ) const {
	return Vec2( ( d_border[ 1 ][ 0 ] + d_border[ 0 ][ 0 ] ) * 0.5f, ( d_border[ 1 ][ 1 ] + d_border[ 0 ][ 1 ] ) * 0.5f );
}

INLINE float Bounds2D::GetRadius( ) const {
	float total = 0.0f;
	for( int i = 0; i < 2; i++ ) {
		float b0 = fabs( d_border[ 0 ][ i ] );
		float b1 = fabs( d_border[ 1 ][ i ] );
		if ( b0 > b1 )
			total += b0 * b0;
		else
			total += b1 * b1;
	}
	return sqrtf( total );
}

INLINE float Bounds2D::GetRadius( const Vec2 & center ) const {
	float total = 0.0f;
	for( int i = 0; i < 2; i++ ) {
		float b0 = fabs( center[ i ] - d_border[ 0 ][ i ] );
		float b1 = fabs( d_border[ 1 ][ i ] - center[ i ] );
		if ( b0 > b1 )
			total += b0 * b0;
		else
			total += b1 * b1;
	}
	return sqrtf( total );
}

INLINE float Bounds2D::GetArea( ) const {
	if( d_border[ 0 ][ 0 ] >= d_border[ 1 ][ 0 ] || d_border[ 0 ][ 1 ] >= d_border[ 1 ][ 1 ] )
		return 0.0f;
	return ( ( d_border[ 1 ][ 0 ] - d_border[ 0 ][ 0 ] ) * ( d_border[ 1 ][ 1 ] - d_border[ 0 ][ 1 ] ) );
}

INLINE bool Bounds2D::AddPoint( const Vec2 & v ) {
	bool expanded = false;
	if( v[ 0 ] < d_border[ 0 ][ 0 ] ) {
		d_border[ 0 ][ 0 ] = v[ 0 ];
		expanded = true;
	}
	if( v[ 0 ] > d_border[ 1 ][ 0 ] ) {
		d_border[ 1 ][ 0 ] = v[ 0 ];
		expanded = true;
	}
	if( v[ 1 ] < d_border[ 0 ][ 1 ] ) {
		d_border[ 0 ][ 1 ] = v[ 1 ];
		expanded = true;
	}
	if( v[ 1 ] > d_border[ 1 ][ 1 ] ) {
		d_border[ 1 ][ 1 ] = v[ 1 ];
		expanded = true;
	}
	return expanded;
}

INLINE bool Bounds2D::AddBounds( const Bounds2D & a ) {
	bool expanded = false;
	if( a.d_border[ 0 ][ 0 ] < d_border[ 0 ][ 0 ] ) {
		d_border[ 0 ][ 0 ] = a.d_border[ 0 ][ 0 ];
		expanded = true;
	}
	if( a.d_border[ 0 ][ 1 ] < d_border[ 0 ][ 1 ] ) {
		d_border[ 0 ][ 1 ] = a.d_border[ 0 ][ 1 ];
		expanded = true;
	}
	if( a.d_border[ 0 ][ 2 ] < d_border[ 0 ][ 2 ] ) {
		d_border[ 0 ][ 2 ] = a.d_border[ 0 ][ 2 ];
		expanded = true;
	}
	if( a.d_border[ 1 ][ 0 ] > d_border[ 1 ][ 0 ] ) {
		d_border[ 1 ][ 0 ] = a.d_border[ 1 ][ 0 ];
		expanded = true;
	}
	return expanded;
}

INLINE Bounds2D Bounds2D::Intersect( const Bounds2D & a ) const {
	Bounds2D n;
	n.d_border[ 0 ][ 0 ] = ( a.d_border[ 0 ][ 0 ] > d_border[ 0 ][ 0 ] ) ? a.d_border[ 0 ][ 0 ] : d_border[ 0 ][ 0 ];
	n.d_border[ 0 ][ 1 ] = ( a.d_border[ 0 ][ 1 ] > d_border[ 0 ][ 1 ] ) ? a.d_border[ 0 ][ 1 ] : d_border[ 0 ][ 1 ];
	n.d_border[ 1 ][ 0 ] = ( a.d_border[ 1 ][ 0 ] < d_border[ 1 ][ 0 ] ) ? a.d_border[ 1 ][ 0 ] : d_border[ 1 ][ 0 ];
	n.d_border[ 1 ][ 1 ] = ( a.d_border[ 1 ][ 1 ] < d_border[ 1 ][ 1 ] ) ? a.d_border[ 1 ][ 1 ] : d_border[ 1 ][ 1 ];
	return n;
}

INLINE Bounds2D & Bounds2D::IntersectSelf( const Bounds2D & a ) {
	if ( a.d_border[ 0 ][ 0 ] > d_border[ 0 ][ 0 ] )
		d_border[ 0 ][ 0 ] = a.d_border[ 0 ][ 0 ];
	if ( a.d_border[ 0 ][ 1 ] > d_border[ 0 ][ 1 ] )
		d_border[ 0 ][ 1 ] = a.d_border[ 0 ][ 1 ];
	if ( a.d_border[ 1 ][ 0 ] < d_border[ 1 ][ 0 ] )
		d_border[ 1 ][ 0 ] = a.d_border[ 1 ][ 0 ];
	if ( a.d_border[ 1 ][ 1 ] < d_border[ 1 ][ 1 ] )
		d_border[ 1 ][ 1 ] = a.d_border[ 1 ][ 1 ];
	return *this;
}

INLINE Bounds2D Bounds2D::Expand( const float d ) const {
	return Bounds2D( Vec2( d_border[ 0 ][ 0 ] - d, d_border[ 0 ][ 1 ] - d ), Vec2( d_border[ 1 ][ 0 ] + d, d_border[ 1 ][ 1 ] + d ) );
}

INLINE Bounds2D & Bounds2D::ExpandSelf( const float d ) {
	d_border[ 0 ][ 0 ] -= d;
	d_border[ 0 ][ 1 ] -= d;
	d_border[ 1 ][ 0 ] += d;
	d_border[ 1 ][ 1 ] += d;
	return *this;
}

INLINE Bounds2D Bounds2D::Translate( const Vec2 & translation ) const {
	return Bounds2D( d_border[ 0 ] + translation, d_border[ 1 ] + translation );
}

INLINE Bounds2D & Bounds2D::TranslateSelf( const Vec2 & translation ) {
	d_border[ 0 ] += translation;
	d_border[ 1 ] += translation;
	return *this;
}

INLINE Bounds2D Bounds2D::Rotate( const Mat2 & rotation ) const {
	Bounds2D bounds;
	bounds.FromTransformedBounds( *this, vec2_origin, rotation );
	return bounds;
}

INLINE Bounds2D & Bounds2D::RotateSelf( const Mat2 & rotation ) {
	FromTransformedBounds( *this, vec2_origin, rotation );
	return *this;
}

INLINE Bounds2D Bounds2D::FlipEndPoint( ) const {
	return Bounds2D( d_border[ 0 ], Vec2( d_border[ 1 ][ 1 ], d_border[ 1 ][ 0 ] ) );
}

INLINE Bounds2D & Bounds2D::FlipEndPointSelf( ) {
	Swap( d_border[ 1 ][ 1 ], d_border[ 1 ][ 0 ] );
	return *this;
}

INLINE bool Bounds2D::ContainsPoint( const Vec2 & p ) const {
	if( p[ 0 ] < d_border[ 0 ][ 0 ] || p[ 1 ] < d_border[ 0 ][ 1 ] || p[ 0 ] > d_border[ 1 ][ 0 ] || p[ 1 ] > d_border[ 1 ][ 1 ] )
		return false;
	return true;
}

INLINE bool Bounds2D::IntersectsBounds( const Bounds2D & a ) const {
	if( a.d_border[ 1 ][ 0 ] < d_border[ 0 ][ 0 ] || a.d_border[ 1 ][ 1 ] < d_border[ 0 ][ 1 ] ||
		a.d_border[ 0 ][ 0 ] > d_border[ 1 ][ 0 ] || a.d_border[ 0 ][ 1 ] > d_border[ 1 ][ 1 ] )
		return false;
	return true;
}

INLINE Vec2 Bounds2D::GetUpperRight( ) const {
	return Vec2( d_border[ 0 ][ 0 ], d_border[ 1 ][ 1 ] );
}

INLINE Vec2 Bounds2D::GetLowerLeft( ) const {
	return Vec2( d_border[ 1 ][ 0 ], d_border[ 0 ][ 1 ] );
}

// // //// // //// // //// //
/// BoundsT
// //// // //// // //// //

template< class type, int dimension, class math_type = float > class BoundsT {
private:
	typedef VecT< type, dimension, math_type > vec_t;
	typedef MatT< type, dimension, math_type > mat_t;
	vec_t					d_border[ 2 ];

public:

							BoundsT( );
							explicit BoundsT( const vec_t & mins, const vec_t & maxs );

	/// returns true if 'a' box fits in caller box
	bool					operator >=( const BoundsT & a ) const;

	/// returns true if 'a' box smaller or equal than caller box
	bool					operator <=( const BoundsT & a ) const;

	const vec_t &			operator[ ]( const int index ) const;
	vec_t &					operator[ ]( const int index );

	/// most tight bounds for the given transformed bounds
	void					FromTransformedBounds( const BoundsT & bounds, const vec_t & origin, const mat_t & axis );

	/// inside out bounds
	void					Clear( );

	/// single point at origin
	void					Zero( );

	/// returns center of bounds
	vec_t					GetCenter( ) const;

	/// returns the radius relative to the bounds origin
	math_type				GetRadius( ) const;

	/// returns the radius relative to the given center
	math_type				GetRadius( const vec_t & center ) const;

	math_type				GetArea( ) const;

	/// add the point, returns true if the bounds expanded
	bool					AddPoint( const vec_t & v );

	/// add the bounds, returns true if the bounds expanded
	bool					AddBounds( const BoundsT & a );

	/// return intersection of this bounds with the given bounds
	BoundsT					Intersect( const BoundsT & a ) const;

	/// intersect this bounds with the given bounds
	BoundsT &				IntersectSelf( const BoundsT & a );

	/// return bounds expanded in all directions with the given value
	BoundsT					Expand( const math_type d ) const;

	/// expand bounds in all directions with the given value
	BoundsT &				ExpandSelf( const math_type d );

	/// return translated bounds
	BoundsT					Translate( const vec_t & translation ) const;

	/// translate this bounds
	BoundsT &				TranslateSelf( const vec_t & translation );

	/// return rotated bounds
	BoundsT					Rotate( const mat_t & rotation ) const;

	/// rotate this bounds
	BoundsT &				RotateSelf( const mat_t & rotation );

	BoundsT					FlipEndPoint( ) const;
	BoundsT &				FlipEndPointSelf( );

	/// includes touching
	bool					ContainsPoint( const vec_t & p ) const;

	/// includes touching
	bool					IntersectsBounds( const BoundsT & a ) const;

	bool					LineIntersection( const vec_t & start, const vec_t & end ) const;
	
	/// intersection point is start + dir * scale
	bool					RayIntersection( const vec_t & start, const vec_t & dir, math_type & scale ) const;
};

typedef BoundsT< int, 2 > BoundsT2i;

template< class type, int dimension, class math_type >
INLINE BoundsT< type, dimension, math_type >::BoundsT( ) {
}

template< class type, int dimension, class math_type >
INLINE BoundsT< type, dimension, math_type >::BoundsT( const vec_t & mins, const vec_t & maxs ) {
	d_border[ 0 ] = mins;
	d_border[ 1 ] = maxs;
}

template< class type, int dimension, class math_type >
INLINE void BoundsT< type, dimension, math_type >::FromTransformedBounds( const BoundsT & bounds, const vec_t & origin, const mat_t & axis ) {
	vec_t center = ( bounds[ 0 ] + bounds[ 1 ] ) * 0.5f;
	vec_t extents = bounds[ 1 ] - center;
	vec_t rotatedExtents;
	for( int i = 0; i < dimension; i++ )
		rotatedExtents[ i ] = fabs( extents[ 0 ] * axis[ 0 ][ i ] ) + fabs( extents[ 1 ] * axis[ 1 ][ i ] );
	center = origin + center * axis;
	d_border[ 0 ] = center - rotatedExtents;
	d_border[ 1 ] = center + rotatedExtents;
}

template< class type, int dimension, class math_type >
INLINE bool BoundsT< type, dimension, math_type >::operator >=( const BoundsT & a ) const {
	vec_t extentsThis = d_border[ 1 ] - d_border[ 0 ];
	vec_t extentsA = a.d_border[ 1 ] - a.d_border[ 0 ];
	bool isFit = true;
	for( int r = 0; r < dimension; r++ ) {
		for( int i = 0; i < dimension; i++ ) {
			if( extentsThis[ i ] < extentsA[ ( r + i ) % dimension ] ) {
				isFit = false;
				break;
			}
			if( i == ( dimension - 1 ) ) {
				if( isFit )
					break;
				isFit = true;
			}
		}
	}
	return isFit;
}

template< class type, int dimension, class math_type >
INLINE bool BoundsT< type, dimension, math_type >::operator <=( const BoundsT & a ) const {
	vec_t extentsThis = d_border[ 1 ] - d_border[ 0 ];
	vec_t extentsA = a.d_border[ 1 ] - a.d_border[ 0 ];
	bool isFit = true;
	for( int r = 0; r < dimension; r++ ) {
		for( int i = 0; i < dimension; i++ ) {
			if( extentsThis[ i ] > extentsA[ ( r + i ) % dimension ] ) {
				isFit = false;
				break;
			}
			if( i == ( dimension - 1 ) ) {
				if( isFit )
					break;
				isFit = true;
			}
		}
	}
	return isFit;
}

template< class type, int dimension, class math_type >
INLINE const VecT< type, dimension, math_type > & BoundsT< type, dimension, math_type >::operator [ ]( const int index ) const {
	return d_border[ index ];
}

template< class type, int dimension, class math_type >
INLINE VecT< type, dimension, math_type > & BoundsT< type, dimension, math_type >::operator[ ]( const int index ) {
	return d_border[ index ];
}

template< class type, int dimension, class math_type >
INLINE void BoundsT< type, dimension, math_type >::Clear( ) {
	for( int i = 0; i < dimension; i++ ) {
		d_border[ 0 ][ i ] = INFINITY;
		d_border[ 1 ][ i ] = -INFINITY;
	}
}

template< class type, int dimension, class math_type >
INLINE void BoundsT< type, dimension, math_type >::Zero( ) {
	d_border[ 0 ].Zero( );
	d_border[ 1 ].Zero( );
}

template< class type, int dimension, class math_type >
INLINE VecT< type, dimension, math_type > BoundsT< type, dimension, math_type >::GetCenter( ) const {
	return ( d_border[ 0 ] + d_border[ 1 ] ) * 0.5f;
}

template< class type, int dimension, class math_type >
INLINE math_type BoundsT< type, dimension, math_type >::GetRadius( ) const {
	math_type total = 0.0f;
	for( int i = 0; i < dimension; i++ ) {
		math_type b0 = fabs( d_border[ 0 ][ i ] );
		math_type b1 = fabs( d_border[ 1 ][ i ] );
		if ( b0 > b1 )
			total += b0 * b0;
		else
			total += b1 * b1;
	}
	return sqrtf( total );
}

template< class type, int dimension, class math_type >
INLINE math_type BoundsT< type, dimension, math_type >::GetRadius( const vec_t & center ) const {
	math_type total = 0;
	for( int i = 0; i < dimension; i++ ) {
		math_type b0 = fabs( center[ i ] - d_border[ 0 ][ i ] );
		math_type b1 = fabs( d_border[ 1 ][ i ] - center[ i ] );
		if ( b0 > b1 )
			total += b0 * b0;
		else
			total += b1 * b1;
	}
	return sqrtf( total );
}

template< class type, int dimension, class math_type >
INLINE math_type BoundsT< type, dimension, math_type >::GetArea( ) const {
	for( int i = 0; i < dimension; i++ )
		if( d_border[ 0 ][ i ] >= d_border[ 1 ][ i ] )
			return ( math_type )0;
	math_type retval = ( math_type)( d_border[ 1 ][ 0 ] - d_border[ 0 ][ 0 ] );
	for( int i = 1; i < dimension; i++ )
		retval *= d_border[ 1 ][ i ] - d_border[ 0 ][ i ];
	return retval;
}

template< class type, int dimension, class math_type >
INLINE bool BoundsT< type, dimension, math_type >::AddPoint( const vec_t & v ) {
	bool expanded = false;
	for( int i = 0; i < dimension; i++ ) {
		if( v[ i ] < d_border[ 0 ][ i ] ) {
			d_border[ 0 ][ i ] = v[ i ];
			expanded = true;
		}
		if( v[ i ] > d_border[ 1 ][ i ] ) {
			d_border[ 1 ][ i ] = v[ i ];
			expanded = true;
		}
	}
	return expanded;
}

template< class type, int dimension, class math_type >
INLINE bool BoundsT< type, dimension, math_type >::AddBounds( const BoundsT & a ) {
	bool expanded = false;
	for( int i = 0; i < dimension; i++ ) {
		if( a.d_border[ 0 ][ i ] < d_border[ 0 ][ i ] ) {
			d_border[ 0 ][ i ] = a.d_border[ 0 ][ i ];
			expanded = true;
		}
		if( a.d_border[ 1 ][ i ] > d_border[ 1 ][ i ] ) {
			d_border[ 1 ][ i ] = a.d_border[ 1 ][ i ];
			expanded = true;
		}
	}
	return expanded;
}

template< class type, int dimension, class math_type >
INLINE BoundsT< type, dimension, math_type > BoundsT< type, dimension, math_type >::Intersect( const BoundsT & a ) const {
	BoundsT n;
	for( int i = 0; i < dimension; i++ ) {
		n.d_border[ 0 ][ i ] = ( a.d_border[ 0 ][ i ] > d_border[ 0 ][ i ] ) ? a.d_border[ 0 ][ i ] : d_border[ 0 ][ i ];
		n.d_border[ 1 ][ i ] = ( a.d_border[ 1 ][ i ] < d_border[ 1 ][ i ] ) ? a.d_border[ 1 ][ i ] : d_border[ 1 ][ i ];
	}
	return n;
}

template< class type, int dimension, class math_type >
INLINE BoundsT< type, dimension, math_type > & BoundsT< type, dimension, math_type >::IntersectSelf( const BoundsT & a ) {
	for( int i = 0; i < dimension; i++ ) {
		if ( a.d_border[ 0 ][ i ] > d_border[ 0 ][ i ] )
			d_border[ 0 ][ i ] = a.d_border[ 0 ][ i ];
		if ( a.d_border[ 1 ][ i ] < d_border[ 1 ][ i ] )
			d_border[ 1 ][ i ] = a.d_border[ 1 ][ i ];
	}
	return *this;
}

template< class type, int dimension, class math_type >
INLINE BoundsT< type, dimension, math_type > BoundsT< type, dimension, math_type >::Expand( const math_type d ) const {
	return BoundsT( d_border[ 0 ] - d, d_border[ 1 ] + d );
}

template< class type, int dimension, class math_type >
INLINE BoundsT< type, dimension, math_type > & BoundsT< type, dimension, math_type >::ExpandSelf( const math_type d ) {
	d_border[ 0 ] -= d;
	d_border[ 1 ] += d;
	return *this;
}

template< class type, int dimension, class math_type >
INLINE BoundsT< type, dimension, math_type > BoundsT< type, dimension, math_type >::Translate( const vec_t & translation ) const {
	return BoundsT( d_border[ 0 ] + translation, d_border[ 1 ] + translation );
}

template< class type, int dimension, class math_type >
INLINE BoundsT< type, dimension, math_type > & BoundsT< type, dimension, math_type >::TranslateSelf( const vec_t & translation ) {
	d_border[ 0 ] += translation;
	d_border[ 1 ] += translation;
	return *this;
}

template< class type, int dimension, class math_type >
INLINE BoundsT< type, dimension, math_type > BoundsT< type, dimension, math_type >::Rotate( const mat_t & rotation ) const {
	BoundsT bounds;
	bounds.FromTransformedBounds( *this, vec2_origin, rotation );
	return bounds;
}

template< class type, int dimension, class math_type >
INLINE BoundsT< type, dimension, math_type > & BoundsT< type, dimension, math_type >::RotateSelf( const mat_t & rotation ) {
	FromTransformedBounds( *this, vec2_origin, rotation );
	return *this;
}

template< class type, int dimension, class math_type >
INLINE BoundsT< type, dimension, math_type > BoundsT< type, dimension, math_type >::FlipEndPoint( ) const {
	vec_t retval;
	for( int i = 0; i < dimension; i++ )
		retval[ i ] = d_border[ 1 ][ ( dimension - 1 ) - i ];
	return BoundsT( d_border[ 0 ], retval );
}

template< class type, int dimension, class math_type >
INLINE BoundsT< type, dimension, math_type > & BoundsT< type, dimension, math_type >::FlipEndPointSelf( ) {
	for( int i = 0; i < dimension; i++ )
		Swap( d_border[ 1 ][ i ], d_border[ 1 ][ ( dimension - 1 ) - i ] );
	return *this;
}

template< class type, int dimension, class math_type >
INLINE bool BoundsT< type, dimension, math_type >::ContainsPoint( const vec_t & p ) const {
	for( int i = 0; i < dimension; i++ )
		if( p[ i ] < d_border[ 0 ][ i ] || p[ i ] > d_border[ 1 ][ i ] )
			return false;
	return true;
}

template< class type, int dimension, class math_type >
INLINE bool BoundsT< type, dimension, math_type >::IntersectsBounds( const BoundsT & a ) const {
	for( int i = 0; i < dimension; i++ )
		if( a.d_border[ 1 ][ i ] < d_border[ 0 ][ i ] || a.d_border[ 0 ][ i ] > d_border[ 1 ][ i ] )
			return false;
	return true;
}

#endif
