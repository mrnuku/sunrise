#include "precompiled.h"
#pragma hdrstop

Bounds bounds_zero( vec3_origin, vec3_origin );

/*
============
Bounds::GetRadius
============
*/
float Bounds::GetRadius( void ) const {

	int		i;
	float	total, b0, b1;

	total = 0.0f;
	for ( i = 0; i < 3; i++ ) {
		b0 = (float)fabs( b[0][i] );
		b1 = (float)fabs( b[1][i] );
		if ( b0 > b1 ) {
			total += b0 * b0;
		} else {
			total += b1 * b1;
		}
	}
	return sqrtf( total );
}

/*
============
Bounds::GetRadius
============
*/
float Bounds::GetRadius( const Vec3 &center ) const {
	int		i;
	float	total, b0, b1;

	total = 0.0f;
	for ( i = 0; i < 3; i++ ) {
		b0 = (float)fabs( center[i] - b[0][i] );
		b1 = (float)fabs( b[1][i] - center[i] );
		if ( b0 > b1 ) {
			total += b0 * b0;
		} else {
			total += b1 * b1;
		}
	}
	return sqrtf( total );
}

/*
================
Bounds::PlaneDistance
================
*/
float Bounds::PlaneDistance( const Plane &plane ) const {
	Vec3 center;
	float d1, d2;

	center = ( b[0] + b[1] ) * 0.5f;

	d1 = plane.Distance( center );
	d2 = fabs( ( b[1][0] - center[0] ) * plane.Normal()[0] ) +
			fabs( ( b[1][1] - center[1] ) * plane.Normal()[1] ) +
				fabs( ( b[1][2] - center[2] ) * plane.Normal()[2] );

	if ( d1 - d2 > 0.0f ) {
		return d1 - d2;
	}
	if ( d1 + d2 < 0.0f ) {
		return d1 + d2;
	}
	return 0.0f;
}

/*
============
Bounds::LineIntersection

  Returns true if the line intersects the bounds between the start and end point.
============
*/
bool Bounds::LineIntersection( const Vec3 &start, const Vec3 &end ) const {
    float ld[3];
	Vec3 center = ( b[0] + b[1] ) * 0.5f;
	Vec3 extents = b[1] - center;
    Vec3 lineDir = 0.5f * ( end - start );
    Vec3 lineCenter = start + lineDir;
    Vec3 dir = lineCenter - center;

    ld[0] = fabs( lineDir[0] );
	if ( fabs( dir[0] ) > extents[0] + ld[0] ) {
        return false;
	}

    ld[1] = fabs( lineDir[1] );
	if ( fabs( dir[1] ) > extents[1] + ld[1] ) {
        return false;
	}

    ld[2] = fabs( lineDir[2] );
	if ( fabs( dir[2] ) > extents[2] + ld[2] ) {
        return false;
	}

    Vec3 cross = lineDir.Cross( dir );

	if ( fabs( cross[0] ) > extents[1] * ld[2] + extents[2] * ld[1] ) {
        return false;
	}

	if ( fabs( cross[1] ) > extents[0] * ld[2] + extents[2] * ld[0] ) {
        return false;
	}

	if ( fabs( cross[2] ) > extents[0] * ld[1] + extents[1] * ld[0] ) {
        return false;
	}

    return true;
}

/*
============
Bounds::RayIntersection

  Returns true if the ray intersects the bounds.
  The ray can intersect the bounds in both directions from the start point.
  If start is inside the bounds it is considered an intersection with scale = 0
============
*/
bool Bounds::RayIntersection( const Vec3 &start, const Vec3 &dir, float &scale ) const {
	int i, ax0, ax1, ax2, side, inside;
	float f;
	Vec3 hit;

	ax0 = -1;
	inside = 0;
	for ( i = 0; i < 3; i++ ) {
		if ( start[i] < b[0][i] ) {
			side = 0;
		}
		else if ( start[i] > b[1][i] ) {
			side = 1;
		}
		else {
			inside++;
			continue;
		}
		if ( dir[i] == 0.0f ) {
			continue;
		}
		f = ( start[i] - b[side][i] );
		if ( ax0 < 0 || fabs( f ) > fabs( scale * dir[i] ) ) {
			scale = - ( f / dir[i] );
			ax0 = i;
		}
	}

	if ( ax0 < 0 ) {
		scale = 0.0f;
		// return true if the start point is inside the bounds
		return ( inside == 3 );
	}

	ax1 = (ax0+1)%3;
	ax2 = (ax0+2)%3;
	hit[ax1] = start[ax1] + scale * dir[ax1];
	hit[ax2] = start[ax2] + scale * dir[ax2];

	return ( hit[ax1] >= b[0][ax1] && hit[ax1] <= b[1][ax1] &&
				hit[ax2] >= b[0][ax2] && hit[ax2] <= b[1][ax2] );
}

/*
============
Bounds::FromTransformedBounds
============
*/
void Bounds::FromTransformedBounds( const Bounds &bounds, const Vec3 &origin, const Mat3 &axis ) {
	int i;
	Vec3 center, extents, rotatedExtents;

	center = (bounds[0] + bounds[1]) * 0.5f;
	extents = bounds[1] - center;

	for ( i = 0; i < 3; i++ ) {
		rotatedExtents[i] = fabs( extents[0] * axis[0][i] ) +
							fabs( extents[1] * axis[1][i] ) +
							fabs( extents[2] * axis[2][i] );
	}

	center = origin + center * axis;
	b[0] = center - rotatedExtents;
	b[1] = center + rotatedExtents;
}

/*
============
Bounds::FromPoints

  Most tight bounds for a point set.
============
*/
void Bounds::FromPoints( const Vec3 * points, const int numPoints ) {

	Common::MinMax( b[0], b[1], points, numPoints );
}

/*
============
Bounds::FromPointTranslation

  Most tight bounds for the translational movement of the given point.
============
*/
void Bounds::FromPointTranslation( const Vec3 &point, const Vec3 &translation ) {
	int i;

	for ( i = 0; i < 3; i++ ) {
		if ( translation[i] < 0.0f ) {
			b[0][i] = point[i] + translation[i];
			b[1][i] = point[i];
		}
		else {
			b[0][i] = point[i];
			b[1][i] = point[i] + translation[i];
		}
	}
}

/*
============
Bounds::FromBoundsTranslation

  Most tight bounds for the translational movement of the given bounds.
============
*/
void Bounds::FromBoundsTranslation( const Bounds &bounds, const Vec3 &origin, const Mat3 &axis, const Vec3 &translation ) {
	int i;

	if ( axis.IsRotated() ) {
		FromTransformedBounds( bounds, origin, axis );
	}
	else {
		b[0] = bounds[0] + origin;
		b[1] = bounds[1] + origin;
	}
	for ( i = 0; i < 3; i++ ) {
		if ( translation[i] < 0.0f ) {
			b[0][i] += translation[i];
		}
		else {
			b[1][i] += translation[i];
		}
	}
}

/*
================
BoundsForPointRotation

  only for rotations < 180 degrees
================
*/
Bounds BoundsForPointRotation( const Vec3 &start, const Rotation &rotation ) {
	int i;
	float radiusSqr;
	Vec3 v1, v2;
	Vec3 origin, axis, end;
	Bounds bounds;

	end = start * rotation;
	axis = rotation.GetVec();
	origin = rotation.GetOrigin() + axis * ( axis * ( start - rotation.GetOrigin() ) );
	radiusSqr = ( start - origin ).LengthSqr();
	v1 = ( start - origin ).Cross( axis );
	v2 = ( end - origin ).Cross( axis );

	for ( i = 0; i < 3; i++ ) {
		// if the derivative changes sign along this axis during the rotation from start to end
		if ( ( v1[i] > 0.0f && v2[i] < 0.0f ) || ( v1[i] < 0.0f && v2[i] > 0.0f ) ) {
			if ( ( 0.5f * (start[i] + end[i]) - origin[i] ) > 0.0f ) {
				bounds[0][i] = Min( start[i], end[i] );
				bounds[1][i] = origin[i] + sqrtf( radiusSqr * ( 1.0f - axis[i] * axis[i] ) );
			}
			else {
				bounds[0][i] = origin[i] - sqrtf( radiusSqr * ( 1.0f - axis[i] * axis[i] ) );
				bounds[1][i] = Max( start[i], end[i] );
			}
		}
		else if ( start[i] > end[i] ) {
			bounds[0][i] = end[i];
			bounds[1][i] = start[i];
		}
		else {
			bounds[0][i] = start[i];
			bounds[1][i] = end[i];
		}
	}

	return bounds;
}

/*
============
Bounds::FromPointRotation

  Most tight bounds for the rotational movement of the given point.
============
*/
void Bounds::FromPointRotation( const Vec3 &point, const Rotation &rotation ) {
	float radius;

	if ( fabs( rotation.GetAngle() ) < 180.0f ) {
		(*this) = BoundsForPointRotation( point, rotation );
	}
	else {

		radius = ( point - rotation.GetOrigin() ).Length();

		// FIXME: these bounds are usually way larger
		b[0].Set( -radius, -radius, -radius );
		b[1].Set( radius, radius, radius );
	}
}

/*
============
Bounds::FromBoundsRotation

  Most tight bounds for the rotational movement of the given bounds.
============
*/
void Bounds::FromBoundsRotation( const Bounds &bounds, const Vec3 &origin, const Mat3 &axis, const Rotation &rotation ) {
	int i;
	float radius;
	Vec3 point;
	Bounds rBounds;

	if ( fabs( rotation.GetAngle() ) < 180.0f ) {

		(*this) = BoundsForPointRotation( bounds[0] * axis + origin, rotation );
		for ( i = 1; i < 8; i++ ) {
			point[0] = bounds[(i^(i>>1))&1][0];
			point[1] = bounds[(i>>1)&1][1];
			point[2] = bounds[(i>>2)&1][2];
			(*this) += BoundsForPointRotation( point * axis + origin, rotation );
		}
	}
	else {

		point = (bounds[1] - bounds[0]) * 0.5f;
		radius = (bounds[1] - point).Length() + (point - rotation.GetOrigin()).Length();

		// FIXME: these bounds are usually way larger
		b[0].Set( -radius, -radius, -radius );
		b[1].Set( radius, radius, radius );
	}
}
