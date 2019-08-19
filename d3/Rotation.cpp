#include "precompiled.h"
#pragma hdrstop

/*
============
Rotation::ToAngles
============
*/
Angles Rotation::ToAngles( void ) const {
	return ToMat3().ToAngles();
}

/*
============
Rotation::ToQuat
============
*/
Quat Rotation::ToQuat( void ) const {
	float a, s, c;

	a = angle * ( DEG2RAD( 0.5f ) );
	SinCos( a, s, c );
	return Quat( vec.x * s, vec.y * s, vec.z * s, c );
}

/*
============
Rotation::toMat3
============
*/
const Mat3 &Rotation::ToMat3( void ) const {
	float wx, wy, wz;
	float xx, yy, yz;
	float xy, xz, zz;
	float x2, y2, z2;
	float a, c, s, x, y, z;

	if ( axisValid ) {
		return axis;
	}

	a = angle * ( DEG2RAD( 0.5f ) );
	SinCos( a, s, c );

	x = vec[0] * s;
	y = vec[1] * s;
	z = vec[2] * s;

	x2 = x + x;
	y2 = y + y;
	z2 = z + z;

	xx = x * x2;
	xy = x * y2;
	xz = x * z2;

	yy = y * y2;
	yz = y * z2;
	zz = z * z2;

	wx = c * x2;
	wy = c * y2;
	wz = c * z2;

	axis[ 0 ][ 0 ] = 1.0f - ( yy + zz );
	axis[ 0 ][ 1 ] = xy - wz;
	axis[ 0 ][ 2 ] = xz + wy;

	axis[ 1 ][ 0 ] = xy + wz;
	axis[ 1 ][ 1 ] = 1.0f - ( xx + zz );
	axis[ 1 ][ 2 ] = yz - wx;

	axis[ 2 ][ 0 ] = xz - wy;
	axis[ 2 ][ 1 ] = yz + wx;
	axis[ 2 ][ 2 ] = 1.0f - ( xx + yy );

	axisValid = true;

	return axis;
}

/*
============
Rotation::ToMat4
============
*/
Mat4 Rotation::ToMat4( void ) const {
	//return ToMat3().ToMat4();
	Mat4 mat4Axis;
	float wx, wy, wz;
	float xx, yy, yz;
	float xy, xz, zz;
	float x2, y2, z2;
	float a, c, s, x, y, z;

	a = angle * ( DEG2RAD( 0.5f ) );
	SinCos( a, s, c );

	x = vec[0] * s;
	y = vec[1] * s;
	z = vec[2] * s;

	x2 = x + x;
	y2 = y + y;
	z2 = z + z;

	xx = x * x2;
	xy = x * y2;
	xz = x * z2;

	yy = y * y2;
	yz = y * z2;
	zz = z * z2;

	wx = c * x2;
	wy = c * y2;
	wz = c * z2;

	mat4Axis[ 0 ][ 0 ] = 1.0f - ( yy + zz );
	mat4Axis[ 0 ][ 1 ] = xy - wz;
	mat4Axis[ 0 ][ 2 ] = xz + wy;
	mat4Axis[ 0 ][ 3 ] = 0.0f;

	mat4Axis[ 1 ][ 0 ] = xy + wz;
	mat4Axis[ 1 ][ 1 ] = 1.0f - ( xx + zz );
	mat4Axis[ 1 ][ 2 ] = yz - wx;
	mat4Axis[ 1 ][ 3 ] = 0.0f;

	mat4Axis[ 2 ][ 0 ] = xz - wy;
	mat4Axis[ 2 ][ 1 ] = yz + wx;
	mat4Axis[ 2 ][ 2 ] = 1.0f - ( xx + yy );
	mat4Axis[ 2 ][ 3 ] = 0.0f;

	mat4Axis[ 3 ][ 0 ] = 0.0f;
	mat4Axis[ 3 ][ 1 ] = 0.0f;
	mat4Axis[ 3 ][ 2 ] = 0.0f;
	mat4Axis[ 3 ][ 3 ] = 1.0f;

	return mat4Axis;
}

/*
============
Rotation::ToAngularVelocity
============
*/
Vec3 Rotation::ToAngularVelocity( void ) const {
	return vec * DEG2RAD( angle );
}

/*
============
Rotation::Normalize180
============
*/
void Rotation::Normalize180( void ) {
	angle -= floor( angle / 360.0f ) * 360.0f;
	if ( angle > 180.0f ) {
		angle -= 360.0f;
	}
	else if ( angle < -180.0f ) {
		angle += 360.0f;
	}
}

/*
============
Rotation::Normalize360
============
*/
void Rotation::Normalize360( void ) {
	angle -= floor( angle / 360.0f ) * 360.0f;
	if ( angle > 360.0f ) {
		angle -= 360.0f;
	}
	else if ( angle < 0.0f ) {
		angle += 360.0f;
	}
}
