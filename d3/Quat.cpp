#include "precompiled.h"
#pragma hdrstop


/*
=====================
Quat::ToAngles
=====================
*/
Angles Quat::ToAngles( void ) const {
	return ToMat3().ToAngles();
}

/*
=====================
Quat::ToRotation
=====================
*/
Rotation Quat::ToRotation( void ) const {
	Vec3 vec;
	float angle;

	vec.x = x;
	vec.y = y;
	vec.z = z;
	angle = ACos( w );
	if ( angle == 0.0f ) {
		vec.Set( 0.0f, 0.0f, 1.0f );
	} else {
		//vec *= (1.0f / sin( angle ));
		vec.Normalize();
		vec.FixDegenerateNormal();
		angle *= RAD2DEG( 2.0f );
	}
	return Rotation( vec3_origin, vec, angle );
}

/*
=====================
Quat::ToMat3
=====================
*/
Mat3 Quat::ToMat3( void ) const {
	Mat3	mat;
	float	wx, wy, wz;
	float	xx, yy, yz;
	float	xy, xz, zz;
	float	x2, y2, z2;

	x2 = x + x;
	y2 = y + y;
	z2 = z + z;

	xx = x * x2;
	xy = x * y2;
	xz = x * z2;

	yy = y * y2;
	yz = y * z2;
	zz = z * z2;

	wx = w * x2;
	wy = w * y2;
	wz = w * z2;

	mat[ 0 ][ 0 ] = 1.0f - ( yy + zz );
	mat[ 0 ][ 1 ] = xy - wz;
	mat[ 0 ][ 2 ] = xz + wy;

	mat[ 1 ][ 0 ] = xy + wz;
	mat[ 1 ][ 1 ] = 1.0f - ( xx + zz );
	mat[ 1 ][ 2 ] = yz - wx;

	mat[ 2 ][ 0 ] = xz - wy;
	mat[ 2 ][ 1 ] = yz + wx;
	mat[ 2 ][ 2 ] = 1.0f - ( xx + yy );

	return mat;
}

/*
=====================
Quat::ToMat4
=====================
*/
Mat4 Quat::ToMat4( void ) const {
	return ToMat3().ToMat4();
}

/*
=====================
Quat::ToCQuat
=====================
*/
CQuat Quat::ToCQuat( void ) const {
	if ( w < 0.0f ) {
		return CQuat( -x, -y, -z );
	}
	return CQuat( x, y, z );
}

/*
============
Quat::ToAngularVelocity
============
*/
Vec3 Quat::ToAngularVelocity( void ) const {
	Vec3 vec;

	vec.x = x;
	vec.y = y;
	vec.z = z;
	vec.Normalize();
	return vec * ACos( w );
}

/*
=============
Quat::ToString
=============
*/
const char *Quat::ToString( int precision ) const {
	return Str::FloatArrayToString( ToFloatPtr(), GetDimension(), precision );
}

/*
=====================
Quat::Slerp

Spherical linear interpolation between two quaternions.
=====================
*/
Quat &Quat::Slerp( const Quat &from, const Quat &to, float t ) {
	Quat	temp;
	float	omega, cosom, sinom, scale0, scale1;

	if ( t <= 0.0f ) {
		*this = from;
		return *this;
	}

	if ( t >= 1.0f ) {
		*this = to;
		return *this;
	}

	if ( from == to ) {
		*this = to;
		return *this;
	}

	cosom = from.x * to.x + from.y * to.y + from.z * to.z + from.w * to.w;
	if ( cosom < 0.0f ) {
		temp = -to;
		cosom = -cosom;
	} else {
		temp = to;
	}

	if ( ( 1.0f - cosom ) > 1e-6f ) {
#if 0
		omega = acos( cosom );
		sinom = 1.0f / sin( omega );
		scale0 = sin( ( 1.0f - t ) * omega ) * sinom;
		scale1 = sin( t * omega ) * sinom;
#else
		scale0 = 1.0f - cosom * cosom;
		sinom = InvSqrt( scale0 );
		omega = ATan16( scale0 * sinom, cosom );
		scale0 = Sin16( ( 1.0f - t ) * omega ) * sinom;
		scale1 = Sin16( t * omega ) * sinom;
#endif
	} else {
		scale0 = 1.0f - t;
		scale1 = t;
	}

	*this = ( scale0 * from ) + ( scale1 * temp );
	return *this;
}

/*
=============
CQuat::ToAngles
=============
*/
Angles CQuat::ToAngles( void ) const {
	return ToQuat().ToAngles();
}

/*
=============
CQuat::ToRotation
=============
*/
Rotation CQuat::ToRotation( void ) const {
	return ToQuat().ToRotation();
}

/*
=============
CQuat::ToMat3
=============
*/
Mat3 CQuat::ToMat3( void ) const {
	return ToQuat().ToMat3();
}

/*
=============
CQuat::ToMat4
=============
*/
Mat4 CQuat::ToMat4( void ) const {
	return ToQuat().ToMat4();
}

/*
=============
CQuat::ToString
=============
*/
const char *CQuat::ToString( int precision ) const {
	return Str::FloatArrayToString( ToFloatPtr(), GetDimension(), precision );
}
