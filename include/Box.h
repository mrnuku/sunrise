#ifndef BOX_H
#define BOX_H

// // //// // //// // //// //
/// BoxBase
// //// // //// // //// //

class BoxBase {
protected:

	Vec3			d_center;
	Vec3			d_extents;

	void			ClearBase( );
	void			ZeroBase( );

public:

	BoxBase &		operator+=( const Vec3 & t );					// translate the box
	BoxBase &		operator-=( const BoxBase & a );

	BoxBase &		ExpandSelf( const float d );					// expand box in all directions with the given value
	BoxBase &		TranslateSelf( const Vec3 & translation );		// translate this box

	const Vec3 &	GetCenter( ) const;								// returns d_center of the box
	const Vec3 &	GetExtents( ) const;							// returns d_extents of the box
	float			GetVolume( ) const;								// returns the volume of the box
	bool			IsCleared( ) const;								// returns true if box are inside out

	float			PlaneDistance( const Plane & plane ) const;
	int				PlaneSide( const Plane & plane, const float epsilon = ON_EPSILON ) const;

	Sphere			ToSphere( ) const;
	Bounds			ToBounds( ) const;
};

// // //// // //// // //// //
/// Box
// //// // //// // //// //

class Box : public BoxBase {
public:
					Box( );
					explicit Box( const Vec3 &center, const Vec3 &extents, const Mat3 &axis );
					explicit Box( const Vec3 &point );
					explicit Box( const Bounds &bounds );
					explicit Box( const Bounds &bounds, const Vec3 &origin, const Mat3 &axis );

	Box				operator+( const Vec3 &t ) const;				// returns translated box	
	Box				operator*( const Mat3 &r ) const;				// returns rotated box
	Box &			operator*=( const Mat3 &r );					// rotate the box
	Box &			operator+=( const Box &a );
	Box				operator+( const Box &a ) const;
	Box				operator-( const Box &a ) const;

	bool			Compare( const Box &a ) const;						// exact compare, no epsilon
	bool			Compare( const Box &a, const float epsilon ) const;	// compare with epsilon
	bool			operator==(	const Box &a ) const;						// exact compare, no epsilon
	bool			operator!=(	const Box &a ) const;						// exact compare, no epsilon

	void			Clear( );									// inside out box
	void			Zero( );									// single point at origin

	const Mat3 &	GetAxis( ) const;							// returns the axis of the box

	bool			AddPoint( const Vec3 &v );					// add the point, returns true if the box expanded
	bool			AddBox( const Box &a );						// add the box, returns true if the box expanded
	Box				Expand( const float d ) const;					// return box expanded in all directions with the given value	
	Box				Translate( const Vec3 &translation ) const;	// return translated box	
	Box				Rotate( const Mat3 &rotation ) const;			// return rotated box
	Box &			RotateSelf( const Mat3 &rotation );			// rotate this box
	bool			ContainsPoint( const Vec3 &p ) const;			// includes touching
	bool			IntersectsBox( const Box &a ) const;			// includes touching
	bool			LineIntersection( const Vec3 &start, const Vec3 &end ) const;
					// intersection points are (start + dir * scale1) and (start + dir * scale2)
	bool			RayIntersection( const Vec3 &start, const Vec3 &dir, float &scale1, float &scale2 ) const;

					// tight box for a collection of points
	void			FromPoints( const Vec3 *points, const int numPoints );
					// most tight box for a translation
	void			FromPointTranslation( const Vec3 &point, const Vec3 &translation );
	void			FromBoxTranslation( const Box &box, const Vec3 &translation );
					// most tight box for a rotation
	void			FromPointRotation( const Vec3 &point, const Rotation &rotation );
	void			FromBoxRotation( const Box &box, const Rotation &rotation );

	void			ToPoints( Vec3 points[8] ) const;
	void			ToPlanes( Plane planes[ 6 ] ) const;

					// calculates the projection of this box onto the given axis
	void			AxisProjection( const Vec3 &dir, float &min, float &max ) const;
	void			AxisProjection( const Mat3 &ax, Bounds &bounds ) const;

					// calculates the silhouette of the box
	int				GetProjectionSilhouetteVerts( const Vec3 &projectionOrigin, Vec3 silVerts[6] ) const;
	int				GetParallelProjectionSilhouetteVerts( const Vec3 &projectionDir, Vec3 silVerts[6] ) const;

private:
	Mat3			axis;
};

extern Box	box_zero;

INLINE Box::Box( ) {
}

INLINE Box::Box( const Vec3 &center, const Vec3 &extents, const Mat3 &axis ) {
	d_center = center;
	d_extents = extents;
	this->axis = axis;
}

INLINE Box::Box( const Vec3 &point ) {
	d_center = point;
	d_extents.Zero();
	this->axis.Identity();
}

INLINE Box::Box( const Bounds &bounds ) {
	d_center = ( bounds[0] + bounds[1] ) * 0.5f;
	d_extents = bounds[1] - d_center;
	this->axis.Identity();
}

INLINE Box::Box( const Bounds &bounds, const Vec3 &origin, const Mat3 &axis ) {
	d_center = ( bounds[0] + bounds[1] ) * 0.5f;
	d_extents = bounds[1] - d_center;
	d_center = origin + d_center * axis;
	this->axis = axis;
}

INLINE Box Box::operator+( const Vec3 &t ) const {
	return Box( d_center + t, d_extents, axis );
}

INLINE BoxBase & BoxBase::operator+=( const Vec3 &t ) {
	d_center += t;
	return *this;
}

INLINE Box Box::operator*( const Mat3 &r ) const {
	return Box( d_center * r, d_extents, axis * r );
}

INLINE Box &Box::operator*=( const Mat3 &r ) {
	d_center *= r;
	axis *= r;
	return *this;
}

INLINE Box Box::operator+( const Box &a ) const {
	Box newBox;
	newBox = *this;
	newBox.AddBox( a );
	return newBox;
}

INLINE Box &Box::operator+=( const Box &a ) {
	Box::AddBox( a );
	return *this;
}

INLINE Box Box::operator-( const Box &a ) const {
	return Box( d_center, d_extents - a.d_extents, axis );
}

INLINE BoxBase &BoxBase::operator-=( const BoxBase &a ) {
	d_extents -= a.d_extents;
	return *this;
}

INLINE bool Box::Compare( const Box &a ) const {
	return ( d_center.Compare( a.d_center ) && d_extents.Compare( a.d_extents ) && axis.Compare( a.axis ) );
}

INLINE bool Box::Compare( const Box &a, const float epsilon ) const {
	return ( d_center.Compare( a.d_center, epsilon ) && d_extents.Compare( a.d_extents, epsilon ) && axis.Compare( a.axis, epsilon ) );
}

INLINE bool Box::operator==( const Box &a ) const {
	return Compare( a );
}

INLINE bool Box::operator!=( const Box &a ) const {
	return !Compare( a );
}

INLINE void BoxBase::ClearBase( ) {
	d_center.Zero( );
	d_extents[ 0 ] = d_extents[ 1 ] = d_extents[ 2 ] = -INFINITY;
}

INLINE void BoxBase::ZeroBase( ) {
	d_center.Zero( );
	d_extents.Zero( );
}

INLINE void Box::Clear( ) {
	ClearBase( );
	axis.Identity();
}

INLINE void Box::Zero( ) {
	ZeroBase( );
	axis.Identity();
}

INLINE const Vec3 &BoxBase::GetCenter( ) const {
	return d_center;
}

INLINE const Vec3 &BoxBase::GetExtents( ) const {
	return d_extents;
}
INLINE float BoxBase::GetVolume( ) const {
	return ( d_extents * 2.0f ).LengthSqr();
}

INLINE bool BoxBase::IsCleared( ) const {
	return d_extents[0] < 0.0f;
}

INLINE BoxBase &BoxBase::ExpandSelf( const float d ) {
	d_extents[ 0 ] += d;
	d_extents[ 1 ] += d;
	d_extents[ 2 ] += d;
	return *this;
}

INLINE const Mat3 &Box::GetAxis( ) const {
	return axis;
}

INLINE BoxBase &BoxBase::TranslateSelf( const Vec3 &translation ) {
	d_center += translation;
	return *this;
}

INLINE Box Box::Expand( const float d ) const {
	return Box( d_center, d_extents + Vec3( d, d, d ), axis );
}

INLINE Box Box::Translate( const Vec3 &translation ) const {
	return Box( d_center + translation, d_extents, axis );
}


INLINE Box Box::Rotate( const Mat3 &rotation ) const {
	return Box( d_center * rotation, d_extents, axis * rotation );
}

INLINE Box &Box::RotateSelf( const Mat3 &rotation ) {
	d_center *= rotation;
	axis *= rotation;
	return *this;
}

INLINE bool Box::ContainsPoint( const Vec3 &p ) const {
	Vec3 lp = p - d_center;
	if ( fabs( lp * axis[0] ) > d_extents[0] ||
			fabs( lp * axis[1] ) > d_extents[1] ||
				fabs( lp * axis[2] ) > d_extents[2] ) {
		return false;
	}
	return true;
}

INLINE Sphere BoxBase::ToSphere( ) const {
	return Sphere( d_center, d_extents.Length() );
}

INLINE Bounds BoxBase::ToBounds( ) const {
	Vec3 bVec = d_extents + d_center;
	return Bounds( ( d_center * 2.0f ) - bVec, bVec );
}

INLINE void Box::AxisProjection( const Vec3 &dir, float &min, float &max ) const {
	float d1 = dir * d_center;
	float d2 = fabs( d_extents[0] * ( dir * axis[0] ) ) +
				fabs( d_extents[1] * ( dir * axis[1] ) ) +
				fabs( d_extents[2] * ( dir * axis[2] ) );
	min = d1 - d2;
	max = d1 + d2;
}

INLINE void Box::AxisProjection( const Mat3 &ax, Bounds &bounds ) const {
	for ( int i = 0; i < 3; i++ ) {
		float d1 = ax[i] * d_center;
		float d2 = fabs( d_extents[0] * ( ax[i] * axis[0] ) ) +
					fabs( d_extents[1] * ( ax[i] * axis[1] ) ) +
					fabs( d_extents[2] * ( ax[i] * axis[2] ) );
		bounds[0][i] = d1 - d2;
		bounds[1][i] = d1 + d2;
	}
}

INLINE void Box::ToPoints( Vec3 points[8] ) const {

	Mat3 ax;
	Vec3 temp[4];

	ax[0] = d_extents[0] * axis[0];
	ax[1] = d_extents[1] * axis[1];
	ax[2] = d_extents[2] * axis[2];
	temp[0] = d_center - ax[0];
	temp[1] = d_center + ax[0];
	temp[2] = ax[1] - ax[2];
	temp[3] = ax[1] + ax[2];
	points[0] = temp[0] - temp[3];
	points[1] = temp[1] - temp[3];
	points[2] = temp[1] + temp[2];
	points[3] = temp[0] + temp[2];
	points[4] = temp[0] - temp[2];
	points[5] = temp[1] - temp[2];
	points[6] = temp[1] + temp[3];
	points[7] = temp[0] + temp[3];
}

INLINE void Box::ToPlanes( Plane planes[ 6 ] ) const {

	Vec3 boxPoints[ 8 ];
	ToPoints( boxPoints );

	planes[ 0 ].FromPoints( boxPoints[ 4 ], boxPoints[ 7 ], boxPoints[ 6 ] );
	planes[ 1 ].FromPoints( boxPoints[ 0 ], boxPoints[ 1 ], boxPoints[ 2 ] );

	planes[ 2 ].FromPoints( boxPoints[ 7 ], boxPoints[ 3 ], boxPoints[ 2 ] );
	planes[ 3 ].FromPoints( boxPoints[ 5 ], boxPoints[ 1 ], boxPoints[ 0 ] );

	planes[ 4 ].FromPoints( boxPoints[ 6 ], boxPoints[ 2 ], boxPoints[ 1 ] );
	planes[ 5 ].FromPoints( boxPoints[ 4 ], boxPoints[ 0 ], boxPoints[ 3 ] );
}

// // //// // //// // //// //
/// Rectangular
// //// // //// // //// //

class Rectangular {
private:
	Vec2											d_center;
	Vec2											d_extents;
	Mat2											d_axis;

public:
													Rectangular( );
													explicit Rectangular( const Vec2 & d_center, const Vec2 & d_extents, const Mat2 & axis );
													explicit Rectangular( const Vec2 & point );
													explicit Rectangular( const Bounds2D & bounds );
													explicit Rectangular( const Bounds2D & bounds, const Vec2 & origin, const Mat2 & axis );

	/// returns rotated box
	Rectangular										operator *( const Mat2 & r ) const;

	Rectangular &									operator *=( const Mat2 & r );
	/// needed by BTree
	Rectangular &									operator =( const int n );
	/// returns true if 'a' box fits in caller box
	bool											operator >=( const Rectangular & a ) const;
	/// returns true if 'a' box smaller or equal than caller box
	bool											operator <=( const Rectangular & a ) const;

	bool											operator >( const Rectangular & a ) const;

	bool											operator <( const Rectangular & a ) const;

	bool											operator ==( const Rectangular & a ) const;

	bool											CanFit( const Rectangular & a ) const;
	/// returns d_center of the box
	const Vec2 &									GetCenter( ) const;
	/// returns d_extents of the box
	const Vec2 &									GetExtents( ) const;
	/// returns the axis of the box
	const Mat2 &									GetAxis( ) const;

	float											GetArea( ) const;
	/// return box expanded in all directions with the given value
	Rectangular										Expand( const float d ) const;
	/// expand box in all directions with the given value
	Rectangular &									ExpandSelf( const float d );
	/// return translated box
	Rectangular										Translate( const Vec2 & translation ) const;
	/// translate this box
	Rectangular &									TranslateSelf( const Vec2 & translation );
	/// return rotated box
	Rectangular										Rotate( const Mat2 & rotation ) const;
	/// rotate this box
	Rectangular &									RotateSelf( const Mat2 & rotation );
};

// // //// // //// // //// //
// Rectangular
// //// // //// // //// //

INLINE Rectangular::Rectangular( ) {
}

INLINE Rectangular::Rectangular( const Vec2 & center, const Vec2 & extents, const Mat2 & axis ) {
	d_center = center;
	d_extents = extents;
	d_axis = axis;
}

INLINE Rectangular::Rectangular( const Vec2 & point ) {
	d_center = point;
	d_extents.Zero( );
	d_axis.Identity( );
}

INLINE Rectangular::Rectangular( const Bounds2D & bounds ) {
	d_center = ( bounds[ 0 ] + bounds[ 1 ] ) * 0.5f;
	d_extents = bounds[ 1 ] - this->d_center;
	d_axis.Identity( );
}

INLINE Rectangular::Rectangular( const Bounds2D & bounds, const Vec2 & origin, const Mat2 & axis ) {
	d_center = ( bounds[ 0 ] + bounds[ 1 ] ) * 0.5f;
	d_extents = bounds[ 1 ] - d_center;
	d_center = origin + d_center * axis;
	d_axis = axis;
}

INLINE const Vec2 & Rectangular::GetCenter( ) const {
	return d_center;
}

INLINE const Vec2 & Rectangular::GetExtents( ) const {
	return d_extents;
}

INLINE const Mat2 & Rectangular::GetAxis( ) const {
	return d_axis;
}

INLINE float Rectangular::GetArea( ) const {
	return d_extents[ 0 ] * d_extents[ 1 ];
}

INLINE Rectangular Rectangular::operator *( const Mat2 & r ) const {
	return Rectangular( d_center * r, d_extents, d_axis * r );
}

INLINE Rectangular & Rectangular::operator *=( const Mat2 & r ) {
	d_center *= r;
	d_axis *= r;
	return *this;
}

INLINE Rectangular & Rectangular::operator =( const int n ) {
	assert( n == 0 ); // only for zeroing
	d_center.Zero( );
	d_extents.Zero( );
	d_axis.Identity( );
	return *this;
}

INLINE bool Rectangular::CanFit( const Rectangular & a ) const {
	if( d_extents[ 0 ] >= a.d_extents[ 0 ] && d_extents[ 1 ] >= a.d_extents[ 1 ] ) // un-rotated test
		return true;
	if( d_extents[ 1 ] >= a.d_extents[ 0 ] && d_extents[ 0 ] >= a.d_extents[ 1 ] ) // second, rotated test
		return true;
	return false; // cant fit
}

INLINE bool Rectangular::operator >=( const Rectangular & a ) const {
	if( GetArea( ) >= a.GetArea( ) )
		return true;
	return false;
}

INLINE bool Rectangular::operator <=( const Rectangular & a ) const {
	if( GetArea( ) <= a.GetArea( ) )
		return true;
	return false;
}

INLINE bool Rectangular::operator >( const Rectangular & a ) const {
	if( GetArea( ) > a.GetArea( ) )
		return true;
	return false;
}

INLINE bool Rectangular::operator <( const Rectangular & a ) const {
	if( this->GetArea( ) < a.GetArea( ) )
		return true;
	return false;
}

INLINE bool Rectangular::operator ==( const Rectangular & a ) const {
	if( this->GetArea( ) == a.GetArea( ) )
		return true;
	return false;
}

INLINE Rectangular Rectangular::Expand( const float d ) const {
	return Rectangular( d_center, d_extents + Vec2( d, d ), d_axis );
}

INLINE Rectangular & Rectangular::ExpandSelf( const float d ) {
	d_extents[ 0 ] += d;
	d_extents[ 1 ] += d;
	return *this;
}

INLINE Rectangular Rectangular::Translate( const Vec2 & translation ) const {
	return Rectangular( d_center + translation, d_extents, d_axis );
}

INLINE Rectangular & Rectangular::TranslateSelf( const Vec2 & translation ) {
	d_center += translation;
	return *this;
}

INLINE Rectangular Rectangular::Rotate( const Mat2 & rotation ) const {
	return Rectangular( d_center * rotation, d_extents, d_axis * rotation );
}

INLINE Rectangular & Rectangular::RotateSelf( const Mat2 & rotation ) {
	d_center *= rotation;
	d_axis *= rotation;
	return *this;
}

// // //// // //// // //// //
/// RectT
// //// // //// // //// //

template< class type, class math_type = float > class RectT {
private:
	typedef VecT< type, 2, math_type >				vec_t;
	typedef MatT< type, 2, math_type >				mat_t;
	typedef BoundsT< type, 2, math_type >			bounds_t;

	vec_t											d_center;
	vec_t											d_extents;
	mat_t											d_axis;

public:
													RectT( );
													explicit RectT( const vec_t & d_center, const vec_t & d_extents, const mat_t & axis );
													explicit RectT( const vec_t & d_center, const vec_t & d_extents );
													explicit RectT( const vec_t & point );
													explicit RectT( const bounds_t & bounds );
													explicit RectT( const bounds_t & bounds, const vec_t & origin, const mat_t & axis );

	/// returns rotated box
	RectT											operator *( const mat_t & r ) const;

	RectT &											operator *=( const mat_t & r );
	/// needed by BTree
	RectT &											operator =( const int n );
	/// returns true if 'a' box fits in caller box
	bool											operator >=( const RectT & a ) const;
	/// returns true if 'a' box smaller or equal than caller box
	bool											operator <=( const RectT & a ) const;

	bool											operator >( const RectT & a ) const;

	bool											operator <( const RectT & a ) const;

	bool											operator ==( const RectT & a ) const;

	bool											CanFit( const RectT & a ) const;
	/// returns d_center of the box
	const vec_t &									GetCenter( ) const;
	/// returns d_extents of the box
	const vec_t &									GetExtents( ) const;
	/// returns the axis of the box
	const mat_t &									GetAxis( ) const;

	math_type										GetArea( ) const;
	/// return box expanded in all directions with the given value
	RectT											Expand( const math_type d ) const;
	/// expand box in all directions with the given value
	RectT &											ExpandSelf( const math_type d );
	/// return translated box
	RectT											Translate( const vec_t & translation ) const;
	/// translate this box
	RectT &											TranslateSelf( const vec_t & translation );
	/// return rotated box
	RectT											Rotate( const mat_t & rotation ) const;
	/// rotate this box
	RectT &											RotateSelf( const mat_t & rotation );
};

typedef RectT< int >	RectTi;

// // //// // //// // //// //
// RectT
// //// // //// // //// //

template< class type, class math_type >
INLINE RectT< type, math_type >::RectT( ) {
}

template< class type, class math_type >
INLINE RectT< type, math_type >::RectT( const vec_t & center, const vec_t & extents, const mat_t & axis ) {
	d_center = center;
	d_extents = extents;
	d_axis = axis;
}

template< class type, class math_type >
INLINE RectT< type, math_type >::RectT( const vec_t & center, const vec_t & extents ) {
	d_center = center;
	d_extents = extents;
	d_axis.Identity( );
}

template< class type, class math_type >
INLINE RectT< type, math_type >::RectT( const vec_t & point ) {
	d_center = point;
	d_extents.Zero( );
	d_axis.Identity( );
}

template< class type, class math_type >
INLINE RectT< type, math_type >::RectT( const bounds_t & bounds ) {
	d_center = ( bounds[ 0 ] + bounds[ 1 ] ) * 0.5f;
	d_extents = bounds[ 1 ] - this->d_center;
	d_axis.Identity( );
}

template< class type, class math_type >
INLINE RectT< type, math_type >::RectT( const bounds_t & bounds, const vec_t & origin, const mat_t & axis ) {
	d_center = ( bounds[ 0 ] + bounds[ 1 ] ) * 0.5f;
	d_extents = bounds[ 1 ] - d_center;
	d_center = origin + d_center * axis;
	d_axis = axis;
}

template< class type, class math_type >
INLINE const VecT< type, 2, math_type > & RectT< type, math_type >::GetCenter( ) const {
	return d_center;
}

template< class type, class math_type >
INLINE const VecT< type, 2, math_type > & RectT< type, math_type >::GetExtents( ) const {
	return d_extents;
}

template< class type, class math_type >
INLINE const MatT< type, 2, math_type > & RectT< type, math_type >::GetAxis( ) const {
	return d_axis;
}

template< class type, class math_type >
INLINE math_type RectT< type, math_type >::GetArea( ) const {
	return ( math_type )( d_extents[ 0 ] * d_extents[ 1 ] );
}

template< class type, class math_type >
INLINE RectT< type, math_type > RectT< type, math_type >::operator *( const mat_t & r ) const {
	return RectT( d_center * r, d_extents, d_axis * r );
}

template< class type, class math_type >
INLINE RectT< type, math_type > & RectT< type, math_type >::operator *=( const mat_t & r ) {
	d_center *= r;
	d_axis *= r;
	return *this;
}

template< class type, class math_type >
INLINE RectT< type, math_type > & RectT< type, math_type >::operator =( const int n ) {
	assert( n == 0 ); // only for zeroing
	d_center.Zero( );
	d_extents.Zero( );
	d_axis.Identity( );
	return *this;
}

template< class type, class math_type >
INLINE bool RectT< type, math_type >::CanFit( const RectT & a ) const {
	if( d_extents[ 0 ] >= a.d_extents[ 0 ] && d_extents[ 1 ] >= a.d_extents[ 1 ] ) // un-rotated test
		return true;
	if( d_extents[ 1 ] >= a.d_extents[ 0 ] && d_extents[ 0 ] >= a.d_extents[ 1 ] ) // second, rotated test
		return true;
	return false; // cant fit
}

template< class type, class math_type >
INLINE bool RectT< type, math_type >::operator >=( const RectT & a ) const {
	if( GetArea( ) >= a.GetArea( ) )
		return true;
	return false;
}

template< class type, class math_type >
INLINE bool RectT< type, math_type >::operator <=( const RectT & a ) const {
	if( GetArea( ) <= a.GetArea( ) )
		return true;
	return false;
}

template< class type, class math_type >
INLINE bool RectT< type, math_type >::operator >( const RectT & a ) const {
	if( GetArea( ) > a.GetArea( ) )
		return true;
	return false;
}

template< class type, class math_type >
INLINE bool RectT< type, math_type >::operator <( const RectT & a ) const {
	if( this->GetArea( ) < a.GetArea( ) )
		return true;
	return false;
}

template< class type, class math_type >
INLINE bool RectT< type, math_type >::operator ==( const RectT & a ) const {
	if( this->GetArea( ) == a.GetArea( ) )
		return true;
	return false;
}

template< class type, class math_type >
INLINE RectT< type, math_type > RectT< type, math_type >::Expand( const math_type d ) const {
	return RectT( d_center, d_extents + Vec2( d, d ), d_axis );
}

template< class type, class math_type >
INLINE RectT< type, math_type > & RectT< type, math_type >::ExpandSelf( const math_type d ) {
	d_extents[ 0 ] += d;
	d_extents[ 1 ] += d;
	return *this;
}

template< class type, class math_type >
INLINE RectT< type, math_type > RectT< type, math_type >::Translate( const vec_t & translation ) const {
	return RectT( d_center + translation, d_extents, d_axis );
}

template< class type, class math_type >
INLINE RectT< type, math_type > & RectT< type, math_type >::TranslateSelf( const vec_t & translation ) {
	d_center += translation;
	return *this;
}

template< class type, class math_type >
INLINE RectT< type, math_type > RectT< type, math_type >::Rotate( const mat_t & rotation ) const {
	return RectT( d_center * rotation, d_extents, d_axis * rotation );
}

template< class type, class math_type >
INLINE RectT< type, math_type > & RectT< type, math_type >::RotateSelf( const mat_t & rotation ) {
	d_center *= rotation;
	d_axis *= rotation;
	return *this;
}

#endif
