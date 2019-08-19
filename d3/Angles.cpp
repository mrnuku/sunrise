#include "precompiled.h"
#pragma hdrstop

Angles ang_zero( 0.0f, 0.0f, 0.0f );

/*
=================
Angles::Normalize360

returns angles normalized to the range [0 <= angle < 360]
=================
*/
Angles& Angles::Normalize360( void ) {
	int i;

	for ( i = 0; i < 3; i++ ) {
		if ( ( (*this)[i] >= 360.0f ) || ( (*this)[i] < 0.0f ) ) {
			(*this)[i] -= floor( (*this)[i] / 360.0f ) * 360.0f;

			if ( (*this)[i] >= 360.0f ) {
				(*this)[i] -= 360.0f;
			}
			if ( (*this)[i] < 0.0f ) {
				(*this)[i] += 360.0f;
			}
		}
	}

	return *this;
}

/*
=================
Angles::Normalize180

returns angles normalized to the range [-180 < angle <= 180]
=================
*/
Angles& Angles::Normalize180( void ) {
	Normalize360();

	if ( pitch > 180.0f ) {
		pitch -= 360.0f;
	}
	
	if ( yaw > 180.0f ) {
		yaw -= 360.0f;
	}

	if ( roll > 180.0f ) {
		roll -= 360.0f;
	}
	return *this;
}

/*
=================
Angles::ToVectors
=================
*/
void Angles::ToVectors( Vec3 *forward, Vec3 *right, Vec3 *up ) const {
	float sr, sp, sy, cr, cp, cy;
	
	SinCos( DEG2RAD( yaw ), sy, cy );
	SinCos( DEG2RAD( pitch ), sp, cp );
	SinCos( DEG2RAD( roll ), sr, cr );

	if ( forward ) {
		forward->Set( cp * cy, cp * sy, -sp );
	}

	if ( right ) {
		right->Set( -sr * sp * cy + cr * sy, -sr * sp * sy + -cr * cy, -sr * cp );
	}

	if ( up ) {
		up->Set( cr * sp * cy + -sr * -sy, cr * sp * sy + -sr * cy, cr * cp );
	}
}

/*
=================
Angles::ToForward
=================
*/
Vec3 Angles::ToForward( void ) const {
	float sp, sy, cp, cy;
	
	SinCos( DEG2RAD( yaw ), sy, cy );
	SinCos( DEG2RAD( pitch ), sp, cp );

	return Vec3( cp * cy, cp * sy, -sp );
}

/*
=================
Angles::ToQuat
=================
*/
Quat Angles::ToQuat( void ) const {
	float sx, cx, sy, cy, sz, cz;
	float sxcy, cxcy, sxsy, cxsy;

	SinCos( DEG2RAD( yaw ) * 0.5f, sz, cz );
	SinCos( DEG2RAD( pitch ) * 0.5f, sy, cy );
	SinCos( DEG2RAD( roll ) * 0.5f, sx, cx );

	sxcy = sx * cy;
	cxcy = cx * cy;
	sxsy = sx * sy;
	cxsy = cx * sy;

	return Quat( cxsy*sz - sxcy*cz, -cxsy*cz - sxcy*sz, sxsy*cz - cxcy*sz, cxcy*cz + sxsy*sz );
}

/*
=================
Angles::ToRotation
=================
*/
Rotation Angles::ToRotation( void ) const {
	Vec3 vec;
	float angle, w;
	float sx, cx, sy, cy, sz, cz;
	float sxcy, cxcy, sxsy, cxsy;

	if ( pitch == 0.0f ) {
		if ( yaw == 0.0f ) {
			return Rotation( vec3_origin, Vec3( -1.0f, 0.0f, 0.0f ), roll );
		}
		if ( roll == 0.0f ) {
			return Rotation( vec3_origin, Vec3( 0.0f, 0.0f, -1.0f ), yaw );
		}
	} else if ( yaw == 0.0f && roll == 0.0f ) {
		return Rotation( vec3_origin, Vec3( 0.0f, -1.0f, 0.0f ), pitch );
	}

	SinCos( DEG2RAD( yaw ) * 0.5f, sz, cz );
	SinCos( DEG2RAD( pitch ) * 0.5f, sy, cy );
	SinCos( DEG2RAD( roll ) * 0.5f, sx, cx );

	sxcy = sx * cy;
	cxcy = cx * cy;
	sxsy = sx * sy;
	cxsy = cx * sy;

	vec.x =  cxsy * sz - sxcy * cz;
	vec.y = -cxsy * cz - sxcy * sz;
	vec.z =  sxsy * cz - cxcy * sz;
	w =		 cxcy * cz + sxsy * sz;
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
=================
Angles::ToAngularVelocity
=================
*/
Vec3 Angles::ToAngularVelocity( void ) const {
	Rotation rotation = Angles::ToRotation();
	return rotation.GetVec() * DEG2RAD( rotation.GetAngle() );
}

/*
=============
Angles::ToString
=============
*/
const char *Angles::ToString( int precision ) const {

	return Str::FloatArrayToString( ToFloatPtr(), GetDimension(), precision );
}
