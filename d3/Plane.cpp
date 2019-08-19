#include "precompiled.h"
#pragma hdrstop

Plane plane_origin( 0.0f, 0.0f, 0.0f, 0.0f );

int Plane::Type( void ) const {
	if( Normal( )[0] == 0.0f ) {
		if( Normal( )[1] == 0.0f )
			return Normal( )[2] > 0.0f ? PLANETYPE_Z : PLANETYPE_NEGZ;
		else if( Normal( )[2] == 0.0f )
			return Normal( )[1] > 0.0f ? PLANETYPE_Y : PLANETYPE_NEGY;
		else
			return PLANETYPE_ZEROX;
	}
	else if( Normal( )[1] == 0.0f ) {
		if( Normal( )[2] == 0.0f )
			return Normal( )[0] > 0.0f ? PLANETYPE_X : PLANETYPE_NEGX;
		else
			return PLANETYPE_ZEROY;
	} else if( Normal( )[2] == 0.0f )
		return PLANETYPE_ZEROZ;
	else
		return PLANETYPE_NONAXIAL;
}

bool Plane::HeightFit( const Vec3 *points, const int numPoints ) {
	int i;
	float sumXX = 0.0f, sumXY = 0.0f, sumXZ = 0.0f;
	float sumYY = 0.0f, sumYZ = 0.0f;
	Vec3 sum, average, dir;
	if( numPoints == 1 ) {
		d_a = 0.0f;
		d_b = 0.0f;
		d_c = 1.0f;
		d_d = -points[0].z;
		return true;
	}
	if( numPoints == 2 ) {
		dir = points[1] - points[0];
		Normal( ) = dir.Cross( Vec3( 0, 0, 1 ) ).Cross( dir );
		Normalize( );
		d_d = -( Normal( ) * points[0] );
		return true;
	}
	sum.Zero( );
	for ( i = 0; i < numPoints; i++)
		sum += points[i];
	average = sum / ( float )numPoints;
	for ( i = 0; i < numPoints; i++ ) {
		dir = points[i] - average;
		sumXX += dir.x * dir.x;
		sumXY += dir.x * dir.y;
		sumXZ += dir.x * dir.z;
		sumYY += dir.y * dir.y;
		sumYZ += dir.y * dir.z;
	}
	Mat2 m( sumXX, sumXY, sumXY, sumYY );
	if( !m.InverseSelf( ) )
		return false;
	d_a = - sumXZ * m[0][0] - sumYZ * m[0][1];
	d_b = - sumXZ * m[1][0] - sumYZ * m[1][1];
	d_c = 1.0f;
	Normalize( );
	d_d = -( d_a * average.x + d_b * average.y + d_c * average.z );
	return true;
}

bool Plane::PlaneIntersection( const Plane & plane, Vec3 & start, Vec3 & dir ) const {
	float n00 = Normal( ).LengthSqr( );
	float n01 = Normal( ) * plane.Normal( );
	float n11 = plane.Normal( ).LengthSqr( );
	float det = n00 * n11 - n01 * n01;
	if( fabsf( det ) < 1e-6f )
		return false;
	float invDet = 1.0f / det;
	float f0 = ( n01 * plane.d_d - n11 * d_d ) * invDet;
	float f1 = ( n01 * d_d - n00 * plane.d_d ) * invDet;
	dir = Normal( ).Cross( plane.Normal( ) );
	start = f0 * Normal( ) + f1 * plane.Normal( );
	return true;
}

const char * Plane::ToString( int precision ) const {
	return Str::FloatArrayToString( ToFloatPtr( ), GetDimension( ), precision );
}

bool Plane::Compare( const Plane & p, const float epsilon ) const {
	if ( fabsf( d_a - p.d_a ) > epsilon )
		return false;			
	if ( fabsf( d_b - p.d_b ) > epsilon )
		return false;
	if ( fabsf( d_c - p.d_c ) > epsilon )
		return false;
	if ( fabsf( d_d - p.d_d ) > epsilon )
		return false;
	return true;
}

bool Plane::FixDegeneracies( float distEpsilon ) {
	bool fixedNormal = FixDegenerateNormal( );
	// only fix dist if the normal was degenerate
	if ( fixedNormal ) {
		if ( fabsf( d_d - floorf( d_d + 0.5f ) ) < distEpsilon )
			d_d = floorf( d_d + 0.5f );
	}
	return fixedNormal;
}

bool Plane::FromPoints( const Vec3 & p1, const Vec3 & p2, const Vec3 & p3, bool fixDegenerate ) {
	Normal( ) = ( p1 - p2 ).Cross( p3 - p2 );
	if( Normalize( fixDegenerate ) == 0.0f )
		return false;
	d_d = -( Normal( ) * p2 );
	return true;
}

bool Plane::FromVecs( const Vec3 & dir1, const Vec3 & dir2, const Vec3 & p, bool fixDegenerate ) {
	Normal( ) = dir1.Cross( dir2 );
	if ( Normalize( fixDegenerate ) == 0.0f )
		return false;
	d_d = -( Normal( ) * p );
	return true;
}

bool Plane::LineIntersection( const Vec3 & start, const Vec3 & end ) const {
	float d1, d2, fraction;
	d1 = Normal( ) * start + d_d;
	d2 = Normal( ) * end + d_d;
	if ( d1 == d2 )
		return false;
	if( d1 > 0.0f && d2 > 0.0f )
		return false;
	if( d1 < 0.0f && d2 < 0.0f )
		return false;
	fraction = ( d1 / ( d1 - d2 ) );
	return ( fraction >= 0.0f && fraction <= 1.0f );
}

bool Plane::LineIntersection( const Vec3 & start, const Vec3 & end, float & fraction ) const {
	float d1 = Normal( ) * start + d_d;
	float d2 = Normal( ) * end + d_d;
	if( d1 == d2 || ( d1 > 0.0f && d2 > 0.0f ) || ( d1 < 0.0f && d2 < 0.0f ) )
		return false;
	fraction = ( d1 / ( d1 - d2 ) );
	return ( fraction >= 0.0f && fraction <= 1.0f );
}

bool Plane::RayIntersection( const Vec3 &start, const Vec3 &dir, float &scale ) const {
	float d1, d2;
	d1 = Normal( ) * start + d_d;
	d2 = Normal( ) * dir;
	if( d2 == 0.0f )
		return false;
	scale = -( d1 / d2 );
	return true;
}
