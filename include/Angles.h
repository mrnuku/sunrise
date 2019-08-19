// Copyright (C) 2004 Id Software, Inc.
//

#ifndef __MATH_ANGLES_H__
#define __MATH_ANGLES_H__

/*
===============================================================================

	Euler angles

===============================================================================
*/

// angle indexes
#define	PITCH				0		// up / down
#define	YAW					1		// left / right
#define	ROLL				2		// fall over

class Vec3;
class Quat;
class Rotation;
class Mat3;
class Mat4;

class Angles {
public:
	float			pitch;
	float			yaw;
	float			roll;

					Angles( void );
					Angles( float pitch, float yaw, float roll );
					explicit Angles( const Vec3 &v );

	void 			Set( float pitch, float yaw, float roll );
	Angles &		Zero( void );

	float			operator[]( int index ) const;
	float &			operator[]( int index );
	Angles			operator-() const;			// negate angles, in general not the inverse rotation
	Angles &		operator=( const Angles &a );
	Angles			operator+( const Angles &a ) const;
	Angles &		operator+=( const Angles &a );
	Angles			operator-( const Angles &a ) const;
	Angles &		operator-=( const Angles &a );
	Angles			operator*( const float a ) const;
	Angles &		operator*=( const float a );
	Angles			operator/( const float a ) const;
	Angles &		operator/=( const float a );

	friend Angles	operator*( const float a, const Angles &b );

	bool			Compare( const Angles &a ) const;							// exact compare, no epsilon
	bool			Compare( const Angles &a, const float epsilon ) const;	// compare with epsilon
	bool			operator==(	const Angles &a ) const;						// exact compare, no epsilon
	bool			operator!=(	const Angles &a ) const;						// exact compare, no epsilon

	Angles &		Normalize360( void );	// normalizes 'this'
	Angles &		Normalize180( void );	// normalizes 'this'

	void			Clamp( const Angles &min, const Angles &max );

	int				GetDimension( void ) const;

	void			ToVectors( Vec3 *forward, Vec3 *right = NULL, Vec3 *up = NULL ) const;
	Vec3			ToForward( void ) const;
	Quat			ToQuat( void ) const;
	Rotation		ToRotation( void ) const;
	Mat3			ToMat3( void ) const;
	Mat4			ToMat4( void ) const;
	Vec3			ToAngularVelocity( void ) const;
	const float *	ToFloatPtr( void ) const;
	float *			ToFloatPtr( void );
	const char *	ToString( int precision = 2 ) const;
};

extern Angles ang_zero;

INLINE Angles::Angles( void ) {
}

INLINE Angles::Angles( float pitch, float yaw, float roll ) {
	this->pitch = pitch;
	this->yaw	= yaw;
	this->roll	= roll;
}

INLINE Angles::Angles( const Vec3 &v ) {
	this->pitch = v[0];
	this->yaw	= v[1];
	this->roll	= v[2];
}

INLINE void Angles::Set( float pitch, float yaw, float roll ) {
	this->pitch = pitch;
	this->yaw	= yaw;
	this->roll	= roll;
}

INLINE Angles &Angles::Zero( void ) {
	pitch = yaw = roll = 0.0f;
	return *this;
}

INLINE float Angles::operator[]( int index ) const {
	assert( ( index >= 0 ) && ( index < 3 ) );
	return ( &pitch )[ index ];
}

INLINE float &Angles::operator[]( int index ) {
	assert( ( index >= 0 ) && ( index < 3 ) );
	return ( &pitch )[ index ];
}

INLINE Angles Angles::operator-() const {
	return Angles( -pitch, -yaw, -roll );
}

INLINE Angles &Angles::operator=( const Angles &a ) {
	pitch	= a.pitch;
	yaw		= a.yaw;
	roll	= a.roll;
	return *this;
}

INLINE Angles Angles::operator+( const Angles &a ) const {
	return Angles( pitch + a.pitch, yaw + a.yaw, roll + a.roll );
}

INLINE Angles& Angles::operator+=( const Angles &a ) {
	pitch	+= a.pitch;
	yaw		+= a.yaw;
	roll	+= a.roll;

	return *this;
}

INLINE Angles Angles::operator-( const Angles &a ) const {
	return Angles( pitch - a.pitch, yaw - a.yaw, roll - a.roll );
}

INLINE Angles& Angles::operator-=( const Angles &a ) {
	pitch	-= a.pitch;
	yaw		-= a.yaw;
	roll	-= a.roll;

	return *this;
}

INLINE Angles Angles::operator*( const float a ) const {
	return Angles( pitch * a, yaw * a, roll * a );
}

INLINE Angles& Angles::operator*=( float a ) {
	pitch	*= a;
	yaw		*= a;
	roll	*= a;
	return *this;
}

INLINE Angles Angles::operator/( const float a ) const {
	float inva = 1.0f / a;
	return Angles( pitch * inva, yaw * inva, roll * inva );
}

INLINE Angles& Angles::operator/=( float a ) {
	float inva = 1.0f / a;
	pitch	*= inva;
	yaw		*= inva;
	roll	*= inva;
	return *this;
}

INLINE Angles operator*( const float a, const Angles &b ) {
	return Angles( a * b.pitch, a * b.yaw, a * b.roll );
}

INLINE bool Angles::Compare( const Angles &a ) const {
	return ( ( a.pitch == pitch ) && ( a.yaw == yaw ) && ( a.roll == roll ) );
}

INLINE bool Angles::Compare( const Angles &a, const float epsilon ) const {
	if ( fabs( pitch - a.pitch ) > epsilon ) {
		return false;
	}
			
	if ( fabs( yaw - a.yaw ) > epsilon ) {
		return false;
	}

	if ( fabs( roll - a.roll ) > epsilon ) {
		return false;
	}

	return true;
}

INLINE bool Angles::operator==( const Angles &a ) const {
	return Compare( a );
}

INLINE bool Angles::operator!=( const Angles &a ) const {
	return !Compare( a );
}

INLINE void Angles::Clamp( const Angles &min, const Angles &max ) {
	if ( pitch < min.pitch ) {
		pitch = min.pitch;
	} else if ( pitch > max.pitch ) {
		pitch = max.pitch;
	}
	if ( yaw < min.yaw ) {
		yaw = min.yaw;
	} else if ( yaw > max.yaw ) {
		yaw = max.yaw;
	}
	if ( roll < min.roll ) {
		roll = min.roll;
	} else if ( roll > max.roll ) {
		roll = max.roll;
	}
}

INLINE int Angles::GetDimension( void ) const {
	return 3;
}

INLINE const float *Angles::ToFloatPtr( void ) const {
	return &pitch;
}

INLINE float *Angles::ToFloatPtr( void ) {
	return &pitch;
}

INLINE Mat3 Angles::ToMat3( void ) const {

	Mat3 mat;
	float sr, sp, sy, cr, cp, cy;

	SinCos( DEG2RAD( yaw ), sy, cy );
	SinCos( DEG2RAD( pitch ), sp, cp );
	SinCos( DEG2RAD( roll ), sr, cr );

	mat[ 0 ].Set( cp * cy, cp * sy, -sp );
	mat[ 1 ].Set( sr * sp * cy + cr * -sy, sr * sp * sy + cr * cy, sr * cp );
	mat[ 2 ].Set( cr * sp * cy + -sr * -sy, cr * sp * sy + -sr * cy, cr * cp );

	return mat;
}

INLINE Mat4 Angles::ToMat4( void ) const {

	Mat4 mat;
	float sr, sp, sy, cr, cp, cy;

	SinCos( DEG2RAD( yaw ), sy, cy );
	SinCos( DEG2RAD( pitch ), sp, cp );
	SinCos( DEG2RAD( roll ), sr, cr );

	float invsy = -sy;
	float invsr = -sr;
	float mulspcy = sp * cy;
	float mulspsy = sp * sy;

	//mat[ 0 ].Set( cp * cy,		sr * mulspcy + cr * invsy,		cr * mulspcy + invsr * invsy,	0.0f );
	//mat[ 1 ].Set( cp * sy,		sr * mulspsy + cr * cy,			cr * mulspsy + invsr * cy,		0.0f );
	//mat[ 2 ].Set( -sp,			sr * cp,						cr * cp,						0.0f );
	//mat[ 3 ].Set( 0.0f,			0.0f,							0.0f,							1.0f );

	mat[ 0 ].Set( cp * cy,						cp * sy,					-sp,		0.0f );
	mat[ 1 ].Set( sr * mulspcy + cr * invsy,	sr * mulspsy + cr * cy,		sr * cp,	0.0f );
	mat[ 2 ].Set( cr * mulspcy + invsr * invsy,	cr * mulspsy + invsr * cy,	cr * cp,	0.0f );
	mat[ 3 ].Set( 0.0f,							0.0f,						0.0f,		1.0f );

	return mat;
}

#endif /* !__MATH_ANGLES_H__ */
