#include "precompiled.h"
#pragma hdrstop

Sphere sphere_zero( vec3_origin, 0.0f );


/*
================
Sphere::PlaneDistance
================
*/
float Sphere::PlaneDistance( const Plane &plane ) const {
	float d;

	d = plane.Distance( origin );
	if ( d > radius ) {
		return d - radius;
	}
	if ( d < -radius ) {
		return d + radius;
	}
	return 0.0f;
}

/*
================
Sphere::PlaneSide
================
*/
int Sphere::PlaneSide( const Plane &plane, const float epsilon ) const {
	float d;

	d = plane.Distance( origin );
	if ( d > radius + epsilon ) {
		return PLANESIDE_FRONT;
	}
	if ( d < -radius - epsilon ) {
		return PLANESIDE_BACK;
	}
	return PLANESIDE_CROSS;
}

/*
============
Sphere::LineIntersection

  Returns true if the line intersects the sphere between the start and end point.
============
*/
bool Sphere::LineIntersection( const Vec3 &start, const Vec3 &end ) const {
	Vec3 r, s, e;
	float a;

	s = start - origin;
	e = end - origin;
	r = e - s;
	a = -s * r;
	if ( a <= 0 ) {
		return ( s * s < radius * radius );
	}
	else if ( a >= r * r ) {
		return ( e * e < radius * radius );
	}
	else {
		r = s + ( a / ( r * r ) ) * r;
		return ( r * r < radius * radius );
	}
}

/*
============
Sphere::RayIntersection

  Returns true if the ray intersects the sphere.
  The ray can intersect the sphere in both directions from the start point.
  If start is inside the sphere then scale1 < 0 and scale2 > 0.
============
*/
bool Sphere::RayIntersection( const Vec3 &start, const Vec3 &dir, float &scale1, float &scale2 ) const {
	double a, b, c, d, sqrtd;
	Vec3 p;

	p = start - origin;
	a = dir * dir;
	b = dir * p;
	c = p * p - radius * radius;
	d = b * b - c * a;

	if ( d < 0.0f ) {
		return false;
	}

	sqrtd = sqrt( d );
	a = 1.0f / a;

	scale1 = ( float )( ( -b + sqrtd ) * a );
	scale2 = ( float )( ( -b - sqrtd ) * a );

	return true;
}

/*
============
Sphere::FromPoints

  Tight sphere for a point set.
============
*/
void Sphere::FromPoints( const Vec3 *points, const int numPoints ) {
	int i;
	float radiusSqr, dist;
	Vec3 mins, maxs;

	Common::MinMax( mins, maxs, points, numPoints );

	origin = ( mins + maxs ) * 0.5f;

	radiusSqr = 0.0f;
	for ( i = 0; i < numPoints; i++ ) {
		dist = ( points[i] - origin ).LengthSqr();
		if ( dist > radiusSqr ) {
			radiusSqr = dist;
		}
	}
	radius = sqrtf( radiusSqr );
}
