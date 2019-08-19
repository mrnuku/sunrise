// Copyright (C) 2004 Id Software, Inc.
//

#ifndef __BV_SPHERE_H__
#define __BV_SPHERE_H__

/*
===============================================================================

	Sphere

===============================================================================
*/

class Sphere {
public:
					Sphere( void );
					explicit Sphere( const Vec3 &point );
					explicit Sphere( const Vec3 &point, const float r );

	float			operator[]( const int index ) const;
	float &			operator[]( const int index );
	Sphere			operator+( const Vec3 &t ) const;				// returns tranlated sphere
	Sphere &		operator+=( const Vec3 &t );					// translate the sphere
	Sphere			operator+( const Sphere &s ) const;
	Sphere &		operator+=( const Sphere &s );

	bool			Compare( const Sphere &a ) const;							// exact compare, no epsilon
	bool			Compare( const Sphere &a, const float epsilon ) const;	// compare with epsilon
	bool			operator==(	const Sphere &a ) const;						// exact compare, no epsilon
	bool			operator!=(	const Sphere &a ) const;						// exact compare, no epsilon

	void			Clear( void );									// inside out sphere
	void			Zero( void );									// single point at origin
	void			SetOrigin( const Vec3 &o );					// set origin of sphere
	void			SetRadius( const float r );						// set square radius

	const Vec3 &	GetOrigin( void ) const;						// returns origin of sphere
	float			GetRadius( void ) const;						// returns sphere radius
	bool			IsCleared( void ) const;						// returns true if sphere is inside out

	bool			AddPoint( const Vec3 &p );					// add the point, returns true if the sphere expanded
	bool			AddSphere( const Sphere &s );					// add the sphere, returns true if the sphere expanded
	Sphere			Expand( const float d ) const;					// return bounds expanded in all directions with the given value
	Sphere &		ExpandSelf( const float d );					// expand bounds in all directions with the given value
	Sphere			Translate( const Vec3 &translation ) const;
	Sphere &		TranslateSelf( const Vec3 &translation );

	float			PlaneDistance( const Plane &plane ) const;
	int				PlaneSide( const Plane &plane, const float epsilon = ON_EPSILON ) const;

	bool			ContainsPoint( const Vec3 &p ) const;			// includes touching
	bool			IntersectsSphere( const Sphere &s ) const;	// includes touching
	bool			LineIntersection( const Vec3 &start, const Vec3 &end ) const;
					// intersection points are (start + dir * scale1) and (start + dir * scale2)
	bool			RayIntersection( const Vec3 &start, const Vec3 &dir, float &scale1, float &scale2 ) const;

					// Tight sphere for a point set.
	void			FromPoints( const Vec3 *points, const int numPoints );
					// Most tight sphere for a translation.
	void			FromPointTranslation( const Vec3 &point, const Vec3 &translation );
	void			FromSphereTranslation( const Sphere &sphere, const Vec3 &start, const Vec3 &translation );
					// Most tight sphere for a rotation.
	void			FromPointRotation( const Vec3 &point, const Rotation &rotation );
	void			FromSphereRotation( const Sphere &sphere, const Vec3 &start, const Rotation &rotation );

	void			AxisProjection( const Vec3 &dir, float &min, float &max ) const;

private:
	Vec3			origin;
	float			radius;
};

extern Sphere		sphere_zero;

INLINE Sphere::Sphere( void ) {
}

INLINE Sphere::Sphere( const Vec3 &point ) {
	origin = point;
	radius = 0.0f;
}

INLINE Sphere::Sphere( const Vec3 &point, const float r ) {
	origin = point;
	radius = r;
}

INLINE float Sphere::operator[]( const int index ) const {
	return ((float *) &origin)[index];
}

INLINE float &Sphere::operator[]( const int index ) {
	return ((float *) &origin)[index];
}

INLINE Sphere Sphere::operator+( const Vec3 &t ) const {
	return Sphere( origin + t, radius );
}

INLINE Sphere &Sphere::operator+=( const Vec3 &t ) {
	origin += t;
	return *this;
}

INLINE bool Sphere::Compare( const Sphere &a ) const {
	return ( origin.Compare( a.origin ) && radius == a.radius );
}

INLINE bool Sphere::Compare( const Sphere &a, const float epsilon ) const {
	return ( origin.Compare( a.origin, epsilon ) && fabs( radius - a.radius ) <= epsilon );
}

INLINE bool Sphere::operator==( const Sphere &a ) const {
	return Compare( a );
}

INLINE bool Sphere::operator!=( const Sphere &a ) const {
	return !Compare( a );
}

INLINE void Sphere::Clear( void ) {
	origin.Zero();
	radius = -1.0f;
}

INLINE void Sphere::Zero( void ) {
	origin.Zero();
	radius = 0.0f;
}

INLINE void Sphere::SetOrigin( const Vec3 &o ) {
	origin = o;
}

INLINE void Sphere::SetRadius( const float r ) {
	radius = r;
}

INLINE const Vec3 &Sphere::GetOrigin( void ) const {
	return origin;
}

INLINE float Sphere::GetRadius( void ) const {
	return radius;
}

INLINE bool Sphere::IsCleared( void ) const {
	return ( radius < 0.0f );
}

INLINE bool Sphere::AddPoint( const Vec3 &p ) {
	if ( radius < 0.0f ) {
		origin = p;
		radius = 0.0f;
		return true;
	}
	else {
		float r = ( p - origin ).LengthSqr();
		if ( r > radius * radius ) {
			r = sqrtf( r );
			origin += ( p - origin ) * 0.5f * (1.0f - radius / r );
			radius += 0.5f * ( r - radius );
			return true;
		}
		return false;
	}
}

INLINE bool Sphere::AddSphere( const Sphere &s ) {
	if ( radius < 0.0f ) {
		origin = s.origin;
		radius = s.radius;
		return true;
	}
	else {
		float r = ( s.origin - origin ).LengthSqr();
		if ( r > ( radius + s.radius ) * ( radius + s.radius ) ) {
			r = sqrtf( r );
			origin += ( s.origin - origin ) * 0.5f * (1.0f - radius / ( r + s.radius ) );
			radius += 0.5f * ( ( r + s.radius ) - radius );
			return true;
		}
		return false;
	}
}

INLINE Sphere Sphere::Expand( const float d ) const {
	return Sphere( origin, radius + d );
}

INLINE Sphere &Sphere::ExpandSelf( const float d ) {
	radius += d;
	return *this;
}

INLINE Sphere Sphere::Translate( const Vec3 &translation ) const {
	return Sphere( origin + translation, radius );
}

INLINE Sphere &Sphere::TranslateSelf( const Vec3 &translation ) {
	origin += translation;
	return *this;
}

INLINE bool Sphere::ContainsPoint( const Vec3 &p ) const {
	if ( ( p - origin ).LengthSqr() > radius * radius ) {
		return false;
	}
	return true;
}

INLINE bool Sphere::IntersectsSphere( const Sphere &s ) const {
	float r = s.radius + radius;
	if ( ( s.origin - origin ).LengthSqr() > r * r ) {
		return false;
	}
	return true;
}

INLINE void Sphere::FromPointTranslation( const Vec3 &point, const Vec3 &translation ) {
	origin = point + 0.5f * translation;
	radius = sqrtf( 0.5f * translation.LengthSqr() );
}

INLINE void Sphere::FromSphereTranslation( const Sphere &sphere, const Vec3 &start, const Vec3 &translation ) {
	origin = start + sphere.origin + 0.5f * translation;
	radius = sqrtf( 0.5f * translation.LengthSqr() ) + sphere.radius;
}

INLINE void Sphere::FromPointRotation( const Vec3 &point, const Rotation &rotation ) {
	Vec3 end = rotation * point;
	origin = ( point + end ) * 0.5f;
	radius = sqrtf( 0.5f * ( end - point ).LengthSqr() );
}

INLINE void Sphere::FromSphereRotation( const Sphere &sphere, const Vec3 &start, const Rotation &rotation ) {
	Vec3 end = rotation * sphere.origin;
	origin = start + ( sphere.origin + end ) * 0.5f;
	radius = sqrtf( 0.5f * ( end - sphere.origin ).LengthSqr() ) + sphere.radius;
}

INLINE void Sphere::AxisProjection( const Vec3 &dir, float &min, float &max ) const {
	float d;
	d = dir * origin;
	min = d - radius;
	max = d + radius;
}

#endif /* !__BV_SPHERE_H__ */
