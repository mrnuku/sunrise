// Copyright (C) 2004 Id Software, Inc.
//

#ifndef __BV_FRUSTUM_H__
#define __BV_FRUSTUM_H__

/*
===============================================================================

	Orthogonal Frustum

===============================================================================
*/

class Frustum {
public:
					Frustum( void );

	void			SetOrigin( const Vec3 &origin );
	void			SetAxis( const Mat3 &axis );
	void			SetSize( float dNear, float dFar, float dLeft, float dUp );
	void			SetPyramid( float dNear, float dFar );
	void			MoveNearDistance( float dNear );
	void			MoveFarDistance( float dFar );

	const Vec3 &	GetOrigin( void ) const;						// returns frustum origin
	const Mat3 &	GetAxis( void ) const;							// returns frustum orientation
	Vec3			GetCenter( void ) const;						// returns center of frustum
	Mat4			Projection( float fov_y, float aspect ) const;

	bool			IsValid( void ) const;							// returns true if the frustum is valid
	float			GetNearDistance( void ) const;					// returns distance to near plane
	float			GetFarDistance( void ) const;					// returns distance to far plane
	float			GetLeft( void ) const;							// returns left vector length
	float			GetUp( void ) const;							// returns up vector length

	Frustum			Expand( const float d ) const;					// returns frustum expanded in all directions with the given value
	Frustum &		ExpandSelf( const float d );					// expands frustum in all directions with the given value
	Frustum			Translate( const Vec3 &translation ) const;	// returns translated frustum
	Frustum &		TranslateSelf( const Vec3 &translation );		// translates frustum
	Frustum			Rotate( const Mat3 &rotation ) const;			// returns rotated frustum
	Frustum &		RotateSelf( const Mat3 &rotation );			// rotates frustum

	float			PlaneDistance( const Plane &plane ) const;
	int				PlaneSide( const Plane &plane, const float epsilon = ON_EPSILON ) const;

					// fast culling but might not cull everything outside the frustum
	bool			CullPoint( const Vec3 &point ) const;
	bool			CullBounds( const Bounds &bounds ) const;
	bool			CullBox( const Box &box ) const;
	bool			CullSphere( const Sphere &sphere ) const;
	bool			CullFrustum( const Frustum &frustum ) const;
	bool			CullWinding( const class Winding &winding ) const;

					// exact intersection tests
	bool			ContainsPoint( const Vec3 &point ) const;
	bool			IntersectsBounds( const Bounds &bounds ) const;
	bool			IntersectsBox( const Box &box ) const;
	bool			IntersectsSphere( const Sphere &sphere ) const;
	bool			IntersectsFrustum( const Frustum &frustum ) const;
	bool			IntersectsWinding( const Winding &winding ) const;
	bool			LineIntersection( const Vec3 &start, const Vec3 &end ) const;
	bool			RayIntersection( const Vec3 &start, const Vec3 &dir, float &scale1, float &scale2 ) const;

					// returns true if the projection origin is far enough away from the bounding volume to create a valid frustum
	bool			FromProjection( const Bounds &bounds, const Vec3 &projectionOrigin, const float dFar );
	bool			FromProjection( const Box &box, const Vec3 &projectionOrigin, const float dFar );
	bool			FromProjection( const Sphere &sphere, const Vec3 &projectionOrigin, const float dFar );

					// moves the far plane so it extends just beyond the bounding volume
	bool			ConstrainToBounds( const Bounds &bounds );
	bool			ConstrainToBox( const Box &box );
	bool			ConstrainToSphere( const Sphere &sphere );
	bool			ConstrainToFrustum( const Frustum &frustum );

	void			ToPlanes( Plane planes[6] ) const;			// planes point outwards
	void			ToPoints( Vec3 points[8] ) const;				// 8 corners of the frustum

					// calculates the projection of this frustum onto the given axis
	void			AxisProjection( const Vec3 &dir, float &min, float &max ) const;
	void			AxisProjection( const Mat3 &ax, Bounds &bounds ) const;

					// calculates the bounds for the projection in this frustum
	bool			ProjectionBounds( const Bounds &bounds, Bounds &projectionBounds ) const;
	bool			ProjectionBounds( const Box &box, Bounds &projectionBounds ) const;
	bool			ProjectionBounds( const Sphere &sphere, Bounds &projectionBounds ) const;
	bool			ProjectionBounds( const Frustum &frustum, Bounds &projectionBounds ) const;
	bool			ProjectionBounds( const Winding &winding, Bounds &projectionBounds ) const;

					// calculates the bounds for the projection in this frustum of the given frustum clipped to the given box
	bool			ClippedProjectionBounds( const Frustum &frustum, const Box &clipBox, Bounds &projectionBounds ) const;

private:
	Vec3			origin;		// frustum origin
	Mat3			axis;		// frustum orientation
	float			dNear;		// distance of near plane, dNear >= 0.0f
	float			dFar;		// distance of far plane, dFar > dNear
	float			dLeft;		// half the width at the far plane
	float			dUp;		// half the height at the far plane
	float			invFar;		// 1.0f / dFar

private:
	bool			CullLocalBox( const Vec3 &localOrigin, const Vec3 &extents, const Mat3 &localAxis ) const;
	bool			CullLocalFrustum( const Frustum &localFrustum, const Vec3 indexPoints[8], const Vec3 cornerVecs[4] ) const;
	bool			CullLocalWinding( const Vec3 *points, const int numPoints, int *pointCull ) const;
	bool			BoundsCullLocalFrustum( const Bounds &bounds, const Frustum &localFrustum, const Vec3 indexPoints[8], const Vec3 cornerVecs[4] ) const;
	bool			LocalLineIntersection( const Vec3 &start, const Vec3 &end ) const;
	bool			LocalRayIntersection( const Vec3 &start, const Vec3 &dir, float &scale1, float &scale2 ) const;
	bool			LocalFrustumIntersectsFrustum( const Vec3 points[8], const bool testFirstSide ) const;
	bool			LocalFrustumIntersectsBounds( const Vec3 points[8], const Bounds &bounds ) const;
	void			ToClippedPoints( const float fractions[4], Vec3 points[8] ) const;
	void			ToIndexPoints( Vec3 indexPoints[8] ) const;
	void			ToIndexPointsAndCornerVecs( Vec3 indexPoints[8], Vec3 cornerVecs[4] ) const;
	void			AxisProjection( const Vec3 indexPoints[8], const Vec3 cornerVecs[4], const Vec3 &dir, float &min, float &max ) const;
	void			AddLocalLineToProjectionBoundsSetCull( const Vec3 &start, const Vec3 &end, int &startCull, int &endCull, Bounds &bounds ) const;
	void			AddLocalLineToProjectionBoundsUseCull( const Vec3 &start, const Vec3 &end, int startCull, int endCull, Bounds &bounds ) const;
	bool			AddLocalCapsToProjectionBounds( const Vec3 endPoints[4], const int endPointCull[4], const Vec3 &point, int pointCull, int pointClip, Bounds &projectionBounds ) const;
	bool			BoundsRayIntersection( const Bounds &bounds, const Vec3 &start, const Vec3 &dir, float &scale1, float &scale2 ) const;
	void			ClipFrustumToBox( const Box &box, float clipFractions[4], int clipPlanes[4] ) const;
	bool			ClipLine( const Vec3 localPoints[8], const Vec3 points[8], int startIndex, int endIndex, Vec3 &start, Vec3 &end, int &startClip, int &endClip ) const;
};

INLINE Frustum::Frustum( void ) {
	dNear = dFar = 0.0f;
}

INLINE void Frustum::SetOrigin( const Vec3 &origin ) {
	this->origin = origin;
}

INLINE void Frustum::SetAxis( const Mat3 &axis ) {
	this->axis = axis;
}

INLINE void Frustum::SetSize( float dNear, float dFar, float dLeft, float dUp ) {
	assert( dNear >= 0.0f && dFar > dNear && dLeft > 0.0f && dUp > 0.0f );
	this->dNear = dNear;
	this->dFar = dFar;
	this->dLeft = dLeft;
	this->dUp = dUp;
	this->invFar = 1.0f / dFar;
}

INLINE void Frustum::SetPyramid( float dNear, float dFar ) {
	assert( dNear >= 0.0f && dFar > dNear );
	this->dNear = dNear;
	this->dFar = dFar;
	this->dLeft = dFar;
	this->dUp = dFar;
	this->invFar = 1.0f / dFar;
}

INLINE void Frustum::MoveNearDistance( float dNear ) {
	assert( dNear >= 0.0f );
	this->dNear = dNear;
}

INLINE void Frustum::MoveFarDistance( float dFar ) {
	assert( dFar > this->dNear );
	float scale = dFar / this->dFar;
	this->dFar = dFar;
	this->dLeft *= scale;
	this->dUp *= scale;
	this->invFar = 1.0f / dFar;
}

INLINE const Vec3 &Frustum::GetOrigin( void ) const {
	return origin;
}

INLINE const Mat3 &Frustum::GetAxis( void ) const {
	return axis;
}

INLINE Vec3 Frustum::GetCenter( void ) const {
	return ( origin + axis[0] * ( ( dFar - dNear ) * 0.5f ) );
}

INLINE Mat4 Frustum::Projection( float fov_y, float aspect ) const {

	float top = tan( DEG2RAD( fov_y / 2.0f ) ) * dNear;	
	float left = -top * aspect;
	float right = top * aspect;
	float bottom = -top;

	return Mat4(	( 2.0f * dNear ) / ( right - left ), 0.0f, ( right + left ) / ( right - left ) /* frsutum */, 0.0f,
					0.0f, ( 2.0f * dNear ) / ( top - bottom ), ( top + bottom ) / ( top - bottom ) /* frsutum */, 0.0f,
					0.0f, 0.0f, -( dFar + dNear ) / ( dFar - dNear ), -( 2.0f * dFar * dNear ) / ( dFar - dNear ),
					0.0f, 0.0f, -1.0f, 1.0f );
}

INLINE bool Frustum::IsValid( void ) const {
	return ( dFar > dNear );
}

INLINE float Frustum::GetNearDistance( void ) const {
	return dNear;
}

INLINE float Frustum::GetFarDistance( void ) const {
	return dFar;
}

INLINE float Frustum::GetLeft( void ) const {
	return dLeft;
}

INLINE float Frustum::GetUp( void ) const {
	return dUp;
}

INLINE Frustum Frustum::Expand( const float d ) const {
	Frustum f = *this;
	f.origin -= d * f.axis[0];
	f.dFar += 2.0f * d;
	f.dLeft = f.dFar * dLeft * invFar;
	f.dUp = f.dFar * dUp * invFar;
	f.invFar = 1.0f / dFar;
	return f;
}

INLINE Frustum &Frustum::ExpandSelf( const float d ) {
	origin -= d * axis[0];
	dFar += 2.0f * d;
	dLeft = dFar * dLeft * invFar;
	dUp = dFar * dUp * invFar;
	invFar = 1.0f / dFar;
	return *this;
}

INLINE Frustum Frustum::Translate( const Vec3 &translation ) const {
	Frustum f = *this;
	f.origin += translation;
	return f;
}

INLINE Frustum &Frustum::TranslateSelf( const Vec3 &translation ) {
	origin += translation;
	return *this;
}

INLINE Frustum Frustum::Rotate( const Mat3 &rotation ) const {
	Frustum f = *this;
	f.axis *= rotation;
	return f;
}

INLINE Frustum &Frustum::RotateSelf( const Mat3 &rotation ) {
	axis *= rotation;
	return *this;
}

#endif /* !__BV_FRUSTUM_H__ */
